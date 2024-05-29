#pragma once

#include "ray.hh"

#include <optional>
#include <utility>

#include <glm/glm.hpp>

class HitRecord;

class Material {
public:
  virtual ~Material() = default;

  virtual std::optional<std::pair<Ray, glm::vec3>>
  scatter(const Ray &ray_in, const HitRecord &record) const;
};

class Lambertian : public Material {
private:
  glm::vec3 _albedo;

public:
  Lambertian() = default;
  Lambertian(const Lambertian &) = default;
  Lambertian(Lambertian &&) = default;
  Lambertian &operator=(const Lambertian &) = default;
  Lambertian &operator=(Lambertian &&) = default;

  Lambertian(const glm::vec3 &albedo);

  std::optional<std::pair<Ray, glm::vec3>>
  scatter(const Ray &ray_in, const HitRecord &record) const override;
};