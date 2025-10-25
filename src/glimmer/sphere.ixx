module;
#include <cmath>
#include <cstddef>
#include <ostream>
#include <type_traits>
#include <optional>

export module glimmer.sphere;

import glimmer.vector;
import glimmer.ray;
import glimmer.aabb;
import glimmer.geometry;

namespace glimmer {
    /**
     * @brief Sphere primitive in 3D.
     * @tparam T arithmetic scalar type
     */
    export template <Arithmetic T>
    class Sphere : public Geometry<T> {
    public:
        /** @brief Construct unit sphere at origin. */
        constexpr Sphere() noexcept : c_{T{0},T{0},T{0}}, r_{T{1}} {}
        /** @brief Construct from center and radius. */
        constexpr Sphere(const Vector<T,3>& center, T radius) noexcept : c_{center}, r_{radius} {}

        /** @brief Center of sphere. */
        [[nodiscard]] constexpr const Vector<T,3>& center() const noexcept { return c_; }
        /** @brief Radius of sphere. */
        [[nodiscard]] constexpr T radius() const noexcept { return r_; }

        /** @brief Point containment test (inclusive). */
        [[nodiscard]] bool contains(const Vector<T,3>& p) const noexcept {
            auto v = p - c_;
            auto d2 = dot(v, v);
            return d2 <= r_ * r_;
        }

        /**
         * @brief Axis-aligned bounding box of the sphere.
         * @return AABB with min = center - (r,r,r) and max = center + (r,r,r).
         */
        [[nodiscard]] AABB<T> aabb() const noexcept override {
            const Vector<T,3> e{r_, r_, r_};
            return AABB<T>{c_ - e, c_ + e};
        }

        /**
         * @brief Ray-sphere intersection.
         * @param ray input ray
         * @return optional hit record with parameter t and outward unit normal; empty if no hit in [tmin,tmax]
         * @details Uses quadratic solution. Chooses the smallest t >= ray.tmin() and <= ray.tmax().
         */
        [[nodiscard]] std::optional<typename Geometry<T>::Hit> intersect(const Ray<T>& ray) const noexcept override {
            const Vector<T,3> oc = ray.origin() - c_;
            const T a = dot(ray.direction(), ray.direction());
            const T b = T{2} * dot(oc, ray.direction());
            const T c = dot(oc, oc) - r_ * r_;
            const T disc = b*b - T{4}*a*c;
            if (disc < T{0}) return std::nullopt;
            const T sqrt_disc = static_cast<T>(std::sqrt(static_cast<long double>(disc)));
            // two roots
            T t0 = (-b - sqrt_disc) / (T{2} * a);
            T t1 = (-b + sqrt_disc) / (T{2} * a);
            if (t0 > t1) { auto tmp = t0; t0 = t1; t1 = tmp; }
            T t_candidate = t0;
            if (t_candidate < ray.tmin() || t_candidate > ray.tmax()) {
                t_candidate = t1;
                if (t_candidate < ray.tmin() || t_candidate > ray.tmax()) return std::nullopt;
            }
            Vector<T,3> p = ray.at(t_candidate);
            Vector<T,3> n = p - c_;
            typename Geometry<T>::Hit hit{};
            hit.t = t_candidate;
            auto nn = n.norm();
            if (nn != T{0}) {
                if constexpr (std::is_floating_point_v<T>) hit.normal = n / nn;
                else hit.normal = Vector<T,3>{ static_cast<T>(n[0]/nn), static_cast<T>(n[1]/nn), static_cast<T>(n[2]/nn) };
            } else {
                hit.normal = Vector<T,3>{T{0},T{0},T{1}};
            }
            return hit;
        }

    private:
        Vector<T,3> c_{};
        T r_{1};
    };
}
