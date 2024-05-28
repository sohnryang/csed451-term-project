#include "ray.hh"

#include <glm/glm.hpp>

Ray::Ray() : Ray(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)) {}

Ray::Ray(const glm::vec3 origin, const glm::vec3 direction)
    : _origin(origin), _direction(direction) {}

glm::vec3 Ray::at(float t) { return _origin + t * _direction; }
