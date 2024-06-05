#include "scene.hh"
#include "utils.hh"
#include "vulkan_engine.hh"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <glm/ext/scalar_constants.hpp>
#include <glm/trigonometric.hpp>

using namespace std::chrono_literals;

int main() {
  std::vector<gpu::Material> materials;
  std::vector<gpu::Hittable> hittables;
  hittables.push_back({
      .kind = gpu::HittableKind::SPHERE,
      .center = glm::vec3(0, -1000, 0),
      .radius = 1000,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(
      {.kind = gpu::MaterialKind::LAMBERTIAN, .color = {0.5f, 0.5f, 0.5f}});

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      const auto choose_mat = random_float();
      const auto center =
          glm::vec3(a + 0.9f * random_float(), 0.2f, b + 0.9f * random_float());
      hittables.push_back({
          .kind = gpu::HittableKind::SPHERE,
          .center = center,
          .radius = 0.2f,
          .material_index = static_cast<uint32_t>(materials.size()),
      });

      if ((center - glm::vec3(4.0, 0.2, 0.0)).length() > 0.9f) {
        gpu::Material sphere_material;

        if (choose_mat < 0.8f) {
          // diffuse
          const auto albedo = random_vec() * random_vec();
          sphere_material.kind = gpu::MaterialKind::LAMBERTIAN;
          sphere_material.color = albedo;
        } else if (choose_mat < 0.95f) {
          // metal
          const auto albedo = random_vec(0.5f, 1);
          const auto fuzz = random_float(0, 0.5f);
          sphere_material.kind = gpu::MaterialKind::METAL;
          sphere_material.color = albedo;
          sphere_material.parameter = fuzz;
        } else {
          // glass
          sphere_material.kind = gpu::MaterialKind::DIELECTRIC;
          sphere_material.parameter = 1.5;
        }
        materials.push_back(sphere_material);
      }
    }
  }

  gpu::Material material1 = {
      .kind = gpu::MaterialKind::DIELECTRIC,
      .parameter = 1.5f,
  };
  hittables.push_back({
      .kind = gpu::HittableKind::SPHERE,
      .center = {0, 1, 0},
      .radius = 1.0f,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(material1);

  gpu::Material material2 = {
      .kind = gpu::MaterialKind::LAMBERTIAN,
      .color = glm::vec3(0.4, 0.2, 0.1),
  };
  hittables.push_back({
      .kind = gpu::HittableKind::SPHERE,
      .center = {-2.5, 1, 0},
      .radius = 1.0f,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(material2);

  gpu::Material material3 = {
      .kind = gpu::MaterialKind::METAL,
      .color = glm::vec3(0.7, 0.6, 0.5),
      .parameter = 0.0f,
  };
  hittables.push_back({
      .kind = gpu::HittableKind::SPHERE,
      .center = glm::vec3(2.5, 1, 0),
      .radius = 1.0f,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(material3);

  const auto source_center = glm::vec3(3.5, 1, 0),
             source_normal = glm::vec3(-1, 0, 0),
             destination_center = glm::vec3(0, 1, 2),
             destination_normal = glm::vec3(0, 0, -1);
  const auto source_material = gpu::Material::from_disk_pair(
      source_center, source_normal, destination_center, destination_normal);
  hittables.push_back({
      .kind = gpu::HittableKind::DISK,
      .center = source_center,
      .normal = source_normal,
      .radius = 1.0f,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(source_material);
  const auto destination_material = gpu::Material::from_disk_pair(
      destination_center, destination_normal, source_center, source_normal);
  hittables.push_back({
      .kind = gpu::HittableKind::DISK,
      .center = destination_center,
      .normal = destination_normal,
      .radius = 1.0f,
      .material_index = static_cast<uint32_t>(materials.size()),
  });
  materials.push_back(destination_material);

  gpu::Scene scene;
  scene.camera = {
      .eye = {0, 2, 12},
      .center = {0, 0, 0},
      .up = {0, 1, 0},
      .vfov = 20,
      .defocus_angle = 0.6f,
      .focus_dist = 10.0f,
  };
  std::cout << "Number of hittable objects: " << hittables.size() << std::endl;
  scene.hittables_count = hittables.size();
  std::copy(hittables.begin(), hittables.end(), scene.hittables);
  std::copy(materials.begin(), materials.end(), scene.materials);

  const std::uint32_t render_calls = 20, samples = 100;
  Settings settings{.window_height = 720,
                    .window_width = 1280,
                    .shader_file = "shader.comp.spv",
                    .group_size_x = 16,
                    .group_size_y = 8};

  VulkanEngine engine(settings);

  std::uint32_t i = 0;
  float pan_angle = 90.0f;
  bool clear = false;
  while (!engine.should_exit()) {
    std::cout << "Render call #" << i << std::endl;
    engine.update();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Camera Control");
    ImGui::SliderFloat("Pan Angle", &pan_angle, 0, 360.0f);
    if (i == render_calls)
      if (ImGui::Button("Render")) {
        i = 0;
        clear = true;
        scene.camera.eye = {12 * std::cos(glm::radians(pan_angle)), 2,
                            12 * std::sin(glm::radians(pan_angle))};
      }
    ImGui::End();
    ImGui::Render();

    RenderCallInfo info = {.read_only = i == render_calls,
                           .clear = clear,
                           .number = i,
                           .total_render_calls = render_calls,
                           .total_samples = samples};
    engine.render(info, scene);

    if (!clear)
      i = std::min(i + 1, render_calls);
    clear = false;
  }
}
