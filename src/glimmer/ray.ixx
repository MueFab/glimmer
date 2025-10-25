module;
#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <limits>

export module glimmer.ray;

import glimmer.vector;

namespace glimmer {
    /**
     * @brief Ray in 3D space parameterized as r(t) = origin + t * direction.
     * @tparam T arithmetic scalar type
     * @details Stores origin, direction, and a valid parameter interval [tmin, tmax].
     */
    export template <Arithmetic T>
    class Ray {
    public:
        using value_type = T;

        /** @brief Default constructs a ray at origin with +Z direction and [0, inf) range. */
        constexpr Ray() noexcept
            : o_{T{0}, T{0}, T{0}}, d_{T{0}, T{0}, T{1}}, tmin_{T{0}}, tmax_{infinity()} {}

        /**
         * @brief Constructs a ray.
         * @param origin ray origin
         * @param direction ray direction (may be non-normalized)
         * @param tmin minimum valid t (default 0)
         * @param tmax maximum valid t (default +inf)
         */
        constexpr Ray(const Vector<T,3>& origin, const Vector<T,3>& direction,
                      T tmin = T{0}, T tmax = infinity()) noexcept
            : o_{origin}, d_{direction}, tmin_{tmin}, tmax_{tmax} {}

        /** @brief Access origin. */
        [[nodiscard]] constexpr const Vector<T,3>& origin() const noexcept { return o_; }
        /** @brief Access direction. */
        [[nodiscard]] constexpr const Vector<T,3>& direction() const noexcept { return d_; }
        /** @brief Access tmin. */
        [[nodiscard]] constexpr T tmin() const noexcept { return tmin_; }
        /** @brief Access tmax. */
        [[nodiscard]] constexpr T tmax() const noexcept { return tmax_; }

        /** @brief Compute point at parameter t: o + t d. */
        [[nodiscard]] constexpr Vector<T,3> at(T t) const noexcept { return o_ + d_ * t; }

        /** @brief Returns a copy with a normalized direction (no-op if zero length). */
        [[nodiscard]] constexpr Ray normalized_dir() const noexcept {
            if (auto n = d_.norm(); n != T{0}) {
                if constexpr (std::is_floating_point_v<T>) return Ray{o_, d_ / n, tmin_, tmax_};
                else return Ray{o_, Vector<T,3>{static_cast<T>(d_[0]/n), static_cast<T>(d_[1]/n), static_cast<T>(d_[2]/n)}, tmin_, tmax_};
            }
            return *this;
        }

        /** @brief Returns true if tmin <= tmax. */
        [[nodiscard]] constexpr bool is_valid() const noexcept { return !(tmax_ < tmin_); }

        /** @brief Sets the parameter interval. */
        constexpr void set_range(T tmin, T tmax) noexcept { tmin_ = tmin; tmax_ = tmax; }

    private:
        static constexpr T infinity() noexcept {
            if constexpr (std::is_floating_point_v<T>) return std::numeric_limits<T>::infinity();
            else return std::numeric_limits<T>::max();
        }

        Vector<T,3> o_{};
        Vector<T,3> d_{0,0,1};
        T tmin_{0};
        T tmax_{infinity()};
    };

    /** @brief Streams a ray as {o=[x,y,z], d=[x,y,z], t=[tmin,tmax]}. */
    export template <Arithmetic T>
    inline std::ostream& operator<<(std::ostream& os, const Ray<T>& r) {
        os << "{o=" << r.origin() << ", d=" << r.direction() << ", t=[" << r.tmin() << ", " << r.tmax() << "]}";
        return os;
    }
}
