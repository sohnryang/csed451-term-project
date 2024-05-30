#pragma once

#include "hittable.hh"
#include "interval.hh"
#include "material.hh"
#include "ray.hh"

#include <memory>
#include <optional>

#include <glm/glm.hpp>

struct Disk : public Hittable {
  glm::vec3 center;
  glm::vec3 normal;
  float radius;
  std::shared_ptr<Material> material;

  Disk(const glm::vec3 &center, const glm::vec3 &normal, float radius,
       std::shared_ptr<Material> material);

  std::optional<HitRecord> hit(const Ray &ray, Interval ray_t) const override;
};
