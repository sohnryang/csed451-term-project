#pragma once

#include "hittable.hh"

#include <memory>
#include <vector>

struct HittableList : public Hittable {
  std::vector<std::shared_ptr<Hittable>> hittables;

  HittableList() = default;
  HittableList(const HittableList &) = default;
  HittableList(HittableList &&) = default;

  std::optional<HitRecord> hit(const Ray &ray, Interval ray_t) const override;
};
