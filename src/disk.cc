#include "disk.hh"

#include "hittable.hh"
#include "material.hh"

#include <cmath>
#include <memory>
#include <optional>

#include <glm/glm.hpp>

Disk::Disk(const glm::vec3 &center, const glm::vec3 &normal, float radius,
           std::shared_ptr<Material> material)
    : center(center), normal(normal), radius(radius), material(material) {}

std::optional<HitRecord> Disk::hit(const Ray &ray, Interval ray_t) const {
  const auto oc = center - ray.origin();
  const auto signed_dist = glm::dot(oc, normal);
  if (signed_dist <= 0)
    return {};

  const auto direction_normal = glm::dot(normal, ray.direction()) * normal;
  const auto root = std::fabs(signed_dist / glm::dot(ray.direction(), normal));
  if (!ray_t.surrounds(root))
    return {};

  const auto point = ray.at(root);
  if (glm::distance(point, center) > radius)
    return {};

  return HitRecord(ray, normal, root, material);
}
