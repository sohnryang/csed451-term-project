#pragma once

#include <glm/glm.hpp>

class Ray {
private:
  glm::vec3 _origin;
  glm::vec3 _direction;

public:
  Ray();
  Ray(const glm::vec3 origin, const glm::vec3 direction);

  glm::vec3 at(float t);
};
