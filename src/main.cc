#include "camera.hh"
#include "hittable_list.hh"
#include "sphere.hh"

#include <memory>

#include <glm/glm.hpp>

int main() {
  auto world = HittableList();
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, 0, -1), 0.5f));
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, -100.5f, -1), 100));

  auto camera = Camera();
  camera.render_to_file("out.ppm", world);
}
