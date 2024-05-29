#include "hittable_list.hh"

#include "hittable.hh"
#include "interval.hh"

#include <optional>

std::optional<HitRecord> HittableList::hit(const Ray &ray,
                                           Interval ray_t) const {
  std::optional<HitRecord> current_record = {};
  auto current_closest = ray_t.hi;

  for (const auto &hittable : hittables) {
    const auto hit_result =
        hittable->hit(ray, Interval(ray_t.lo, current_closest));
    if (!hit_result.has_value())
      continue;

    current_record = hit_result;
    current_closest = hit_result->t;
  }

  return current_record;
}
