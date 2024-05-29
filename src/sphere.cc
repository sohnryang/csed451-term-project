#include "sphere.hh"

#include "hittable.hh"
#include "interval.hh"
#include "material.hh"
#include "ray.hh"

#include <cmath>
#include <memory>
#include <optional>

#include <glm/glm.hpp>

Sphere::Sphere(const glm::vec3 &center, float radius,
               std::shared_ptr<Material> material)
    : _center(center), _radius(radius), _material(material) {}

std::optional<HitRecord> Sphere::hit(const Ray &ray, Interval ray_t) const {
  const auto oc = _center - ray.origin();
  const auto a = glm::dot(ray.direction(), ray.direction()),
             h = glm::dot(ray.direction(), oc),
             c = glm::dot(oc, oc) - _radius * _radius,
             discriminant = h * h - a * c;
  if (discriminant < 0)
    return std::nullopt;

  const auto sqrtd = std::sqrt(discriminant);

  auto root = (h - sqrtd) / a;
  if (!ray_t.surrounds(root)) {
    root = (h + sqrtd) / a;
    if (!ray_t.surrounds(root))
      return std::nullopt;
  }

  const auto point = ray.at(root);
  const auto outward_normal = (point - _center) / _radius;
  return HitRecord(ray, outward_normal, root, _material);
}
