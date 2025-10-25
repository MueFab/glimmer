module;
#include <optional>

export module glimmer.geometry;

import glimmer.vector;
import glimmer.ray;
import glimmer.aabb;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module defining an abstract geometry interface for ray intersections.
     */

    /**
     * @brief Abstract base for geometric primitives supporting ray intersection and AABB queries.
     * @tparam T arithmetic scalar type
     * @details Provides a common interface so different shapes (Sphere, Mesh, etc.) can be treated uniformly
     * for acceleration structures and renderers. Implementations must return an axis-aligned bounding box and
     * implement ray intersection that returns the nearest hit within the ray's [tmin, tmax] interval, when any.
     */
    export template <Arithmetic T>
    class Geometry {
    public:
        using value_type = T;
        /** @brief Minimal hit record returned by Geometry::intersect. */
        struct Hit { T t{}; Vector<T,3> normal{}; };

        virtual ~Geometry() = default;
        /** @brief Axis-aligned bounding box in object space. */
        [[nodiscard]] virtual AABB<T> aabb() const noexcept = 0;
        /** @brief Ray intersection returning nearest hit within [tmin, tmax], if any. */
        [[nodiscard]] virtual std::optional<Hit> intersect(const Ray<T>& ray) const noexcept = 0;
    };
}
