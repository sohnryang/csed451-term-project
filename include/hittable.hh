#pragma once

#include "interval.hh"
#include "material.hh"
#include "ray.hh"

#include <memory>
#include <optional>

#include <glm/glm.hpp>

struct HitRecord {
  glm::vec3 point;
  glm::vec3 normal;
  float t;
  bool front_face;
  std::shared_ptr<Material> material;

  HitRecord() = default;
  HitRecord(const HitRecord &) = default;
  HitRecord(HitRecord &&) = default;
  HitRecord &operator=(const HitRecord &) = default;
  HitRecord &operator=(HitRecord &&) = default;

  HitRecord(const Ray &ray, const glm::vec3 &outward_normal, float t,
            std::shared_ptr<Material> material);
};

class Hittable {
public:
  virtual ~Hittable() = default;
  virtual std::optional<HitRecord> hit(const Ray &ray,
                                       Interval ray_t) const = 0;
};
