#pragma once

#include "hittable.hh"
#include "ray.hh"

#include <ostream>
#include <string>

#include <glm/glm.hpp>

struct CameraConfig {
  float aspect_ratio;
  int image_width;
  int samples_per_pixel;
  int max_depth;
  float vfov;
  glm::vec3 eye;
  glm::vec3 center;
  glm::vec3 up;
  float defocus_angle;
  float focus_dist;
};

class Camera {
private:
  CameraConfig _config;
  int _image_height;
  glm::vec3 _pixel00_location;
  glm::vec3 _pixel_delta_u;
  glm::vec3 _pixel_delta_v;
  glm::vec3 _defocus_disk_u;
  glm::vec3 _defocus_disk_v;

  void _write_color(std::ostream &out, const glm::vec3 &color) const;

  glm::vec3 _ray_color(const Ray &ray, const Hittable &world,
                       int depth = 0) const;

  glm::vec3 _sample_square() const;

  glm::vec3 _defocus_disk_sample() const;

  Ray _ray_at_pixel(int y, int x) const;

public:
  static constexpr CameraConfig DEFAULT_CONFIG = {
      16.0f / 9.0f, 400,        100,       50,   90.0f,
      {0, 0, 0},    {0, 0, -1}, {0, 1, 0}, 0.0f, 10.0f};

  Camera();
  Camera(const Camera &) = default;
  Camera(Camera &&) = default;
  Camera &operator=(const Camera &) = default;
  Camera &operator=(Camera &&) = default;

  Camera(const CameraConfig &config);

  void render_to_file(const std::string &filename, const Hittable &world);
};
