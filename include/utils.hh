#pragma once

#include <random>

inline float random_float() {
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  static std::mt19937 gen;
  return dist(gen);
}

inline float random_float(float lo, float hi) {
  return lo + (hi - lo) * random_float();
}
