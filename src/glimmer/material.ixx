module;
#include <cstddef>
#include <ostream>
#include <type_traits>

export module glimmer.material;

import glimmer.vector;
import glimmer.color;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing a simple physically-inspired material descriptor.
     */

    /**
     * @brief Material parameters for basic rendering use cases (no type tags).
     * @tparam T arithmetic scalar type (float/double recommended)
     * @details Lightweight POD-like container holding a set of common parameters:
     * - albedo: base color (RGB)
     * - roughness: microfacet roughness clamped to [0,1]
     * - transparency: transmission factor clamped to [0,1]
     * - radiance: emissive color (RGB)
     *
     * Factory helpers are provided to conveniently construct common presets
     * (lambertian, metal, emissive, glass) but no tag is stored; materials are
     * described purely by their parameters.
     */
    export template <Arithmetic T>
    class Material {
    public:
        using value_type = T;
        using Color3 = Color<T,3>;

        /** @brief Constructs a default black opaque material with zero roughness and zero emission power. */
        constexpr Material() noexcept : albedo_{T{0},T{0},T{0}}, roughness_{T{0}}, radiance_{T{0},T{0},T{0}}, transparency_{T{0}}, emission_{T{0}} {}

        /** @brief Returns diffuse/specular base color (RGB). */
        [[nodiscard]] constexpr const Color3& albedo() const noexcept { return albedo_; }
        /** @brief Returns surface roughness in [0,1]. */
        [[nodiscard]] constexpr T roughness() const noexcept { return roughness_; }
        /** @brief Returns emissive radiance (RGB) which is scaled by emission() to get emitted power per unit area/steradian. */
        [[nodiscard]] constexpr const Color3& radiance() const noexcept { return radiance_; }
        /** @brief Returns transparency (0=opaque, 1=fully transparent). */
        [[nodiscard]] constexpr T transparency() const noexcept { return transparency_; }
        /** @brief Returns emission power/intensity multiplier (>=0). */
        [[nodiscard]] constexpr T emission() const noexcept { return emission_; }

        /**
         * @brief Factory: Lambertian material with diffuse albedo.
         * @param albedo diffuse reflectance RGB (linear space)
         */
        [[nodiscard]] static constexpr Material lambertian(const Color3& albedo) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = T{0}; m.radiance_ = {T{0},T{0},T{0}}; m.transparency_ = T{0}; return m;
        }
        /**
         * @brief Factory: Metallic material with albedo and roughness.
         * @param albedo base color RGB
         * @param roughness microfacet roughness; clamped to [0,1]
         */
        [[nodiscard]] static constexpr Material metal(const Color3& albedo, T roughness) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = clamp01(roughness); m.radiance_ = {T{0},T{0},T{0}}; m.transparency_ = T{0}; return m;
        }
        /**
         * @brief Factory: Emissive light-emitting material.
         * @param radiance emitted radiance RGB
         */
        [[nodiscard]] static constexpr Material emissive(const Color3& radiance) noexcept {
            // Backward-compatible: default emission power = 1
            Material m; m.albedo_ = {T{0},T{0},T{0}}; m.roughness_ = T{0}; m.radiance_ = radiance; m.transparency_ = T{0}; m.emission_ = T{1}; return m;
        }
        /**
         * @brief Factory: Emissive light-emitting material with explicit power.
         * @param radiance emitted radiance RGB (color)
         * @param power emission power/intensity multiplier (>=0)
         */
        [[nodiscard]] static constexpr Material emissive(const Color3& radiance, T power) noexcept {
            Material m; m.albedo_ = {T{0},T{0},T{0}}; m.roughness_ = T{0}; m.radiance_ = radiance; m.transparency_ = T{0}; m.emission_ = clamp_nonneg(power); return m;
        }
        /**
         * @brief Factory: Glass-like material supporting transparency.
         * @param albedo base color tint
         * @param roughness surface roughness in [0,1]
         * @param transparency transmission factor in [0,1]
         * @param radiance optional emissive radiance (default black)
         */
        [[nodiscard]] static constexpr Material glass(const Color3& albedo, T roughness, T transparency, const Color3& radiance = Color3{T{0},T{0},T{0}}) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = clamp01(roughness); m.transparency_ = clamp01(transparency); m.radiance_ = radiance; m.emission_ = T{0}; return m;
        }
        /**
         * @brief Factory: Glass-like material with explicit emission power.
         */
        [[nodiscard]] static constexpr Material glass(const Color3& albedo, T roughness, T transparency, const Color3& radiance, T power) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = clamp01(roughness); m.transparency_ = clamp01(transparency); m.radiance_ = radiance; m.emission_ = clamp_nonneg(power); return m;
        }
        /**
         * @brief Generic parameterized material allowing any combination.
         * @param albedo diffuse/specular base color
         * @param roughness microfacet roughness in [0,1]
         * @param transparency transmission factor in [0,1]
         * @param radiance emissive radiance
         */
        [[nodiscard]] static constexpr Material from_params(const Color3& albedo, T roughness, T transparency, const Color3& radiance) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = clamp01(roughness); m.transparency_ = clamp01(transparency); m.radiance_ = radiance; m.emission_ = T{0}; return m;
        }
        /** @brief Generic parameterized material with explicit emission power. */
        [[nodiscard]] static constexpr Material from_params(const Color3& albedo, T roughness, T transparency, const Color3& radiance, T power) noexcept {
            Material m; m.albedo_ = albedo; m.roughness_ = clamp01(roughness); m.transparency_ = clamp01(transparency); m.radiance_ = radiance; m.emission_ = clamp_nonneg(power); return m;
        }

        /** @brief Exact equality comparison of all stored fields. */
        [[nodiscard]] constexpr bool operator==(const Material& rhs) const noexcept {
            return albedo_ == rhs.albedo_ && roughness_ == rhs.roughness_ && radiance_ == rhs.radiance_ && transparency_ == rhs.transparency_ && emission_ == rhs.emission_;
        }
        /** @brief Logical negation of operator==. */
        [[nodiscard]] constexpr bool operator!=(const Material& rhs) const noexcept { return !(*this == rhs); }

    private:
        static constexpr T clamp01(T v) noexcept {
            if (v < T{0}) return T{0};
            if (v > T{1}) return T{1};
            return v;
        }
        static constexpr T clamp_nonneg(T v) noexcept {
            return v < T{0} ? T{0} : v;
        }

        Color3 albedo_{};
        T roughness_{};
        Color3 radiance_{};
        T transparency_{};
        T emission_{}; // emission power/intensity multiplier
    };

    /**
     * @brief Streams a material in a readable form.
     * @tparam T arithmetic scalar type
     */
    export template <Arithmetic T>
    inline std::ostream& operator<<(std::ostream& os, const Material<T>& m) {
        os << "Material{albedo=" << m.albedo() << ", roughness=" << m.roughness()
           << ", transparency=" << m.transparency() << ", radiance=" << m.radiance()
           << ", emission=" << m.emission() << '}';
        return os;
    }
}
