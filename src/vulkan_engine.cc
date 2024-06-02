#include "vulkan_engine.hh"

#include "scene.hh"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

#ifdef __APPLE__
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan_beta.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

std::uint32_t VulkanEngine::_find_memory_type_index(
    std::uint32_t memory_type_bits, const vk::MemoryPropertyFlags &properties) {
  vk::PhysicalDeviceMemoryProperties memory_properties =
      _selected_dev.getMemoryProperties();

  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    if ((memory_type_bits & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("Unable to find suitable memory type!");
}

VulkanBuffer VulkanEngine::_create_buffer(
    const vk::DeviceSize &size, const vk::Flags<vk::BufferUsageFlagBits> &usage,
    const vk::Flags<vk::MemoryPropertyFlagBits> &memory_property) {
  vk::BufferCreateInfo buffer_createInfo{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
  };

  vk::Buffer buffer = _device.createBuffer(buffer_createInfo);

  vk::MemoryRequirements memory_requirements =
      _device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocate_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = _find_memory_type_index(
          memory_requirements.memoryTypeBits, memory_property)};

  vk::DeviceMemory memory = _device.allocateMemory(allocate_info);

  _device.bindBufferMemory(buffer, memory, 0);

  return {
      .buffer = buffer,
      .memory = memory,
  };
}

vk::ImageView VulkanEngine::_create_image_view(const vk::Image &image,
                                               const vk::Format &format) {
  vk::ImageViewCreateInfo image_view_create_info{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      }};
  return _device.createImageView(image_view_create_info);
}

VulkanImage
VulkanEngine::_create_image(const vk::Format &format,
                            const vk::Flags<vk::ImageUsageFlagBits> &usage) {
  vk::ImageCreateInfo image_create_info{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {.width = _settings.window_width,
                 .height = _settings.window_height,
                 .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined,
  };

  vk::Image image = _device.createImage(image_create_info);

  vk::MemoryRequirements memory_requirements =
      _device.getImageMemoryRequirements(image);

  vk::MemoryAllocateInfo alloc_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          _find_memory_type_index(memory_requirements.memoryTypeBits,
                                  vk::MemoryPropertyFlagBits::eDeviceLocal),
  };

  vk::DeviceMemory memory = _device.allocateMemory(alloc_info);

  _device.bindImageMemory(image, memory, 0);

  return {.image = image,
          .memory = memory,
          .view = _create_image_view(image, format)};
}

std::vector<char> VulkanEngine::_read_binary_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
    throw std::runtime_error("Failed to open: " + filename);

  size_t size = (size_t)file.tellg();
  std::vector<char> buffer(size);

  file.seekg(0);
  file.read(buffer.data(), size);

  return buffer;
}

vk::ImageMemoryBarrier VulkanEngine::_image_pipeline_barrier(
    const vk::AccessFlagBits &src_flags, const vk::AccessFlagBits &dst_flags,
    const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout,
    const vk::Image &image) const {
  return {
      .srcAccessMask = src_flags,
      .dstAccessMask = dst_flags,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = _compute_queue_family,
      .dstQueueFamilyIndex = _compute_queue_family,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
}

void VulkanEngine::_destroy_image(const VulkanImage &image) const {
  _device.destroyImageView(image.view);
  _device.destroyImage(image.image);
  _device.freeMemory(image.memory);
}

void VulkanEngine::_destroy_buffer(const VulkanBuffer &buffer) const {
  _device.destroyBuffer(buffer.buffer);
  _device.freeMemory(buffer.memory);
}

void VulkanEngine::_create_window() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  _window = glfwCreateWindow(_settings.window_width, _settings.window_height,
                             "GPU Tracer", nullptr, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_message_fn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                 VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                 VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                 void *pUserData) {
  std::cout << "["
            << vk::to_string(
                   static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(
                       messageSeverity))
            << " | "
            << vk::to_string(
                   static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes))
            << "]:\n"
            << "id      : " << pCallbackData->pMessageIdName << "\n"
            << "message : " << pCallbackData->pMessage << "\n"
            << std::endl;

  return false;
}

void VulkanEngine::_create_instance() {
  vk::ApplicationInfo app_info{
      .pApplicationName = "GPU Tracer",
      .applicationVersion = 1,
      .pEngineName = "GPU Tracer",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_2,
  };

  std::uint32_t window_extension_count;
  const char **window_extension_names =
      glfwGetRequiredInstanceExtensions(&window_extension_count);
  std::vector<const char *> enabled_extensions;
  enabled_extensions.insert(enabled_extensions.end(), window_extension_names,
                            window_extension_names + window_extension_count);
#ifdef __APPLE__
  enabled_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

  std::vector<const char *> enabled_layers = {"VK_LAYER_KHRONOS_validation"};
#ifdef _DEBUG
  enabled_layers.clear();
#endif

  vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_info;
  debug_messenger_info.messageSeverity =
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
  debug_messenger_info.messageType =
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
  debug_messenger_info.pfnUserCallback = &debug_message_fn;
  debug_messenger_info.pUserData = nullptr;

  vk::InstanceCreateInfo instance_create_info{
      .pNext = &debug_messenger_info,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
      .ppEnabledLayerNames = enabled_layers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
      .ppEnabledExtensionNames = enabled_extensions.data(),
  };
#ifdef __APPLE__
  instance_create_info.flags |=
      vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

#ifndef _DEBUG
  instance_create_info.pNext = nullptr;
#endif

  _instance = vk::createInstance(instance_create_info);
}

void VulkanEngine::_create_surface() {
  glfwCreateWindowSurface(_instance, _window, nullptr,
                          reinterpret_cast<VkSurfaceKHR *>(&_surface));
}

void VulkanEngine::_select_phys_device() {
  const auto physical_devs = _instance.enumeratePhysicalDevices();
  if (physical_devs.empty())
    throw std::runtime_error("No GPU found");

  std::vector<std::string> required_dev_extensions;
  std::transform(_required_device_extensions.begin(),
                 _required_device_extensions.end(),
                 std::back_inserter(required_dev_extensions),
                 [](const char *name) { return std::string(name); });

  for (const auto &dev : physical_devs) {
    auto available_extensions = dev.enumerateDeviceExtensionProperties();
    std::vector<std::string> available_extension_names;
    std::transform(available_extensions.begin(), available_extensions.end(),
                   std::back_inserter(available_extension_names),
                   [](const auto &ext) { return ext.extensionName; });
    std::cout << "Available extensions:";
    for (const auto &ext : available_extension_names)
      std::cout << " " << ext;
    std::cout << std::endl;
    std::vector<std::string> diff;
    std::set_difference(
        required_dev_extensions.begin(), required_dev_extensions.end(),
        available_extension_names.begin(), available_extension_names.end(),
        std::back_inserter(diff));
    std::cout << "Diff:";
    for (const auto &ext : diff)
      std::cout << " " << ext;
    if (diff.empty()) {
      _selected_dev = dev;
      return;
    }
  }

  throw std::runtime_error("No suitable GPU found");
}

void VulkanEngine::_find_queue_families() {
  std::vector<vk::QueueFamilyProperties> queue_families =
      _selected_dev.getQueueFamilyProperties();

  bool compute_family_found = false;
  bool present_family_found = false;

  for (std::uint32_t i = 0; i < queue_families.size(); i++) {
    const auto supports_graphics =
        (queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics) ==
        vk::QueueFlagBits::eGraphics;
    const auto supports_compute =
        (queue_families[i].queueFlags & vk::QueueFlagBits::eCompute) ==
        vk::QueueFlagBits::eCompute;
    const auto supports_presenting =
        _selected_dev.getSurfaceSupportKHR(i, _surface);

    if (supports_compute && !supports_graphics && !compute_family_found) {
      _compute_queue_family = i;
      compute_family_found = true;
      continue;
    }

    if (supports_presenting && !present_family_found) {
      _present_queue_family = i;
      present_family_found = true;
    }

    if (compute_family_found && present_family_found)
      break;
  }
}

void VulkanEngine::_create_logical_device() {
  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo compute_queue_info{
      .queueFamilyIndex = _compute_queue_family,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  },
      present_queue_info{
          .queueFamilyIndex = _present_queue_family,
          .queueCount = 1,
          .pQueuePriorities = &queue_priority,
      };
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {
      compute_queue_info, present_queue_info};

  vk::PhysicalDeviceFeatures device_features = {};

  vk::DeviceCreateInfo device_create_info{
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(_required_device_extensions.size()),
      .ppEnabledExtensionNames = _required_device_extensions.data(),
      .pEnabledFeatures = &device_features,
  };

  _device = _selected_dev.createDevice(device_create_info);

  _compute_queue = _device.getQueue(_compute_queue_family, 0);
  _present_queue = _device.getQueue(_present_queue_family, 0);
}

void VulkanEngine::_create_scene_buffer() {
  _scene_buffer = _create_buffer(sizeof(gpu::Scene),
                                 vk::BufferUsageFlagBits::eUniformBuffer,
                                 vk::MemoryPropertyFlagBits::eHostVisible |
                                     vk::MemoryPropertyFlagBits::eHostCoherent);
  void *data = _device.mapMemory(_scene_buffer.memory, 0, sizeof(gpu::Scene));
  std::memcpy(data, &_scene, sizeof(gpu::Scene));
  _device.unmapMemory(_scene_buffer.memory);
}

void VulkanEngine::_create_render_call_info_buffer() {
  _render_call_info_buffer = _create_buffer(
      sizeof(RenderCallInfo), vk::BufferUsageFlagBits::eUniformBuffer,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);
}
void VulkanEngine::_update_render_call_info_buffer(
    const RenderCallInfo &render_call_info) {
  void *data = _device.mapMemory(_render_call_info_buffer.memory, 0,
                                 sizeof(RenderCallInfo));
  std::memcpy(data, &render_call_info, sizeof(RenderCallInfo));
  _device.unmapMemory(_render_call_info_buffer.memory);
}

void VulkanEngine::_create_summed_pixel_color_image() {
  _summed_image = _create_image(vk::Format::eR16G16B16A16Unorm,
                                vk::ImageUsageFlagBits::eStorage);
}

void VulkanEngine::_create_command_pool() {
  vk::CommandPoolCreateInfo info;
  info.queueFamilyIndex = _compute_queue_family;
  _command_pool = _device.createCommandPool(info);
}

void VulkanEngine::_create_swap_chain() {
  vk::SwapchainCreateInfoKHR swap_chain_create_info{
      .surface = _surface,
      .minImageCount = 1,
      .imageFormat = vk::Format::eR8G8B8A8Unorm,
      .imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
      .imageExtent = {.width = _settings.window_width,
                      .height = _settings.window_height},
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eStorage |
                    vk::ImageUsageFlagBits::eTransferSrc,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform =
          _selected_dev.getSurfaceCapabilitiesKHR(_surface).currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = vk::PresentModeKHR::eImmediate,
      .clipped = true,
      .oldSwapchain = nullptr,
  };

  _swap_chain = _device.createSwapchainKHR(swap_chain_create_info);

  // swap chain images
  std::vector<vk::Image> swap_chain_images =
      _device.getSwapchainImagesKHR(_swap_chain);
  _swap_chain_image = swap_chain_images.front();
  _swap_chain_image_view =
      _create_image_view(_swap_chain_image, vk::Format::eR8G8B8A8Unorm);
}

void VulkanEngine::_create_descriptor_set_layout() {
  std::vector<vk::DescriptorSetLayoutBinding> bindings = {
      {.binding = 0,
       .descriptorType = vk::DescriptorType::eStorageImage,
       .descriptorCount = 1,
       .stageFlags = vk::ShaderStageFlagBits::eCompute},
      {.binding = 1,
       .descriptorType = vk::DescriptorType::eStorageImage,
       .descriptorCount = 1,
       .stageFlags = vk::ShaderStageFlagBits::eCompute},
      {.binding = 2,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .descriptorCount = 1,
       .stageFlags = vk::ShaderStageFlagBits::eCompute},
      {.binding = 3,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .descriptorCount = 1,
       .stageFlags = vk::ShaderStageFlagBits::eCompute},
  };

  _descriptor_set_layout =
      _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{
          .bindingCount = static_cast<uint32_t>(bindings.size()),
          .pBindings = bindings.data()});
}

void VulkanEngine::_create_descriptor_pool() {
  std::vector<vk::DescriptorPoolSize> poolSizes{
      {.type = vk::DescriptorType::eStorageImage, .descriptorCount = 2},
      {.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 2}};

  _descriptor_pool = _device.createDescriptorPool(
      {.maxSets = 1,
       .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
       .pPoolSizes = poolSizes.data()});
}

void VulkanEngine::_create_descriptor_set() {
  _descriptor_set =
      _device
          .allocateDescriptorSets({.descriptorPool = _descriptor_pool,
                                   .descriptorSetCount = 1,
                                   .pSetLayouts = &_descriptor_set_layout})
          .front();

  vk::DescriptorImageInfo render_target_image_info = {
      {}, _swap_chain_image_view, vk::ImageLayout::eGeneral};

  vk::DescriptorImageInfo summed_image_info = {
      {}, _summed_image.view, vk::ImageLayout::eGeneral};

  vk::DescriptorBufferInfo scene_buffer_info = {_scene_buffer.buffer, 0,
                                                sizeof(gpu::Scene)};

  vk::DescriptorBufferInfo render_call_info_buffer_info = {
      _render_call_info_buffer.buffer, 0, sizeof(RenderCallInfo)};

  std::vector<vk::WriteDescriptorSet> descriptor_writes{
      {.dstSet = _descriptor_set,
       .dstBinding = 0,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eStorageImage,
       .pImageInfo = &render_target_image_info},
      {.dstSet = _descriptor_set,
       .dstBinding = 1,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eStorageImage,
       .pImageInfo = &summed_image_info},
      {.dstSet = _descriptor_set,
       .dstBinding = 2,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .pBufferInfo = &scene_buffer_info},
      {.dstSet = _descriptor_set,
       .dstBinding = 3,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .pBufferInfo = &render_call_info_buffer_info}};

  _device.updateDescriptorSets(static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(), 0, nullptr);
}

void VulkanEngine::_create_pipeline_layout() {
  _pipeline_layout = _device.createPipelineLayout({
      .setLayoutCount = 1,
      .pSetLayouts = &_descriptor_set_layout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  });
}

void VulkanEngine::_create_pipeline() {
  std::vector<char> _shader_code = _read_binary_file(_settings.shader_file);

  vk::ShaderModuleCreateInfo shader_module_create_info = {
      .codeSize = _shader_code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(_shader_code.data())};

  vk::ShaderModule compute_shader_module =
      _device.createShaderModule(shader_module_create_info);

  vk::PipelineShaderStageCreateInfo shaderStage = {
      .stage = vk::ShaderStageFlagBits::eCompute,
      .module = compute_shader_module,
      .pName = "main",
  };

  vk::ComputePipelineCreateInfo pipeline_create_info = {
      .stage = shaderStage, .layout = _pipeline_layout};

  _pipeline =
      _device.createComputePipeline(nullptr, pipeline_create_info).value;

  _device.destroyShaderModule(compute_shader_module);
}

void VulkanEngine::_create_command_buffer() {
  _command_buffer = _device
                        .allocateCommandBuffers({
                            .commandPool = _command_pool,
                            .level = vk::CommandBufferLevel::ePrimary,
                            .commandBufferCount = 1,
                        })
                        .front();

  vk::CommandBufferBeginInfo begin_info = {};
  const auto res = _command_buffer.begin(&begin_info);
  if (res != vk::Result::eSuccess)
    throw std::runtime_error("Vulkan error");

  _command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline);

  std::vector<vk::DescriptorSet> descriptorSets = {_descriptor_set};
  _command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                     _pipeline_layout, 0, descriptorSets,
                                     nullptr);

  vk::ImageMemoryBarrier imageBarriersToGeneral[2] = {
      _image_pipeline_barrier(vk::AccessFlagBits::eNoneKHR,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eGeneral, _swap_chain_image),
      _image_pipeline_barrier(vk::AccessFlagBits::eNoneKHR,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eGeneral, _summed_image.image),
  };

  _command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                  vk::PipelineStageFlagBits::eComputeShader,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 2, imageBarriersToGeneral);

  _command_buffer.dispatch(
      static_cast<uint32_t>(std::ceil(float(_settings.window_width) /
                                      float(_settings.group_size_x))),
      static_cast<uint32_t>(std::ceil(float(_settings.window_height) /
                                      float(_settings.group_size_y))),
      1);

  vk::ImageMemoryBarrier image_barrier_to_present = _image_pipeline_barrier(
      vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eMemoryRead,
      vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR,
      _swap_chain_image);
  _command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                  vk::PipelineStageFlagBits::eBottomOfPipe,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 1, &image_barrier_to_present);

  _command_buffer.end();
}

void VulkanEngine::_create_fence() {
  _fence = _device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
}

void VulkanEngine::_create_semaphore() {
  _sema = _device.createSemaphore({});
  _render_sema = _device.createSemaphore({});
}

void VulkanEngine::_recreate_swapchain() {
  _compute_queue.waitIdle();
  _present_queue.waitIdle();

  _device.destroyImageView(_swap_chain_image_view);
  _device.destroySwapchainKHR(_swap_chain);

  _create_swap_chain();
}

VulkanEngine::VulkanEngine(const Settings &settings, gpu::Scene scene)
    : _settings(settings), _scene(scene) {
  _create_window();
  _create_instance();
  _create_surface();
  _select_phys_device();
  _find_queue_families();
  _create_logical_device();
  _create_scene_buffer();
  _create_render_call_info_buffer();
  _create_summed_pixel_color_image();
  _create_command_pool();
  _create_swap_chain();
  _create_descriptor_set_layout();
  _create_descriptor_pool();
  _create_descriptor_set();
  _create_pipeline_layout();
  _create_pipeline();
  _create_command_buffer();
  _create_fence();
  _create_semaphore();
}

VulkanEngine::~VulkanEngine() {
  _destroy_image(_summed_image);
  _destroy_buffer(_scene_buffer);
  _destroy_buffer(_render_call_info_buffer);

  _device.destroySemaphore(_sema);
  _device.destroyFence(_fence);
  _device.destroySemaphore(_render_sema);
  _device.destroyPipeline(_pipeline);
  _device.destroyPipelineLayout(_pipeline_layout);
  _device.destroyDescriptorSetLayout(_descriptor_set_layout);
  _device.destroyDescriptorPool(_descriptor_pool);
  _device.destroyImageView(_swap_chain_image_view);
  _device.destroySwapchainKHR(_swap_chain);
  _device.destroyCommandPool(_command_pool);
  _device.destroy();

  glfwDestroyWindow(_window);
  glfwTerminate();
}

void VulkanEngine::update() { glfwPollEvents(); }

void VulkanEngine::render(const RenderCallInfo &render_call_info) {
  _update_render_call_info_buffer(render_call_info);

  const auto swap_chain_image_index =
      _device
          .acquireNextImageKHR(_swap_chain,
                               std::numeric_limits<std::uint64_t>::max(), _sema)
          .value;
  _device.resetFences(_fence);
  if (swap_chain_image_index == 1)
    return;

  vk::SubmitInfo submit_info{.commandBufferCount = 1,
                             .pCommandBuffers = &_command_buffer};
  auto res = _compute_queue.submit(1, &submit_info, _fence);
  if (res != vk::Result::eSuccess)
    throw std::runtime_error("Submit failed");

  res = _device.waitForFences(1, &_fence, true,
                              std::numeric_limits<std::uint64_t>::max());
  if (res != vk::Result::eSuccess)
    throw std::runtime_error("Fence wait failed");
  _device.resetFences(_fence);

  vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &_sema,
      .swapchainCount = 1,
      .pSwapchains = &_swap_chain,
      .pImageIndices = &swap_chain_image_index,
  };
  res = _present_queue.presentKHR(present_info);
}

bool VulkanEngine::should_exit() const {
  return glfwWindowShouldClose(_window);
}
