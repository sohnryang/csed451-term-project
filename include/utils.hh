#pragma once

#include <cmath>
#include <random>

#include <glm/glm.hpp>

inline float random_float() {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  static std::mt19937 gen;
  return dist(gen);
}

inline float random_float(float lo, float hi) {
  return lo + (hi - lo) * random_float();
}

inline glm::vec3 random_vec() {
  return {random_float(), random_float(), random_float()};
}

inline glm::vec3 random_in_unit_sphere() {
  while (true) {
    const auto v = random_vec();
    if (glm::dot(v, v) < 1)
      return v;
  }
}

inline glm::vec3 random_unit_vector() {
  return glm::normalize(random_in_unit_sphere());
}

inline glm::vec3 random_on_hemisphere(const glm::vec3 &normal) {
  const auto on_unit_sphere = random_unit_vector();
  if (glm::dot(on_unit_sphere, normal) > 0.0f)
    return on_unit_sphere;
  else
    return -on_unit_sphere;
}

inline glm::vec3 random_in_unit_disk() {
  while (true) {
    const auto v = glm::vec3(random_float(), random_float(), 0);
    if (glm::dot(v, v) < 1)
      return v;
  }
}

inline float linear_to_gamma(float linear_component) {
  if (linear_component > 0)
    return std::sqrt(linear_component);
  return 0;
}
