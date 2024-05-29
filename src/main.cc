#include "ray.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ostream>

#include <glm/glm.hpp>

float hit_sphere(const glm::vec3 &center, float radius, const Ray &ray) {
  const auto oc = center - ray.origin();
  const auto a = glm::dot(ray.direction(), ray.direction());
  const auto b = -2.0f * glm::dot(ray.direction(), oc),
             c = glm::dot(oc, oc) - radius * radius,
             discriminant = b * b - 4 * a * c;
  return discriminant < 0 ? -1.0f : (-b - std::sqrt(discriminant)) / (2.0f * a);
}

glm::vec3 ray_color(const Ray &ray) {
  const auto t = hit_sphere({0, 0, -1}, 0.5f, ray);
  if (t > 0.0f) {
    const auto normal = glm::normalize(ray.at(t) - glm::vec3(0, 0, -1));
    return 0.5f * glm::vec3(normal[0] + 1, normal[1] + 1, normal[2] + 1);
  }
  const auto unit_direction = glm::normalize(ray.direction());
  const auto a = 0.5f * (unit_direction[1] + 1.0f);
  return (1.0f - a) * glm::vec3(1, 1, 1) + a * glm::vec3(0.5, 0.7, 1.0);
}

void write_color(std::ostream &out, const glm::vec3 &color) {
  const auto r = static_cast<int>(255.999f * color[0]),
             g = static_cast<int>(255.999f * color[1]),
             b = static_cast<int>(255.999f * color[2]);
  out << r << " " << g << " " << b << "\n";
}

int main() {
  const auto aspect_ratio = 16.0f / 9.0f;
  const auto image_width = 400,
             image_height =
                 std::max(static_cast<int>(image_width / aspect_ratio), 1);

  const auto focal_length = 1.0f, viewport_height = 2.0f,
             viewport_width = viewport_height *
                              (static_cast<float>(image_width) / image_height);

  const auto camera_center = glm::vec3(0, 0, 0),
             viewport_u = glm::vec3(viewport_width, 0, 0),
             viewport_v = glm::vec3(0, -viewport_height, 0),
             pixel_delta_u = viewport_u / static_cast<float>(image_width),
             pixel_delta_v = viewport_v / static_cast<float>(image_height),
             viewport_upper_left = camera_center -
                                   glm::vec3(0, 0, focal_length) -
                                   viewport_u / 2.0f - viewport_v / 2.0f,
             pixel00_location =
                 viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

  std::ofstream outfile("out.ppm");
  outfile << "P3\n" << image_width << " " << image_height << "\n255\n";
  for (int j = 0; j < image_height; j++) {
    std::clog << "\rRemaining scanlines: " << (image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < image_width; i++) {
      const auto pixel_center = pixel00_location +
                                static_cast<float>(i) * pixel_delta_u +
                                static_cast<float>(j) * pixel_delta_v;
      const auto ray_direction = pixel_center - camera_center;
      Ray r(camera_center, ray_direction);
      const auto pixel_color = ray_color(r);
      write_color(outfile, pixel_color);
    }
  }
  std::clog << "\rDone.                 \n";
}
