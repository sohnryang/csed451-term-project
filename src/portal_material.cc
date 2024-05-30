#include "portal_material.hh"

#include <cmath>
#include <utility>

#include <glm/glm.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

PortalMaterial::PortalMaterial(const glm::vec3 &source_origin,
                               const glm::vec3 &source_normal,
                               const glm::vec3 &destination_origin,
                               const glm::vec3 &destination_normal) {
  translate_mat_before = glm::translate(glm::mat4(1), -source_origin);
  translate_mat_after = glm::translate(glm::mat4(1), destination_origin);
  const auto displacement = destination_origin - source_origin,
             axis =
                 glm::normalize(glm::cross(source_normal, destination_normal));
  const auto angle = std::acos(glm::dot(glm::normalize(source_normal),
                                        glm::normalize(destination_normal)));
  rotation_mat = glm::rotate(glm::mat4(1), angle, axis);
  if (glm::any(glm::isnan(rotation_mat * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)))) {
    if (angle < 0.1f)
      rotation_mat = glm::mat4(1.0f);
    else if (angle > glm::pi<float>() - 0.1f)
      rotation_mat = glm::rotate(
          glm::mat4(1), angle,
          glm::cross(source_normal, glm::vec3(source_normal.z, source_normal.x,
                                              source_normal.y)));
    else
      throw std::invalid_argument("invalid angle");
  }
}

std::optional<std::pair<Ray, glm::vec3>>
PortalMaterial::scatter(const Ray &ray_in, const HitRecord &record) const {
  const auto cp = glm::vec3(translate_mat_before *
                            glm::vec4(record.point, 1.0f)),
             cp_rotated = glm::vec3(rotation_mat * glm::vec4(cp, 0.0f)),
             origin =
                 glm::vec3(translate_mat_after * glm::vec4(cp_rotated, 1.0f)),
             direction =
                 glm::vec3(rotation_mat * glm::vec4(ray_in.direction(), 0.0f));
  const auto scattered = Ray(origin, direction);
  return std::make_pair(scattered, glm::vec3(1, 1, 1));
}
