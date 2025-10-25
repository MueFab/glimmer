module;
#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <ostream>

export module glimmer.aabb;

import glimmer.vector;
import glimmer.ray;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing an axis-aligned bounding box (AABB) utility for 3D.
     */

    /**
     * @brief Axis-aligned bounding box in 3D.
     * @tparam T arithmetic scalar type
     * @details Stores minimum and maximum corners with the invariant min[i] <= max[i] for valid boxes.
     * An empty box is represented by min > max (min initialized to +inf, max to -inf).
     */
    export template <Arithmetic T>
    class AABB {
    public:
        using value_type = T;

        /** @brief Constructs an empty box. */
        constexpr AABB() noexcept : min_{ pos_inf(), pos_inf(), pos_inf() }, max_{ neg_inf(), neg_inf(), neg_inf() } {}

        /** @brief Constructs a box from minimum and maximum corners. */
        constexpr AABB(const Vector<T,3>& min_pt, const Vector<T,3>& max_pt) noexcept : min_{min_pt}, max_{max_pt} {}

        /** @brief Constructs a box from center and half-extent (non-negative). */
        static constexpr AABB from_center_extent(const Vector<T,3>& c, const Vector<T,3>& e) noexcept {
            return AABB{ Vector<T,3>{ c[0]-e[0], c[1]-e[1], c[2]-e[2] },
                         Vector<T,3>{ c[0]+e[0], c[1]+e[1], c[2]+e[2] } };
        }

        /** @brief Returns true if the box is empty (no valid extent). */
        [[nodiscard]] constexpr bool empty() const noexcept {
            return !(min_[0] <= max_[0] && min_[1] <= max_[1] && min_[2] <= max_[2]);
        }

        /** @brief Returns true if the box is valid (non-empty). */
        [[nodiscard]] constexpr bool valid() const noexcept { return !empty(); }

        /** @brief Access minimum corner. */
        [[nodiscard]] constexpr const Vector<T,3>& min() const noexcept { return min_; }
        /** @brief Access maximum corner. */
        [[nodiscard]] constexpr const Vector<T,3>& max() const noexcept { return max_; }

        /** @brief Resets to empty. */
        constexpr void clear() noexcept {
            min_ = { pos_inf(), pos_inf(), pos_inf() };
            max_ = { neg_inf(), neg_inf(), neg_inf() };
        }

        /** @brief Returns the center of the box (undefined for empty). */
        [[nodiscard]] constexpr Vector<T,3> center() const noexcept {
            return Vector<T,3>{ (min_[0]+max_[0])/two(), (min_[1]+max_[1])/two(), (min_[2]+max_[2])/two() };
        }

        /** @brief Returns the size along each axis (max - min). */
        [[nodiscard]] constexpr Vector<T,3> extent() const noexcept {
            return Vector<T,3>{ max_[0]-min_[0], max_[1]-min_[1], max_[2]-min_[2] };
        }

        /** @brief Returns the diagonal length vector (alias of extent). */
        [[nodiscard]] constexpr Vector<T,3> diagonal() const noexcept { return extent(); }

        /** @brief Surface area = 2*(dx*dy + dy*dz + dz*dx); returns 0 for empty boxes. */
        [[nodiscard]] T surface_area() const noexcept {
            if (empty()) return T{0};
            auto e = extent();
            return static_cast<T>(2) * (e[0]*e[1] + e[1]*e[2] + e[2]*e[0]);
        }

        /** @brief Expands the box to include a point. */
        constexpr void expand(const Vector<T,3>& p) noexcept {
            if (empty()) {
                min_ = max_ = p;
            } else {
                for (std::size_t i=0;i<3;++i) {
                    if (p[i] < min_[i]) min_[i] = p[i];
                    if (p[i] > max_[i]) max_[i] = p[i];
                }
            }
        }

        /** @brief Expands the box to include another box. */
        constexpr void expand(const AABB& b) noexcept {
            if (b.empty()) return;
            if (empty()) { min_ = b.min_; max_ = b.max_; return; }
            for (std::size_t i=0;i<3;++i) {
                if (b.min_[i] < min_[i]) min_[i] = b.min_[i];
                if (b.max_[i] > max_[i]) max_[i] = b.max_[i];
            }
        }

        /** @brief Component-wise union (this ∪ b). */
        [[nodiscard]] constexpr AABB united(const AABB& b) const noexcept {
            if (empty()) return b;
            if (b.empty()) return *this;
            AABB r = *this; r.expand(b); return r;
        }

        /** @brief Returns true if the point lies inside the box (inclusive). */
        [[nodiscard]] constexpr bool contains(const Vector<T,3>& p) const noexcept {
            if (empty()) return false;
            return (p[0] >= min_[0] && p[0] <= max_[0]) &&
                   (p[1] >= min_[1] && p[1] <= max_[1]) &&
                   (p[2] >= min_[2] && p[2] <= max_[2]);
        }

        /** @brief Returns true if the box fully contains b. */
        [[nodiscard]] constexpr bool contains(const AABB& b) const noexcept {
            if (empty() || b.empty()) return false;
            return (b.min_[0] >= min_[0] && b.max_[0] <= max_[0]) &&
                   (b.min_[1] >= min_[1] && b.max_[1] <= max_[1]) &&
                   (b.min_[2] >= min_[2] && b.max_[2] <= max_[2]);
        }

        /** @brief Returns true if this box overlaps b (non-empty intersection). */
        [[nodiscard]] constexpr bool overlaps(const AABB& b) const noexcept {
            if (empty() || b.empty()) return false;
            return (min_[0] <= b.max_[0] && max_[0] >= b.min_[0]) &&
                   (min_[1] <= b.max_[1] && max_[1] >= b.min_[1]) &&
                   (min_[2] <= b.max_[2] && max_[2] >= b.min_[2]);
        }

        /** @brief Ray intersection using the slabs method.
         * @param ray input ray with range [tmin, tmax]
         * @return optional {t_near, t_far} if intersection occurs within range
         */
        struct RayHit { T t_near{}; T t_far{}; };
        [[nodiscard]] std::optional<RayHit> intersect(const Ray<T>& ray) const noexcept {
            if (empty()) return std::nullopt;
            T t0 = ray.tmin();
            T t1 = ray.tmax();
            for (std::size_t i=0;i<3;++i) {
                const T di = ray.direction()[i];
                const T oi = ray.origin()[i];
                if (di == T{0}) {
                    // Ray is parallel to slab; must be within bounds to intersect
                    if (oi < min_[i] || oi > max_[i]) return std::nullopt;
                    // Else, no restriction from this axis
                    continue;
                }
                const T invD = T{1} / di;
                T t_near = (min_[i] - oi) * invD;
                T t_far  = (max_[i] - oi) * invD;
                if (t_near > t_far) { auto tmp = t_near; t_near = t_far; t_far = tmp; }
                if (t_near > t0) t0 = t_near;
                if (t_far < t1) t1 = t_far;
                if (t1 < t0) return std::nullopt;
            }
            return RayHit{t0, t1};
        }

        /** @brief Exact equality comparison. */
        [[nodiscard]] constexpr bool operator==(const AABB& rhs) const noexcept { return min_ == rhs.min_ && max_ == rhs.max_; }
        /** @brief Logical negation of operator==. */
        [[nodiscard]] constexpr bool operator!=(const AABB& rhs) const noexcept { return !(*this == rhs); }

    private:
        static constexpr T pos_inf() noexcept {
            if constexpr (std::is_floating_point_v<T>) return std::numeric_limits<T>::infinity();
            else return std::numeric_limits<T>::max();
        }
        static constexpr T neg_inf() noexcept {
            if constexpr (std::is_floating_point_v<T>) return -std::numeric_limits<T>::infinity();
            else return std::numeric_limits<T>::lowest();
        }
        static constexpr T two() noexcept { return static_cast<T>(2); }
        static constexpr T static_castT(int v) noexcept { return static_cast<T>(v); }

        Vector<T,3> min_{};
        Vector<T,3> max_{};
    };

    /** @brief Streams an AABB as {min=[...], max=[...]}. */
    export template <Arithmetic T>
    inline std::ostream& operator<<(std::ostream& os, const AABB<T>& b) {
        os << "{min=" << b.min() << ", max=" << b.max() << "}";
        return os;
    }
}
