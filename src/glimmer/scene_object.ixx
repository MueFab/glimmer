module;
#include <memory>
#include <optional>
#include <type_traits>

export module glimmer.scene_object;

import glimmer.vector;
import glimmer.matrix;
import glimmer.transform;
import glimmer.geometry;
import glimmer.ray;
import glimmer.aabb;
import glimmer.material;

namespace glimmer {
    /**
     * @brief A renderable scene object combining geometry, material, and transform with a cached world-space AABB.
     * @tparam T arithmetic scalar type (float/double recommended)
     */
    export template <Arithmetic T>
    class SceneObject {
    public:
        using GeoPtr = std::shared_ptr<const Geometry<T>>;

        /** @brief Constructs an empty object (no geometry). */
        SceneObject() { cache_from_transform_(); }

        /**
         * @brief Constructs a scene object from geometry, material, and transform.
         * @param geom shared ownership pointer to immutable geometry
         * @param material material parameters
         * @param xform object-to-world transform
         */
        SceneObject(GeoPtr geom, const Material<T>& material, const Transform<T>& xform)
            : geom_{std::move(geom)}, mat_{material}, xf_{xform} {
            cache_from_transform_();
            update_aabb();
        }

        /** @brief Access geometry pointer (may be null). */
        [[nodiscard]] const GeoPtr& geometry() const noexcept { return geom_; }
        /** @brief Access material parameters. */
        [[nodiscard]] const Material<T>& material() const noexcept { return mat_; }
        /** @brief Access transform. */
        [[nodiscard]] const Transform<T>& transform() const noexcept { return xf_; }

        /** @brief Sets a new transform and refreshes cached AABB. */
        void set_transform(const Transform<T>& xform) { xf_ = xform; cache_from_transform_(); update_aabb(); }
        /** @brief Sets a new material. */
        void set_material(const Material<T>& m) noexcept { mat_ = m; }

        /** @brief World-space axis-aligned bounding box (cached). */
        [[nodiscard]] const AABB<T>& aabb() const noexcept { return aabb_world_; }

        /**
         * @brief Ray intersection in world space.
         * @param ray_w world-space ray
         * @return optional hit (t in world-space parameterization and world-space normal)
         */
        [[nodiscard]] std::optional<typename Geometry<T>::Hit> intersect(const Ray<T>& ray_w) const noexcept {
            if (!geom_) return std::nullopt;
            if (!aabb_hits_(ray_w)) return std::nullopt;

            Ray<T> ray_o = to_object_ray_(ray_w);
            auto local_hit = geom_->intersect(ray_o);
            if (!local_hit) return std::nullopt;

            auto wh = map_hit_to_world_(ray_w, ray_o, *local_hit);
            if (wh.t < ray_w.tmin() || wh.t > ray_w.tmax()) return std::nullopt;
            return wh;
        }

        /** @brief Recomputes and caches the world-space AABB from geometry and transform. */
        void update_aabb() {
            if (!geom_) { aabb_world_ = AABB<T>{}; return; }
            auto box = geom_->aabb();
            if (box.empty()) { aabb_world_ = AABB<T>{}; return; }
            // Transform all 8 corners and expand
            Vector<T,3> mn = box.min();
            Vector<T,3> mx = box.max();
            Vector<T,3> corners[8] = {
                {mn[0], mn[1], mn[2]}, {mx[0], mn[1], mn[2]}, {mn[0], mx[1], mn[2]}, {mn[0], mn[1], mx[2]},
                {mx[0], mx[1], mn[2]}, {mx[0], mn[1], mx[2]}, {mn[0], mx[1], mx[2]}, {mx[0], mx[1], mx[2]}
            };
            AABB<T> world_box;
            for (auto& c : corners) {
                // transform corner using cached matrix via homogeneous coordinates
                Vector<T,4> c4 = to_homogeneous_point(c);
                Vector<T,4> cw = m_world_ * c4;
                world_box.expand(Vector<T,3>{cw[0], cw[1], cw[2]});
            }
            aabb_world_ = world_box;
        }

    private:
        // Fast-path helpers extracted from intersect() for readability and inlining.
        [[nodiscard]] constexpr bool aabb_hits_(const Ray<T>& ray_w) const noexcept {
            auto box_hit = aabb_world_.intersect(ray_w);
            return box_hit.has_value();
        }

        [[nodiscard]] Ray<T> to_object_ray_(const Ray<T>& ray_w) const noexcept {
            const Vector<T,3> o = resize_dim<T,4,3>(inv_world_ * to_homogeneous_point(ray_w.origin()));
            const Vector<T,3> d_obj = resize_dim<T,4,3>(inv_world_ * to_homogeneous_dir(ray_w.direction()));
            const T len = d_obj.norm();
            if (len != T{}) {
                Vector<T,3> d_unit{};
                if constexpr (std::is_floating_point_v<T>) d_unit = d_obj / len;
                else d_unit = Vector<T,3>{ static_cast<T>(d_obj[0]/len), static_cast<T>(d_obj[1]/len), static_cast<T>(d_obj[2]/len) };
                const T tmin = ray_w.tmin() * len;
                const T tmax = ray_w.tmax() * len;
                return Ray<T>{o, d_unit, tmin, tmax};
            } else {
                return Ray<T>{o, d_obj, ray_w.tmin(), ray_w.tmax()};
            }
        }

        [[nodiscard]] constexpr T compute_world_t_(const Ray<T>& ray_w, const Vector<T,3>& p_world) const noexcept {
            const Vector<T,3> w = p_world - ray_w.origin();
            const T denom = dot(ray_w.direction(), ray_w.direction());
            if (denom != T{}) return dot(w, ray_w.direction()) / denom;
            return ray_w.tmax();
        }

        [[nodiscard]] typename Geometry<T>::Hit map_hit_to_world_(const Ray<T>& ray_w,
                                                                  const Ray<T>& ray_o,
                                                                  const typename Geometry<T>::Hit& local_hit) const noexcept {
            const T t_obj = local_hit.t;
            const Vector<T,3> p_obj = ray_o.at(t_obj);
            const Vector<T,4> p4_w = m_world_ * to_homogeneous_point(p_obj);
            const Vector<T,3> p_world{ p4_w[0], p4_w[1], p4_w[2] };
            const Vector<T,3> n_world = normal_it3_ * local_hit.normal;
            const T t_world = compute_world_t_(ray_w, p_world);
            typename Geometry<T>::Hit wh{};
            wh.t = t_world;
            wh.normal = n_world;
            wh.uv = local_hit.uv; // UVs are defined in object space and remain the same under rigid transforms
            return wh;
        }

        // Cache matrices to avoid recomputing in hot paths
        void cache_from_transform_() noexcept {
            m_world_ = xf_.matrix();
            inv_world_ = xf_.inverse_matrix();
            // Precompute 3x3 normal matrix = (inverse)^T upper-left
            normal_it3_ = Matrix<T,3,3>{
                inv_world_(0,0), inv_world_(1,0), inv_world_(2,0),
                inv_world_(0,1), inv_world_(1,1), inv_world_(2,1),
                inv_world_(0,2), inv_world_(1,2), inv_world_(2,2)
            };
        }

        GeoPtr geom_{};
        Material<T> mat_{};
        Transform<T> xf_{}; // object-to-world
        // Cached matrices for fast transformations
        Matrix<T,4,4> m_world_{};
        Matrix<T,4,4> inv_world_{};
        Matrix<T,3,3> normal_it3_{};
        AABB<T> aabb_world_{};
    };
}
