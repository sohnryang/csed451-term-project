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

class Metal : public Material {
private:
  glm::vec3 _albedo;
  float _fuzz;

public:
  Metal() = default;
  Metal(const Metal &) = default;
  Metal(Metal &&) = default;
  Metal &operator=(const Metal &) = default;
  Metal &operator=(Metal &&) = default;

  Metal(const glm::vec3 &albedo, float fuzz);

  std::optional<std::pair<Ray, glm::vec3>>
  scatter(const Ray &ray_in, const HitRecord &record) const override;
};

class Dielectric : public Material {
private:
  float _refraction_index;

  float _reflectance(float cosine, float refraction_index) const;

public:
  Dielectric() = default;
  Dielectric(const Dielectric &) = default;
  Dielectric(Dielectric &&) = default;
  Dielectric &operator=(const Dielectric &) = default;
  Dielectric &operator=(Dielectric &&) = default;

  Dielectric(float refraction_index);

  std::optional<std::pair<Ray, glm::vec3>>
  scatter(const Ray &ray_in, const HitRecord &record) const override;
};
