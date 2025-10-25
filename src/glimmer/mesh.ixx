module;
#include <vector>
#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <optional>

export module glimmer.mesh;

import glimmer.vector;
import glimmer.ray;
import glimmer.aabb;
import glimmer.geometry;

namespace glimmer {
    /**
     * @brief Triangle mesh holding positions and triangle indices.
     * @tparam T arithmetic scalar type
     */
    export template <Arithmetic T>
    class Mesh : public Geometry<T> {
    public:
        struct Triangle { std::size_t i0{}, i1{}, i2{}; };

        /** @brief Adds a vertex position and returns its index. */
        std::size_t add_vertex(const Vector<T,3>& p) { vertices_.push_back(p); return vertices_.size()-1; }
        /** @brief Adds a triangle by vertex indices. */
        void add_triangle(std::size_t i0, std::size_t i1, std::size_t i2) { tris_.push_back({i0,i1,i2}); }

        /** @brief Number of vertices. */
        [[nodiscard]] std::size_t vertex_count() const noexcept { return vertices_.size(); }
        /** @brief Number of triangles. */
        [[nodiscard]] std::size_t triangle_count() const noexcept { return tris_.size(); }

        /** @brief Access vertex by index (unchecked). */
        [[nodiscard]] const Vector<T,3>& vertex(std::size_t i) const noexcept { return vertices_[i]; }
        /** @brief Access triangle by index (unchecked). */
        [[nodiscard]] const Triangle& triangle(std::size_t i) const noexcept { return tris_[i]; }

        /**
         * @brief Computes the axis-aligned bounding box over all vertices.
         * @return AABB spanning all vertex positions; empty if the mesh has no vertices.
         */
        [[nodiscard]] AABB<T> aabb() const noexcept override {
            AABB<T> box;
            for (const auto& v : vertices_) box.expand(v);
            return box;
        }

        /**
         * @brief Ray-mesh intersection (brute-force over triangles, two-sided).
         * @param ray input ray with parameter range
         * @return optional nearest hit information if any triangle is hit within range
         */
        [[nodiscard]] std::optional<typename Geometry<T>::Hit> intersect(const Ray<T>& ray) const noexcept override {
            bool hit_any = false;
            T best_t = ray.tmax();
            typename Geometry<T>::Hit best{};
            for (std::size_t idx=0; idx<tris_.size(); ++idx) {
                const auto& tri = tris_[idx];
                const auto& p0 = vertices_[tri.i0];
                const auto& p1 = vertices_[tri.i1];
                const auto& p2 = vertices_[tri.i2];
                if (auto ih = intersect_triangle(p0,p1,p2, ray)) {
                    const auto& h = *ih;
                    if (h.t >= ray.tmin() && h.t <= best_t) {
                        hit_any = true;
                        best_t = h.t;
                        best.t = h.t;
                        best.normal = h.normal;
                    }
                }
            }
            if (hit_any) return best;
            return std::nullopt;
        }

    private:
        std::vector<Vector<T,3>> vertices_{};
        std::vector<Triangle> tris_{};
    };

    /**
     * @brief Two-sided Möller–Trumbore ray-triangle intersection.
     * @param p0 triangle vertex 0
     * @param p1 triangle vertex 1
     * @param p2 triangle vertex 2
     * @param ray ray with range
     * @return optional hit with t,u,v and outward unit normal; empty if no intersection in range
     */
    export template <Arithmetic T>
    struct TriHit { T t{}; T u{}; T v{}; Vector<T,3> normal{}; };

    export template <Arithmetic T>
    [[nodiscard]] std::optional<TriHit<T>> intersect_triangle(const Vector<T,3>& p0, const Vector<T,3>& p1, const Vector<T,3>& p2,
                                          const Ray<T>& ray) noexcept {
        const Vector<T,3> e1 = p1 - p0;
        const Vector<T,3> e2 = p2 - p0;
        const Vector<T,3> pvec = cross(ray.direction(), e2);
        const T det = dot(e1, pvec);
        const T eps = static_cast<T>(1e-8);
        T inv_det{};
        if (det > eps || det < -eps) inv_det = T{1} / det; else {
            return std::nullopt;
        }
        const Vector<T,3> tvec = ray.origin() - p0;
        const T u = dot(tvec, pvec) * inv_det;
        if (u < T{0} || u > T{1}) return std::nullopt;
        const Vector<T,3> qvec = cross(tvec, e1);
        const T v = dot(ray.direction(), qvec) * inv_det;
        if (v < T{0} || u + v > T{1}) return std::nullopt;
        const T t = dot(e2, qvec) * inv_det;
        if (t < ray.tmin() || t > ray.tmax()) return std::nullopt;
        Vector<T,3> n = cross(e1, e2);
        auto nn = n.norm();
        Vector<T,3> normal{};
        if (nn != T{0}) {
            if constexpr (std::is_floating_point_v<T>) normal = n / nn;
            else normal = Vector<T,3>{ static_cast<T>(n[0]/nn), static_cast<T>(n[1]/nn), static_cast<T>(n[2]/nn) };
        } else normal = Vector<T,3>{T{0},T{0},T{1}};
        return TriHit<T>{t, u, v, normal};
    }
}
