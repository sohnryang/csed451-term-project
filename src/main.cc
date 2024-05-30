#include "camera.hh"
#include "hittable_list.hh"
#include "material.hh"
#include "sphere.hh"

#include <memory>

#include <glm/glm.hpp>

int main() {
  auto world = HittableList();

  auto material_ground = std::make_shared<Lambertian>(glm::vec3(0.8, 0.8, 0.0));
  auto material_center = std::make_shared<Lambertian>(glm::vec3(0.1, 0.2, 0.5));
  auto material_left = std::make_shared<Dielectric>(1.5f);
  auto material_bubble = std::make_shared<Dielectric>(1.0f / 1.5f);
  auto material_right = std::make_shared<Metal>(glm::vec3(0.8, 0.6, 0.2), 1.0f);

  world.hittables.push_back(std::make_shared<Sphere>(glm::vec3(0, -100.5f, -1),
                                                     100, material_ground));
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, 0, -1.2f), 0.5f, material_center));
  world.hittables.push_back(std::make_shared<Sphere>(glm::vec3(-1.0f, 0, -1.0f),
                                                     0.5f, material_left));
  world.hittables.push_back(std::make_shared<Sphere>(glm::vec3(-1.0f, 0, -1.0f),
                                                     0.4f, material_bubble));
  world.hittables.push_back(std::make_shared<Sphere>(glm::vec3(1.0f, 0, -1.0f),
                                                     0.5f, material_right));

  auto camera = Camera();
  camera.render_to_file("out.ppm", world);
}
