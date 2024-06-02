#pragma once

#include "scene.hh"

#include <cstdint>
#include <string>
#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

struct Settings {
  std::uint32_t window_height;
  std::uint32_t window_width;
  std::string shader_file;
  std::uint32_t group_size_x;
  std::uint32_t group_size_y;
};

struct RenderCallInfo {
  std::uint32_t number;
  std::uint32_t total_render_calls;
  std::uint32_t total_samples;
};

struct VulkanBuffer {
  vk::Buffer buffer;
  vk::DeviceMemory memory;
};

struct VulkanImage {
  vk::Image image;
  vk::DeviceMemory memory;
  vk::ImageView view;
};

class VulkanEngine {
private:
  Settings _settings;
  gpu::Scene _scene;

  GLFWwindow *_window;

  vk::Instance _instance;

  vk::SurfaceKHR _surface;

  vk::PhysicalDevice _selected_dev;

  std::uint32_t _compute_queue_family;
  std::uint32_t _present_queue_family;

  std::vector<const char *> _required_device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::Device _device;

  vk::Queue _compute_queue;
  vk::Queue _present_queue;

  VulkanBuffer _scene_buffer;
  VulkanBuffer _render_call_info_buffer;
  VulkanImage _summed_image;

  vk::DescriptorSetLayout _descriptor_set_layout;

  vk::DescriptorPool _descriptor_pool;

  vk::DescriptorSet _descriptor_set;

  vk::PipelineLayout _pipeline_layout;

  vk::Pipeline _pipeline;

  vk::CommandBuffer _command_buffer;

  vk::CommandPool _command_pool;

  vk::Fence _fence;

  vk::Semaphore _sema;

  vk::SwapchainKHR _swap_chain;
  vk::Image _swap_chain_image;
  vk::ImageView _swap_chain_image_view;

  [[nodiscard]] std::uint32_t
  _find_memory_type_index(std::uint32_t memory_type_bits,
                          const vk::MemoryPropertyFlags &properties);

  [[nodiscard]] VulkanBuffer
  _create_buffer(const vk::DeviceSize &size,
                 const vk::Flags<vk::BufferUsageFlagBits> &usage,
                 const vk::Flags<vk::MemoryPropertyFlagBits> &memory_property);

  [[nodiscard]]
  vk::ImageView _create_image_view(const vk::Image &image,
                                   const vk::Format &format);

  [[nodiscard]]
  VulkanImage _create_image(const vk::Format &format,
                            const vk::Flags<vk::ImageUsageFlagBits> &usage);

  [[nodiscard]]
  std::vector<char> _read_binary_file(const std::string &filename);

  [[nodiscard]]
  vk::ImageMemoryBarrier _image_pipeline_barrier(
      const vk::AccessFlagBits &src_flags, const vk::AccessFlagBits &dst_flags,
      const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout,
      const vk::Image &image) const;

  void _destroy_image(const VulkanImage &image) const;

  void _destroy_buffer(const VulkanBuffer &buffer) const;

  void _create_window();
  void _create_instance();
  void _create_surface();
  void _select_phys_device();
  void _find_queue_families();
  void _create_logical_device();
  void _create_scene_buffer();
  void _create_render_call_info_buffer();
  void _update_render_call_info_buffer(const RenderCallInfo &render_call_info);
  void _create_summed_pixel_color_image();
  void _create_command_pool();
  void _create_swap_chain();
  void _create_descriptor_set_layout();
  void _create_descriptor_pool();
  void _create_descriptor_set();
  void _create_pipeline_layout();
  void _create_pipeline();
  void _create_command_buffer();
  void _create_fence();
  void _create_semaphore();

public:
  VulkanEngine(const Settings &settings, gpu::Scene scene);
  ~VulkanEngine();

  void update();

  void render(const RenderCallInfo &render_call_info);

  [[nodiscard]] bool should_exit() const;
};
