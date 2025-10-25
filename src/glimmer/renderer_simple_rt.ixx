module;
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <thread>
#include <vector>

export module glimmer.renderer_simple_rt;

import glimmer.vector;
import glimmer.color;
import glimmer.ray;
import glimmer.camera;
import glimmer.scene;
import glimmer.scene_object;
import glimmer.geometry;
import glimmer.image;
import glimmer.material;
import glimmer.renderer; // base interface

namespace glimmer {
    /**
     * @file
     * @brief Simple single-bounce ray-casting renderer implementation.
     */

    /**
     * @brief Minimal reference renderer implementing the Renderer<T> interface.
     * @tparam T arithmetic scalar type (float/double recommended)
     */
    export template <Arithmetic T>
    class RendererSimpleRT : public Renderer<T> {
    public:
        using Color3 = Color<T,3>;

        [[nodiscard]] Color3 trace_ray(const Scene<T>& scene, const Ray<T>& ray) const noexcept override {
            // Iterate objects and track closest hit
            T best_t = ray.tmax();
            Vector<T,3> best_n{};
            const Material<T>* best_m = nullptr;
            bool any_hit = false;
            for (const auto& obj : scene.objects()) {
                // Early AABB reject in world space via cached aabb in SceneObject
                if (auto box_hit = obj.aabb().intersect(ray); !box_hit.has_value()) {
                    continue;
                }
                if (auto h = obj.intersect(ray)) {
                    if (h->t >= ray.tmin() && h->t <= best_t) {
                        any_hit = true;
                        best_t = h->t;
                        best_n = h->normal;
                        best_m = &obj.material();
                    }
                }
            }
            if (!any_hit) return scene.background();
            // Simple shading
            const Vector<T,3> L = Vector<T,3>{ T{1}, T{1}, T{1} }.normalized();
            const T ndotl = std::max<T>(T{0}, dot(best_n.normalized(), L));
            Color3 emission = (best_m ? best_m->radiance() * best_m->emission() : Color3::zeros());
            Color3 diffuse = (best_m ? best_m->albedo() * ndotl : Color3::zeros());
            return emission + diffuse;
        }

        void render(const Scene<T>& scene, Image<T,3>& out, std::size_t width, std::size_t height) const override {
            if (out.width() != width || out.height() != height) {
                out.resize(width, height, Color<T,3>{T{0},T{0},T{0}});
            }
            if (width == 0 || height == 0) return;
            const auto& cam = scene.camera();

            const std::size_t hw = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
            const std::size_t thread_count = std::min<std::size_t>(hw, height);
            std::vector<std::thread> threads;
            threads.reserve(thread_count);

            auto worker = [&](std::size_t y_begin, std::size_t y_end) noexcept {
                for (std::size_t y = y_begin; y < y_end; ++y) {
                    for (std::size_t x = 0; x < width; ++x) {
                        Ray<T> ray = cam.generate_ray(static_cast<T>(x), static_cast<T>(y), width, height);
                        out(x,y) = trace_ray(scene, ray);
                    }
                }
            };

            // Split rows into contiguous blocks per thread
            const std::size_t rows_per_thread = height / thread_count;
            const std::size_t remainder = height % thread_count;
            std::size_t y0 = 0;
            for (std::size_t t = 0; t < thread_count; ++t) {
                const std::size_t count = rows_per_thread + (t < remainder ? 1 : 0);
                const std::size_t y1 = y0 + count;
                threads.emplace_back(worker, y0, y1);
                y0 = y1;
            }
            for (auto& th : threads) th.join();
        }
    };
}
