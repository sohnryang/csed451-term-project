#include "scene.hh"

#include <stdexcept>

#include <glm/glm.hpp>

#include <glm/ext/matrix_transform.hpp>

gpu::Material gpu::Material::from_disk_pair(
    const glm::vec3 &source_origin, const glm::vec3 &source_normal,
    const glm::vec3 &destination_origin, const glm::vec3 &destination_normal) {
  const auto translate_mat_before =
      glm::translate(glm::mat4(1), -source_origin);
  const auto translate_mat_after =
      glm::translate(glm::mat4(1), destination_origin);
  const auto displacement = destination_origin - source_origin,
             axis =
                 glm::normalize(glm::cross(source_normal, destination_normal));
  const auto angle = std::acos(glm::dot(glm::normalize(source_normal),
                                        glm::normalize(destination_normal)));
  auto rotation_mat = glm::rotate(glm::mat4(1), angle, axis);
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

  return {.kind = gpu::MaterialKind::PORTAL,
          .translation_mat1 = translate_mat_before,
          .translation_mat2 = translate_mat_after,
          .rotation_mat = rotation_mat};
}
