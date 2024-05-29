#include "interval.hh"

#include <algorithm>
#include <cmath>

Interval::Interval() : lo(+INFINITY), hi(-INFINITY) {}

Interval::Interval(float lo, float hi) : lo(lo), hi(hi) {}

float Interval::length() const { return hi - lo; }

bool Interval::contains(float x) const { return lo <= x & x <= hi; }

bool Interval::surrounds(float x) const { return lo < x & x < hi; }

float Interval::clamp(float x) const { return std::clamp(x, lo, hi); }

const Interval Interval::empty = Interval(+INFINITY, -INFINITY);
const Interval Interval::universe = Interval(-INFINITY, +INFINITY);
