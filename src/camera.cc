#include "camera.hh"

#include "hittable.hh"
#include "interval.hh"
#include "ray.hh"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

Camera::Camera() : Camera(16.0f / 9.0f, 400, 10) {}

Camera::Camera(float aspect_ratio, int image_width, int samples_per_pixel)
    : _aspect_ratio(aspect_ratio), _image_width(image_width),
      _samples_per_pixel(samples_per_pixel) {
  _image_height = std::max(static_cast<int>(image_width / aspect_ratio), 1);
  _center = glm::vec3(0, 0, 0);

  const auto focal_length = 1.0f, viewport_height = 2.0f,
             viewport_width =
                 viewport_height *
                 (static_cast<float>(_image_width) / _image_height);
  const auto viewport_u = glm::vec3(viewport_width, 0, 0),
             viewport_v = glm::vec3(0, -viewport_height, 0),
             viewport_upper_left = _center - glm::vec3(0, 0, focal_length) -
                                   viewport_u / 2.0f - viewport_v / 2.0f;
  _pixel_delta_u = viewport_u / static_cast<float>(_image_width);
  _pixel_delta_v = viewport_v / static_cast<float>(_image_height);
  _pixel00_location =
      viewport_upper_left + 0.5f * (_pixel_delta_u + _pixel_delta_v);
}

void Camera::_write_color(std::ostream &out, const glm::vec3 &color) const {
  static const auto intensity_interval = Interval(0.0f, 0.999f);
  const auto r = static_cast<int>(256 * intensity_interval.clamp(color[0])),
             g = static_cast<int>(256 * intensity_interval.clamp(color[1])),
             b = static_cast<int>(256 * intensity_interval.clamp(color[2]));
  out << r << " " << g << " " << b << "\n";
}

void Camera::render_to_file(const std::string &filename,
                            const Hittable &world) {
  std::ofstream outfile(filename);
  outfile << "P3\n" << _image_width << " " << _image_height << "\n255\n";
  for (int j = 0; j < _image_height; j++) {
    std::clog << "\rRemaining scanlines: " << (_image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < _image_width; i++) {
      auto pixel_color = glm::vec3(0, 0, 0);
      for (int sample = 0; sample < _samples_per_pixel; sample++) {
        const auto pixel_center = _pixel00_location +
                                  static_cast<float>(i) * _pixel_delta_u +
                                  static_cast<float>(j) * _pixel_delta_v;
        const auto ray_direction = pixel_center - _center;
        Ray r(_center, ray_direction);
        pixel_color += _ray_color(r, world);
      }
      _write_color(outfile,
                   pixel_color / static_cast<float>(_samples_per_pixel));
    }
  }
  std::clog << "\rDone.                 \n";
}

glm::vec3 Camera::_ray_color(const Ray &ray, const Hittable &world) const {
  const auto record = world.hit(ray, Interval(0, INFINITY));
  if (record.has_value())
    return 0.5f * (record->normal + glm::vec3(1, 1, 1));

  const auto unit_direction = glm::normalize(ray.direction());
  const auto a = 0.5f * (unit_direction[1] + 1.0f);
  return (1.0f - a) * glm::vec3(1, 1, 1) + a * glm::vec3(0.5, 0.7, 1.0);
}
