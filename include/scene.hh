#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace gpu {
enum HittableKind { SPHERE = 0, DISK = 1 };
enum MaterialKind { LAMBERTIAN = 0, METAL = 1, DIELECTRIC = 2, PORTAL = 3 };

struct Hittable {
  alignas(4) std::uint32_t kind;
  alignas(16) glm::vec3 center;
  alignas(16) glm::vec3 normal;
  alignas(4) float radius;
  alignas(4) std::uint32_t material_index;
};

struct Color {
  alignas(16) glm::vec3 color;
};

struct Material {
  alignas(4) std::uint32_t kind;
  alignas(16) glm::vec3 color;
  alignas(4) float parameter;
  alignas(16) glm::mat4 translation_mat1;
  alignas(16) glm::mat4 translation_mat2;
  alignas(16) glm::mat4 rotation_mat;

  static Material from_disk_pair(const glm::vec3 &source_center,
                                 const glm::vec3 &source_normal,
                                 const glm::vec3 &destination_center,
                                 const glm::vec3 &destination_normal);
};

struct Camera {
  alignas(16) glm::vec3 eye;
  alignas(16) glm::vec3 center;
  alignas(16) glm::vec3 up;
  alignas(4) float vfov;
  alignas(4) float defocus_angle;
  alignas(4) float focus_dist;
};

struct Scene {
  alignas(4) std::uint32_t hittables_count;
  Camera camera;
  Hittable hittables[500];
  Material materials[500];
};
} // namespace gpu
