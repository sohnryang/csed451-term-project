#include "material.hh"

#include "hittable.hh"
#include "ray.hh"
#include "utils.hh"

#include <algorithm>
#include <cmath>
#include <cstdlib>
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

Metal::Metal(const glm::vec3 &albedo, float fuzz)
    : _albedo(albedo), _fuzz(fuzz) {}

std::optional<std::pair<Ray, glm::vec3>>
Metal::scatter(const Ray &ray_in, const HitRecord &record) const {
  const auto reflected =
      glm::normalize(glm::reflect(ray_in.direction(), record.normal)) +
      (_fuzz * random_unit_vector());
  const auto scattered = Ray(record.point, reflected);
  if (glm::dot(scattered.direction(), record.normal) > 0)
    return std::make_pair(scattered, _albedo);
  else
    return {};
}

float Dielectric::_reflectance(float cosine, float refraction_index) const {
  const auto r0 =
      std::pow((1 - refraction_index) / (1 + refraction_index), 2.0f);
  return r0 + (1 - r0) * std::pow(1 - cosine, 5.0f);
}

Dielectric::Dielectric(float refraction_index)
    : _refraction_index(refraction_index) {}

std::optional<std::pair<Ray, glm::vec3>>
Dielectric::scatter(const Ray &ray_in, const HitRecord &record) const {
  const auto ri =
      record.front_face ? 1.0f / _refraction_index : _refraction_index;
  const auto unit_direction = glm::normalize(ray_in.direction());

  const auto cos_theta =
                 std::min(glm::dot(-unit_direction, record.normal), 1.0f),
             sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

  glm::vec3 direction;
  if (ri * sin_theta > 1.0f || _reflectance(cos_theta, ri) > random_float())
    direction = glm::reflect(unit_direction, record.normal);
  else {
    const auto r_out_perp = ri * (unit_direction + cos_theta * record.normal),
               r_out_parallel = -std::sqrt(std::fabs(
                                    1.0f - glm::dot(r_out_perp, r_out_perp))) *
                                record.normal;
    direction = r_out_perp + r_out_parallel;
  }

  const auto scattered = Ray(record.point, direction);
  return std::make_pair(scattered, glm::vec3(1, 1, 1));
}
