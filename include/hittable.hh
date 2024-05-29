#pragma once

#include "interval.hh"
#include "ray.hh"

#include <optional>

#include <glm/glm.hpp>

struct HitRecord {
  glm::vec3 point;
  glm::vec3 normal;
  float t;
  bool front_face;

  HitRecord() = default;
  HitRecord(const HitRecord &) = default;
  HitRecord(HitRecord &&) = default;
  HitRecord &operator=(const HitRecord &) = default;
  HitRecord &operator=(HitRecord &&) = default;

  HitRecord(const Ray &ray, const glm::vec3 &outward_normal, float t);
};

class Hittable {
public:
  virtual ~Hittable() = default;
  virtual std::optional<HitRecord> hit(const Ray &ray,
                                       Interval ray_t) const = 0;
};
