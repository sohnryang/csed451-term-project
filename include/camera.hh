#pragma once

#include "hittable.hh"
#include "ray.hh"

#include <ostream>
#include <string>

#include <glm/glm.hpp>

class Camera {
private:
  float _aspect_ratio;
  int _image_width;
  int _samples_per_pixel;
  int _image_height;
  int _max_depth;
  float _vfov;
  glm::vec3 _center;
  glm::vec3 _pixel00_location;
  glm::vec3 _pixel_delta_u;
  glm::vec3 _pixel_delta_v;

  void _write_color(std::ostream &out, const glm::vec3 &color) const;

  glm::vec3 _ray_color(const Ray &ray, const Hittable &world,
                       int depth = 0) const;

  glm::vec3 _sample_square() const;

  Ray _ray_at_pixel(int y, int x) const;

public:
  Camera();
  Camera(const Camera &) = default;
  Camera(Camera &&) = default;
  Camera &operator=(const Camera &) = default;
  Camera &operator=(Camera &&) = default;

  Camera(float aspect_ratio, int image_width, int samples_per_pixel,
         int max_depth, float vfov);

  void render_to_file(const std::string &filename, const Hittable &world);
};
