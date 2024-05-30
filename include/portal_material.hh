#pragma once

#include "hittable.hh"
#include "material.hh"

#include <glm/glm.hpp>

class PortalMaterial : public Material {
private:
  glm::mat4 rotation_mat;
  glm::mat4 translate_mat_before, translate_mat_after;

public:
  PortalMaterial() = default;
  PortalMaterial(const PortalMaterial &) = default;
  PortalMaterial(PortalMaterial &&) = default;
  PortalMaterial &operator=(const PortalMaterial &) = default;
  PortalMaterial &operator=(PortalMaterial &&) = default;

  PortalMaterial(const glm::vec3 &source_origin, const glm::vec3 &source_normal,
                 const glm::vec3 &destination_origin,
                 const glm::vec3 &destination_normal);

  std::optional<std::pair<Ray, glm::vec3>>
  scatter(const Ray &ray_in, const HitRecord &record) const override;
};
