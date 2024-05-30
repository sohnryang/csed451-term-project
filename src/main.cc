#include "camera.hh"
#include "disk.hh"
#include "hittable_list.hh"
#include "material.hh"
#include "portal_material.hh"
#include "sphere.hh"
#include "utils.hh"

#include <memory>

#include <glm/glm.hpp>

int main() {
  HittableList world;

  auto ground_material = std::make_shared<Lambertian>(glm::vec3(0.5, 0.5, 0.5));
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, -1000, 0), 1000, ground_material));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      const auto choose_mat = random_float();
      const auto center =
          glm::vec3(a + 0.9f * random_float(), 0.2f, b + 0.9f * random_float());

      if ((center - glm::vec3(4.0, 0.2, 0.0)).length() > 0.9f) {
        std::shared_ptr<Material> sphere_material;

        if (choose_mat < 0.8f) {
          // diffuse
          const auto albedo = random_vec() * random_vec();
          sphere_material = std::make_shared<Lambertian>(albedo);
          world.hittables.push_back(
              std::make_shared<Sphere>(center, 0.2f, sphere_material));
        } else if (choose_mat < 0.95f) {
          // metal
          const auto albedo = random_vec(0.5f, 1);
          const auto fuzz = random_float(0, 0.5f);
          sphere_material = std::make_shared<Metal>(albedo, fuzz);
          world.hittables.push_back(
              std::make_shared<Sphere>(center, 0.2f, sphere_material));
        } else {
          // glass
          sphere_material = std::make_shared<Dielectric>(1.5f);
          world.hittables.push_back(
              std::make_shared<Sphere>(center, 0.2f, sphere_material));
        }
      }
    }
  }

  auto material1 = std::make_shared<Dielectric>(1.5f);
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(0, 1, 0), 1.0f, material1));

  auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4, 0.2, 0.1));
  world.hittables.push_back(
      make_shared<Sphere>(glm::vec3(-4, 1, 0), 1.0f, material2));

  auto material3 = std::make_shared<Metal>(glm::vec3(0.7, 0.6, 0.5), 0.0f);
  world.hittables.push_back(
      std::make_shared<Sphere>(glm::vec3(4, 1, 0), 1.0f, material3));

  const auto source_center = glm::vec3(5, 1, 0),
             source_normal = glm::vec3(-1, 0, 0),
             destination_center = glm::vec3(0, 1, -2),
             destination_normal = glm::vec3(0, 0, 1);
  auto source_material = std::make_shared<PortalMaterial>(
      source_center, source_normal, destination_center, destination_normal);
  auto destination_material = std::make_shared<PortalMaterial>(
      destination_center, destination_normal, source_center, source_normal);
  world.hittables.push_back(std::make_shared<Disk>(source_center, source_normal,
                                                   1.0f, source_material));
  world.hittables.push_back(std::make_shared<Disk>(
      destination_center, destination_normal, 1.0f, destination_material));

  const CameraConfig config = {16.0f / 9.0f, 400,       500,       50,   20,
                               {12, 2, 3},   {0, 0, 0}, {0, 1, 0}, 0.6f, 10.0f};
  Camera cam(config);

  cam.render_to_file("out.ppm", world);
}
