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

  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, 0, -1), 0.5f, material_center));
  world.hittables.push_back(std::make_shared<Sphere>(glm::vec3(0, -100.5f, -1),
                                                     100, material_ground));

  auto camera = Camera();
  camera.render_to_file("out.ppm", world);
}
