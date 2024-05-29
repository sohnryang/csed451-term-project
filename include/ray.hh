#pragma once

#include <glm/glm.hpp>

class Ray {
private:
  glm::vec3 _origin;
  glm::vec3 _direction;

public:
  Ray(const Ray &) = default;
  Ray(Ray &&) = default;
  Ray &operator=(const Ray &) = default;
  Ray &operator=(Ray &&) = default;

  Ray();
  Ray(const glm::vec3 origin, const glm::vec3 direction);

  glm::vec3 &origin();
  const glm::vec3 &origin() const;
  glm::vec3 &direction();
  const glm::vec3 &direction() const;

  glm::vec3 at(float t) const;
};
