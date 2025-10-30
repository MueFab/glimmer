module;
#include <type_traits>
#include <optional>
#include <cmath>
#include <limits>

export module glimmer.plane;

import glimmer.vector;
import glimmer.ray;
import glimmer.aabb;
import glimmer.geometry;

namespace glimmer
{
    /**
     * @brief Infinite plane in 3D defined by a point and a normal.
     * @tparam T arithmetic scalar type
     * @details The plane consists of all points p such that dot(n, p - p0) = 0, with n normalized when possible.
     */
    export template <Arithmetic T>
    class Plane : public Geometry<T>
    {
    public:
        using Vector3 = Vector<T, 3>;

        /** @brief Default constructs the plane z=0 with normal +Z. */
        constexpr Plane() noexcept : p0_{T{0}, T{0}, T{0}}, n_{T{0}, T{0}, T{1}}
        {
        }

        /** @brief Constructs a plane from a point and a (not necessarily unit) normal. */
        constexpr Plane(const Vector3& point, const Vector3& normal) noexcept : p0_{point}, n_{normalize_(normal)}
        {
        }

        /** @brief Access point on plane. */
        [[nodiscard]] constexpr const Vector3& point() const noexcept { return p0_; }
        /** @brief Access (unit) normal. */
        [[nodiscard]] constexpr const Vector3& normal() const noexcept { return n_; }

        /** @brief Returns a very large finite AABB to approximate an infinite plane for culling.
         * @details To remain compatible with SceneObject's finite-corner transform, we avoid infinities here.
         */
        [[nodiscard]] AABB<T> aabb() const noexcept override
        {
            const T M = big_extent_();
            const Vector<T, 3> e{M, M, M};
            return AABB<T>{p0_ - e, p0_ + e};
        }

        /**
         * @brief Ray-plane intersection (two-sided).
         * @param ray input ray
         * @return nearest hit within [tmin,tmax] if the ray is not parallel and intersects the plane
         */
        [[nodiscard]] std::optional<typename Geometry<T>::Hit> intersect(const Ray<T>& ray) const noexcept override
        {
            const T denom = dot(n_, ray.direction());
            // Treat near-parallel as no hit
            const T eps = static_cast<T>(1e-8);
            if (denom > -eps && denom < eps) return std::nullopt;
            const T t = dot(p0_ - ray.origin(), n_) / denom;
            if (t < ray.tmin() || t > ray.tmax()) return std::nullopt;
            typename Geometry<T>::Hit h{};
            h.t = t;
            // Keep stored normal orientation (do not flip with respect to ray)
            h.normal = n_;
            return h;
        }

    private:
        static constexpr T big_extent_() noexcept
        {
            if constexpr (std::is_floating_point_v<T>) return static_cast<T>(1e6);
            else return std::numeric_limits<T>::max() / static_cast<T>(4);
        }

        static constexpr Vector3 normalize_(const Vector3& v) noexcept
        {
            const T len = v.norm();
            if (len != T{0})
            {
                if constexpr (std::is_floating_point_v<T>) return v / len;
                else return Vector3{static_cast<T>(v[0] / len), static_cast<T>(v[1] / len), static_cast<T>(v[2] / len)};
            }
            // Fallback to +Z if zero-length provided
            return Vector3{T{0}, T{0}, T{1}};
        }

        Vector3 p0_{};
        Vector3 n_{0, 0, 1};
    };
}
