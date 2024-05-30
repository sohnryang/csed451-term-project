#include "camera.hh"
#include "disk.hh"
#include "hittable_list.hh"
#include "material.hh"
#include "sphere.hh"
#include "utils.hh"

#include <memory>

#include <glm/glm.hpp>

int main() {
  HittableList world;

  auto ground_material = std::make_shared<Lambertian>(glm::vec3(0.5, 0.5, 0.5));
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, ground_material));

  auto material1 = std::make_shared<Dielectric>(1.5f);

  auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4, 0.2, 0.1));
  world.hittables.push_back(
      make_shared<Sphere>(glm::vec3(-2, 1, 0), 1.0f, material2));

  auto material3 = std::make_shared<Metal>(glm::vec3(0.7, 0.6, 0.5), 0.0f);
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(2, 1, 4), 1.0f, material3));

  world.hittables.push_back(std::make_shared<Disk>(
      glm::vec3(0, 3, 2), glm::vec3(0, 0, 1), 3, material3));

  const CameraConfig config = {16.0f / 9.0f, 400,       500,       50,   20,
                               {0, 1, -10},  {0, 1, 0}, {0, 1, 0}, 0.6f, 10.0f};
  Camera cam(config);

  cam.render_to_file("out.ppm", world);
}
