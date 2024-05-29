#pragma once

#include "hittable.hh"
#include "interval.hh"
#include "material.hh"
#include "ray.hh"

#include <memory>
#include <optional>

#include <glm/glm.hpp>

class Sphere : public Hittable {
private:
  glm::vec3 _center;
  float _radius;
  std::shared_ptr<Material> _material;

public:
  Sphere(const glm::vec3 &center, float radius,
         std::shared_ptr<Material> material);

  std::optional<HitRecord> hit(const Ray &ray, Interval ray_t) const override;
};
