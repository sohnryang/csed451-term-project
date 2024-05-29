#include "material.hh"

#include "hittable.hh"
#include "ray.hh"
#include "utils.hh"

#include <optional>
#include <utility>

#include <glm/glm.hpp>

std::optional<std::pair<Ray, glm::vec3>>
Material::scatter(const Ray &ray_in, const HitRecord &record) const {
  return std::nullopt;
}

Lambertian::Lambertian(const glm::vec3 &albedo) : _albedo(albedo) {}

std::optional<std::pair<Ray, glm::vec3>>
Lambertian::scatter(const Ray &ray_in, const HitRecord &record) const {
  const auto scatter_direction = record.normal + random_unit_vector();
  const auto scattered = Ray(record.point, scatter_direction);
  return std::make_pair(scattered, _albedo);
}

Metal::Metal(const glm::vec3 &albedo) : _albedo(albedo) {}

std::optional<std::pair<Ray, glm::vec3>>
Metal::scatter(const Ray &ray_in, const HitRecord &record) const {
  const auto reflected = glm::reflect(ray_in.direction(), record.normal);
  const auto scattered = Ray(record.point, reflected);
  return std::make_pair(scattered, _albedo);
}
