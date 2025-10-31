module;
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <thread>
#include <vector>
#include <random>
#include <cmath>
#include <numbers>
#include <bit>

export module glimmer.renderer_path_tracer;

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

namespace glimmer
{
    /**
     * @file
     * @brief Simple path-tracing renderer with recursive Monte Carlo integration, reflections and refractions.
     */

    namespace detail_pt
    {
        template <Arithmetic T>
        [[nodiscard]] inline Vector<T, 3> reflect(const Vector<T, 3>& v, const Vector<T, 3>& n) noexcept
        {
            return v - n * (T{2} * dot(v, n));
        }

        template <Arithmetic T>
        [[nodiscard]] inline bool refract(const Vector<T, 3>& v, const Vector<T, 3>& n, T eta,
                                          Vector<T, 3>& out) noexcept
        {
            // v: incident direction (unit), n: surface normal (unit), eta = n1/n2
            const T cosi = std::clamp<T>(-dot(v, n), T{-1}, T{1});
            const T sint2 = std::max<T>(T{0}, T{1} - cosi * cosi);
            const T k = T{1} - eta * eta * (T{1} - cosi * cosi);
            if (k < T{0}) return false;
            out = v * eta + n * (eta * cosi - static_cast<T>(std::sqrt(static_cast<long double>(k))));
            return true;
        }

        template <Arithmetic T>
        [[nodiscard]] inline T schlick(T cos_theta, T eta_i, T eta_t) noexcept
        {
            T r0 = (eta_i - eta_t) / (eta_i + eta_t);
            r0 = r0 * r0;
            return r0 + (T{1} - r0) * std::pow(static_cast<double>(T{1} - cos_theta), 5.0);
        }

        template <Arithmetic T>
        [[nodiscard]] inline Vector<T, 3> cosine_sample_hemisphere(T u1, T u2) noexcept
        {
            const T r = static_cast<T>(std::sqrt(static_cast<long double>(u1)));
            const T theta = static_cast<T>(T{2} * std::numbers::pi_v<T> * u2);
            const T x = r * static_cast<T>(std::cos(static_cast<long double>(theta)));
            const T y = r * static_cast<T>(std::sin(static_cast<long double>(theta)));
            const T z = static_cast<T>(std::sqrt(static_cast<long double>(std::max<T>(T{0}, T{1} - u1))));
            return {x, y, z};
        }

        template <Arithmetic T>
        [[nodiscard]] inline void build_ortho_basis(const Vector<T, 3>& n, Vector<T, 3>& t, Vector<T, 3>& b) noexcept
        {
            // n normalized expected
            if (std::abs(static_cast<double>(n[2])) < 0.999)
            {
                t = cross(Vector<T, 3>{T{0}, T{0}, T{1}}, n).normalized();
            }
            else
            {
                t = cross(Vector<T, 3>{T{1}, T{0}, T{0}}, n).normalized();
            }
            b = cross(n, t);
        }

        template <Arithmetic T>
        [[nodiscard]] inline Vector<T, 3> to_world(const Vector<T, 3>& local, const Vector<T, 3>& n) noexcept
        {
            Vector<T, 3> t, b;
            build_ortho_basis(n, t, b);
            return t * local[0] + b * local[1] + n * local[2];
        }

        // Simple hash for deterministic seeding from pixel/ray
        [[nodiscard]] inline std::uint64_t mix_seed(std::uint64_t a, std::uint64_t b) noexcept
        {
            a ^= b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2);
            return a;
        }

        template <Arithmetic T>
        [[nodiscard]] inline Vector<T, 3> hadamard(const Vector<T, 3>& a, const Vector<T, 3>& b) noexcept
        {
            return Vector<T, 3>{a[0] * b[0], a[1] * b[1], a[2] * b[2]};
        }
    }

    /**
     * @brief Path tracing renderer with configurable samples per pixel and depth.
     */
    export template <Arithmetic T>
    class RendererPathTracer : public Renderer<T>
    {
    public:
        using Color3 = Color<T, 3>;

        explicit RendererPathTracer(std::size_t samples_per_pixel = 1600, std::size_t max_depth = 4,
                                    std::uint64_t seed = 1337ULL)
            : spp_{samples_per_pixel}, max_depth_{max_depth}, seed_{seed}
        {
        }

        [[nodiscard]] Color3 trace_ray(const Scene<T>& scene, const Ray<T>& ray) const noexcept override
        {
            // Single-sample path trace for this ray with a deterministic seed from ray data
            std::uint64_t h = seed_;
            auto o = ray.origin();
            auto d = ray.direction();
            // hash origin and direction components
            for (int i = 0; i < 3; ++i)
            {
                h = detail_pt::mix_seed(h, std::bit_cast<std::uint64_t>(static_cast<double>(o[i])));
                h = detail_pt::mix_seed(h, std::bit_cast<std::uint64_t>(static_cast<double>(d[i])));
            }
            std::mt19937_64 rng{h};
            return path_trace_(scene, ray, rng);
        }

        void render(const Scene<T>& scene, Image<T, 3>& out, std::size_t width, std::size_t height) const override
        {
            if (out.width() != width || out.height() != height)
            {
                out.resize(width, height, Color<T, 3>{T{0}, T{0}, T{0}});
            }
            if (width == 0 || height == 0) return;
            const auto& cam = scene.camera();

            const std::size_t hw = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
            const std::size_t thread_count = std::min<std::size_t>(hw, height);
            std::vector<std::thread> threads;
            threads.reserve(thread_count);

            auto worker = [&](std::size_t y_begin, std::size_t y_end, std::uint64_t thread_seed) noexcept
            {
                std::mt19937_64 rng{thread_seed};
                std::uniform_real_distribution<double> uni(0.0, 1.0);
                for (std::size_t y = y_begin; y < y_end; ++y)
                {
                    for (std::size_t x = 0; x < width; ++x)
                    {
                        Color3 sum{T{0}, T{0}, T{0}};
                        for (std::size_t s = 0; s < spp_; ++s)
                        {
                            T jx = static_cast<T>(uni(rng));
                            T jy = static_cast<T>(uni(rng));
                            T fx = static_cast<T>(x) + jx;
                            T fy = static_cast<T>(y) + jy;
                            Ray<T> ray = cam.generate_ray(fx, fy, width, height);
                            sum += path_trace_(scene, ray, rng);
                        }
                        out(x, y) = sum / static_cast<T>(spp_);
                    }
                }
            };

            // Split rows into contiguous blocks per thread
            const std::size_t rows_per_thread = height / thread_count;
            const std::size_t remainder = height % thread_count;
            std::size_t y0 = 0;
            for (std::size_t t = 0; t < thread_count; ++t)
            {
                const std::size_t count = rows_per_thread + (t < remainder ? 1 : 0);
                const std::size_t y1 = y0 + count;
                const std::uint64_t tseed = detail_pt::mix_seed(seed_, static_cast<std::uint64_t>((y0 << 32) ^ y1 ^ t));
                threads.emplace_back(worker, y0, y1, tseed);
                y0 = y1;
            }
            for (auto& th : threads) th.join();
        }

    private:
        [[nodiscard]] Color3 path_trace_(const Scene<T>& scene, Ray<T> ray, std::mt19937_64& rng) const noexcept
        {
            using Vec3 = Vector<T, 3>;
            std::uniform_real_distribution<double> uni(0.0, 1.0);
            Color3 L{T{0}, T{0}, T{0}}; // accumulated radiance
            Color3 beta{T{1}, T{1}, T{1}}; // throughput

            for (std::size_t depth = 0; depth < max_depth_; ++depth)
            {
                // Intersect scene
                T best_t = ray.tmax();
                Vec3 best_n{};
                Vec3 best_p{};
                Vector<T, 2> best_uv{};
                const Material<T>* best_m = nullptr;
                bool any_hit = false;
                for (const auto& obj : scene.objects())
                {
                    if (auto box_hit = obj.aabb().intersect(ray); !box_hit.has_value()) continue;
                    if (auto h = obj.intersect(ray))
                    {
                        if (h->t >= ray.tmin() && h->t <= best_t)
                        {
                            any_hit = true;
                            best_t = h->t;
                            best_n = h->normal.normalized();
                            best_p = ray.at(h->t);
                            best_uv = h->uv;
                            best_m = &obj.material();
                        }
                    }
                }

                if (!any_hit)
                {
                    L += detail_pt::hadamard<T>(beta, scene.background());
                    break;
                }

                // Emission
                if (best_m)
                {
                    Color3 emission = best_m->radiance(best_uv) * best_m->emission(best_uv);
                    L += detail_pt::hadamard<T>(beta, emission);
                }

                // Prepare material terms
                const Color3 albedo = (best_m ? best_m->albedo(best_uv) : Color3::zeros());
                const T roughness = (best_m ? best_m->roughness(best_uv) : T{0});
                const T transparency = (best_m ? best_m->transparency(best_uv) : T{0});
                const T ior = (best_m ? best_m->refractive_index(best_uv) : T{1});

                // Russian roulette (after a few bounces)
                if (depth >= 3)
                {
                    T max_c = std::max({beta[0], beta[1], beta[2]});
                    max_c = std::clamp<T>(max_c, T{0}, T{1});
                    T q = static_cast<T>(1) - max_c;
                    if (static_cast<T>(uni(rng)) < q) break;
                    beta = beta / (static_cast<T>(1) - q);
                }

                // Compute next ray by sampling BSDF components
                const Vec3 n = best_n;
                Vec3 wo = -ray.direction(); // outgoing direction toward eye

                // Handle transmission if any transparency
                const bool entering = dot(ray.direction(), n) < T{0};
                Vec3 nl = entering ? n : -n; // oriented normal

                if (transparency > T{0})
                {
                    // Dielectric: sample Fresnel reflect vs refract. Always include Fresnel reflections.
                    // Compute indices and oriented normal
                    T eta_i = entering ? T{1} : ior;
                    T eta_t = entering ? ior : T{1};
                    T eta = eta_i / eta_t;

                    // Cosine of incident angle w.r.t. the oriented normal
                    T cos_theta_i = std::clamp<T>(-dot(ray.direction(), nl), T{0}, T{1});
                    // Schlick Fresnel reflectance
                    T R = detail_pt::schlick<T>(cos_theta_i, eta_i, eta_t);

                    // Try to compute refraction direction ahead of time to detect TIR
                    Vec3 refr_dir;
                    const bool can_refract = detail_pt::refract(ray.direction(), nl, eta, refr_dir);

                    // If TIR, force reflection
                    if (!can_refract)
                    {
                        Vec3 refl = detail_pt::reflect(ray.direction(), nl).normalized();
                        ray = Ray<T>{best_p + refl * static_cast<T>(1e-4), refl, ray.tmin(), ray.tmax()};
                        // Specular reflection branch. Throughput scaled by albedo; divide by prob 1 to keep unbiased.
                        beta = detail_pt::hadamard<T>(beta, albedo);
                        continue;
                    }

                    // Sample reflection vs refraction according to R
                    T xi = static_cast<T>(uni(rng));
                    if (xi < R)
                    {
                        // Reflect
                        Vec3 refl = detail_pt::reflect(ray.direction(), nl).normalized();
                        ray = Ray<T>{best_p + refl * static_cast<T>(1e-4), refl, ray.tmin(), ray.tmax()};
                        // Unbiased: divide by sampling prob R. For simple model, tint by albedo.
                        beta = detail_pt::hadamard<T>(beta, albedo) / std::max<T>(R, static_cast<T>(1e-3));
                        continue;
                    }
                    else
                    {
                        // Refract
                        refr_dir = refr_dir.normalized();
                        ray = Ray<T>{best_p + refr_dir * static_cast<T>(1e-4), refr_dir, ray.tmin(), ray.tmax()};
                        // Unbiased: divide by sampling prob (1-R). Apply transparency as transmission factor.
                        T prob = std::max<T>(T{1} - R, static_cast<T>(1e-3));
                        beta = detail_pt::hadamard<T>(beta, albedo) * transparency / prob;
                        continue;
                    }
                }

                // Opaque surface: mix specular and diffuse by roughness
                T prob_spec = std::clamp<T>(T{1} - roughness, T{0}, T{1});
                if (static_cast<T>(uni(rng)) < prob_spec)
                {
                    // Specular (perfect) reflection
                    Vec3 refl = detail_pt::reflect(-wo, n).normalized();
                    ray = Ray<T>{best_p + refl * static_cast<T>(1e-4), refl, ray.tmin(), ray.tmax()};
                    beta = detail_pt::hadamard<T>(beta, albedo) / std::max(prob_spec, static_cast<T>(1e-3));
                }
                else
                {
                    // Diffuse Lambert: cosine-weighted hemisphere sampling
                    Vector<T, 3> local = detail_pt::cosine_sample_hemisphere<T>(
                        static_cast<T>(uni(rng)), static_cast<T>(uni(rng)));
                    Vec3 dir = detail_pt::to_world(local, n).normalized();
                    ray = Ray<T>{best_p + dir * static_cast<T>(1e-4), dir, ray.tmin(), ray.tmax()};
                    // BRDF = albedo/pi, pdf = cos(theta)/pi => weight = albedo
                    beta = detail_pt::hadamard<T>(beta, albedo) / std::max<T>(T{1} - prob_spec, static_cast<T>(1e-3));
                }
            }

            return L;
        }

        std::size_t spp_;
        std::size_t max_depth_;
        std::uint64_t seed_;
    };
}
