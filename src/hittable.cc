#include "hittable.hh"

#include "ray.hh"

#include <glm/glm.hpp>

HitRecord::HitRecord(const Ray &ray, const glm::vec3 &outward_normal, float t)
    : t(t) {
  point = ray.at(t);
  front_face = glm::dot(ray.direction(), outward_normal) < 0;
  normal = front_face ? outward_normal : -outward_normal;
}
