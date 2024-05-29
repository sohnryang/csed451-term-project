#pragma once

struct Interval {
  float lo;
  float hi;

  Interval();
  Interval(float lo, float hi);
  Interval(const Interval &) = default;
  Interval(Interval &&) = default;
  Interval &operator=(const Interval &) = default;
  Interval &operator=(Interval &&) = default;

  float length() const;
  bool contains(float x) const;
  bool surrounds(float x) const;
  float clamp(float x) const;

  static const Interval empty, universe;
};
