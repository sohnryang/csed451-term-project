#include "interval.hh"

#include <cmath>

Interval::Interval() : lo(+INFINITY), hi(-INFINITY) {}

Interval::Interval(float lo, float hi) : lo(lo), hi(hi) {}

float Interval::length() const { return hi - lo; }

bool Interval::contains(float x) const { return lo <= x & x <= hi; }

bool Interval::surrounds(float x) const { return lo < x & x < hi; }

const Interval Interval::empty = Interval(+INFINITY, -INFINITY);
const Interval Interval::universe = Interval(-INFINITY, +INFINITY);
