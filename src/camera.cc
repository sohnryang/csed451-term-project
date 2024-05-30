#include "camera.hh"

#include "hittable.hh"
#include "interval.hh"
#include "ray.hh"
#include "utils.hh"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

Camera::Camera() : Camera(DEFAULT_CONFIG) {}

Camera::Camera(const CameraConfig &config) : _config(config) {
  _image_height =
      std::max(static_cast<int>(_config.image_width / _config.aspect_ratio), 1);

  const auto theta = glm::radians(_config.vfov), h = std::tan(theta / 2),
             viewport_height = 2.0f * h * _config.focus_dist,
             viewport_width =
                 viewport_height *
                 (static_cast<float>(_config.image_width) / _image_height);
  const auto w = glm::normalize(_config.eye - _config.center),
             u = glm::normalize(glm::cross(_config.up, w)),
             v = glm::cross(w, u), viewport_u = viewport_width * u,
             viewport_v = -viewport_height * v,
             viewport_upper_left = _config.eye - (_config.focus_dist * w) -
                                   viewport_u / 2.0f - viewport_v / 2.0f;
  _pixel_delta_u = viewport_u / static_cast<float>(_config.image_width);
  _pixel_delta_v = viewport_v / static_cast<float>(_image_height);
  _pixel00_location =
      viewport_upper_left + 0.5f * (_pixel_delta_u + _pixel_delta_v);

  const auto defocus_radius =
      _config.focus_dist * tan(glm::radians(_config.defocus_angle / 2));
  _defocus_disk_u = defocus_radius * u;
  _defocus_disk_v = defocus_radius * v;
}

void Camera::_write_color(std::ostream &out, const glm::vec3 &color) const {
  static const auto intensity_interval = Interval(0.0f, 0.999f);
  const auto r = static_cast<int>(
                 256 * intensity_interval.clamp(linear_to_gamma(color[0]))),
             g = static_cast<int>(
                 256 * intensity_interval.clamp(linear_to_gamma(color[1]))),
             b = static_cast<int>(
                 256 * intensity_interval.clamp(linear_to_gamma(color[2])));
  out << r << " " << g << " " << b << "\n";
}

glm::vec3 Camera::_sample_square() const {
  return {random_float() - 1.0f, random_float() - 1.0f, 0};
}

glm::vec3 Camera::_defocus_disk_sample() const {
  const auto p = random_in_unit_disk();
  return _config.eye + p[0] * _defocus_disk_u + p[1] * _defocus_disk_v;
}

Ray Camera::_ray_at_pixel(int y, int x) const {
  const auto offset = _sample_square();
  const auto pixel_sample =
                 _pixel00_location +
                 (static_cast<float>(x) + offset[0]) * _pixel_delta_u +
                 (static_cast<float>(y) + offset[1]) * _pixel_delta_v,
             origin = _config.defocus_angle <= 0 ? _config.eye
                                                 : _defocus_disk_sample(),
             direction = pixel_sample - origin;
  return {origin, direction};
}

void Camera::render_to_file(const std::string &filename,
                            const Hittable &world) {
  std::ofstream outfile(filename);
  outfile << "P3\n" << _config.image_width << " " << _image_height << "\n255\n";
  for (int y = 0; y < _image_height; y++) {
    std::clog << "\rRemaining scanlines: " << (_image_height - y) << ' '
              << std::flush;
    for (int x = 0; x < _config.image_width; x++) {
      auto pixel_color = glm::vec3(0, 0, 0);
      for (int sample = 0; sample < _config.samples_per_pixel; sample++) {
        const auto ray = _ray_at_pixel(y, x);
        pixel_color += _ray_color(ray, world);
      }
      _write_color(outfile,
                   pixel_color / static_cast<float>(_config.samples_per_pixel));
    }
  }
  std::clog << "\rDone.                 \n";
}

glm::vec3 Camera::_ray_color(const Ray &ray, const Hittable &world,
                             int depth) const {
  if (depth >= _config.max_depth)
    return {0, 0, 0};

  const auto record = world.hit(ray, Interval(0.001f, INFINITY));
  if (record.has_value()) {
    const auto direction =
        random_on_hemisphere(record->normal) + record->normal;
    const auto material_hit = record->material->scatter(ray, *record);
    if (material_hit.has_value()) {
      const auto &[scattered, attenuation] = *material_hit;
      return _ray_color(scattered, world, depth + 1) * attenuation;
    }
    return {0, 0, 0};
  }

  const auto unit_direction = glm::normalize(ray.direction());
  const auto a = 0.5f * (unit_direction[1] + 1.0f);
  return (1.0f - a) * glm::vec3(1, 1, 1) + a * glm::vec3(0.5, 0.7, 1.0);
}
