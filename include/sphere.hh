#pragma once

#include "hittable.hh"
#include "interval.hh"
#include "ray.hh"

#include <optional>

#include <glm/glm.hpp>

class Sphere : public Hittable {
private:
  glm::vec3 _center;
  float _radius;

public:
  Sphere(const glm::vec3 &center, float radius);

  std::optional<HitRecord> hit(const Ray &ray, Interval ray_t) const override;
};
