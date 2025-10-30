module;
#include <cstddef>
#include <memory>
#include <ostream>
#include <type_traits>

export module glimmer.material;

import glimmer.vector;
import glimmer.color;
import glimmer.material_property;
import glimmer.material_property.uniform;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing a simple physically-inspired material descriptor.
     */

    /**
     * @brief Material parameters for basic rendering use cases using UV-dependent properties.
     * @tparam T arithmetic scalar type (float/double recommended)
     * @details Each parameter is represented by a `MaterialProperty` which can be a texture,
     * procedural function, or a uniform constant.
     */
    export template <Arithmetic T>
    class Material {
    public:
        using value_type = T;
        using Color3 = Color<T,3>;
        using Vector2 = Vector<T, 2>;
        using Normal3 = Vector<T, 3>;

        /** @brief Constructs a default black opaque material with zero roughness and zero emission power. */
        Material() noexcept
            : albedo_{std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}})}
              , roughness_{std::make_shared<UniformMaterialProperty<T, T>>(T{0})}
              , radiance_{std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}})}
              , transparency_{std::make_shared<UniformMaterialProperty<T, T>>(T{0})}
              , emission_{std::make_shared<UniformMaterialProperty<T, T>>(T{0})}
              , refractive_index_{std::make_shared<UniformMaterialProperty<T, T>>(T{1})}
              , normal_relative_{std::make_shared<UniformMaterialProperty<T, Normal3>>(Normal3{T{0}, T{0}, T{1}})}
        {
        }

        /** @brief Returns diffuse/specular base color (RGB) at given UV. */
        [[nodiscard]] Color3 albedo(const Vector2& uv) const noexcept { return albedo_->get(uv); }
        /** @brief Returns surface roughness in [0,1] at given UV. */
        [[nodiscard]] T roughness(const Vector2& uv) const noexcept { return clamp01(roughness_->get(uv)); }
        /** @brief Returns emissive radiance (RGB) sampled at given UV. */
        [[nodiscard]] Color3 radiance(const Vector2& uv) const noexcept { return radiance_->get(uv); }
        /** @brief Returns transparency (0=opaque, 1=fully transparent) at given UV. */
        [[nodiscard]] T transparency(const Vector2& uv) const noexcept { return clamp01(transparency_->get(uv)); }
        /** @brief Returns emission power/intensity multiplier (>=0) at given UV. */
        [[nodiscard]] T emission(const Vector2& uv) const noexcept { return clamp_nonneg(emission_->get(uv)); }
        /** @brief Returns refractive index (>=1) at given UV. */
        [[nodiscard]] T refractive_index(const Vector2& uv) const noexcept
        {
            return clamp_min_one(refractive_index_->get(uv));
        }

        /** @brief Returns normal perturbation relative to the geometric normal in tangent space (+Z is geometric normal). */
        [[nodiscard]] Normal3 normal_relative(const Vector2& uv) const noexcept { return normal_relative_->get(uv); }

        // Backward-compatibility helpers: sample at UV=(0,0)
        [[nodiscard]] Color3 albedo() const noexcept { return albedo(Vector2{T{0}, T{0}}); }
        [[nodiscard]] T roughness() const noexcept { return roughness(Vector2{T{0}, T{0}}); }
        [[nodiscard]] Color3 radiance() const noexcept { return radiance(Vector2{T{0}, T{0}}); }
        [[nodiscard]] T transparency() const noexcept { return transparency(Vector2{T{0}, T{0}}); }
        [[nodiscard]] T emission() const noexcept { return emission(Vector2{T{0}, T{0}}); }
        [[nodiscard]] T refractive_index() const noexcept { return refractive_index(Vector2{T{0}, T{0}}); }
        [[nodiscard]] Normal3 normal_relative() const noexcept { return normal_relative(Vector2{T{0}, T{0}}); }

        /** Factory helpers producing materials with uniform (UV-independent) properties. */
        [[nodiscard]] static Material lambertian(const Color3& albedo) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}});
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        [[nodiscard]] static Material metal(const Color3& albedo, T roughness) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(roughness));
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}});
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        [[nodiscard]] static Material emissive(const Color3& radiance) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}});
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        [[nodiscard]] static Material emissive(const Color3& radiance, T power) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(Color3{T{0}, T{0}, T{0}});
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp_nonneg(power));
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        [[nodiscard]] static Material glass(const Color3& albedo, T roughness, T transparency,
                                            const Color3& radiance = Color3{T{0}, T{0}, T{0}}) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(roughness));
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(transparency));
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1.5});
            return m;
        }

        [[nodiscard]] static Material glass(const Color3& albedo, T roughness, T transparency, const Color3& radiance,
                                            T power) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(roughness));
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(transparency));
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp_nonneg(power));
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1.5});
            return m;
        }

        [[nodiscard]] static Material from_params(const Color3& albedo, T roughness, T transparency,
                                                  const Color3& radiance) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(roughness));
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(transparency));
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(T{0});
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        [[nodiscard]] static Material from_params(const Color3& albedo, T roughness, T transparency,
                                                  const Color3& radiance, T power) noexcept
        {
            Material m;
            m.albedo_ = std::make_shared<UniformMaterialProperty<T, Color3>>(albedo);
            m.roughness_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(roughness));
            m.transparency_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp01(transparency));
            m.radiance_ = std::make_shared<UniformMaterialProperty<T, Color3>>(radiance);
            m.emission_ = std::make_shared<UniformMaterialProperty<T, T>>(clamp_nonneg(power));
            m.refractive_index_ = std::make_shared<UniformMaterialProperty<T, T>>(T{1});
            return m;
        }

        /** @brief Equality comparison by sampling all properties at UV=(0,0). */
        [[nodiscard]] bool operator==(const Material& rhs) const noexcept
        {
            const Vector2 uv{T{0}, T{0}};
            return albedo(uv) == rhs.albedo(uv)
                && roughness(uv) == rhs.roughness(uv)
                && radiance(uv) == rhs.radiance(uv)
                && transparency(uv) == rhs.transparency(uv)
                && emission(uv) == rhs.emission(uv)
                && refractive_index(uv) == rhs.refractive_index(uv)
                && normal_relative(uv) == rhs.normal_relative(uv);
        }

        /** @brief Logical negation of operator==. */
        [[nodiscard]] bool operator!=(const Material& rhs) const noexcept { return !(*this == rhs); }

        // Access to underlying property objects (advanced use)
        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, Color3>>& albedo_property() const noexcept
        {
            return albedo_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, T>>& roughness_property() const noexcept
        {
            return roughness_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, Color3>>& radiance_property() const noexcept
        {
            return radiance_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, T>>& transparency_property() const noexcept
        {
            return transparency_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, T>>& emission_property() const noexcept
        {
            return emission_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, T>>& refractive_index_property() const noexcept
        {
            return refractive_index_;
        }

        [[nodiscard]] const std::shared_ptr<MaterialProperty<T, Normal3>>& normal_relative_property() const noexcept
        {
            return normal_relative_;
        }

        // Setters for swapping property implementations
        void set_albedo_property(std::shared_ptr<MaterialProperty<T, Color3>> p) noexcept { albedo_ = std::move(p); }
        void set_roughness_property(std::shared_ptr<MaterialProperty<T, T>> p) noexcept { roughness_ = std::move(p); }

        void set_radiance_property(std::shared_ptr<MaterialProperty<T, Color3>> p) noexcept
        {
            radiance_ = std::move(p);
        }

        void set_transparency_property(std::shared_ptr<MaterialProperty<T, T>> p) noexcept
        {
            transparency_ = std::move(p);
        }

        void set_emission_property(std::shared_ptr<MaterialProperty<T, T>> p) noexcept { emission_ = std::move(p); }
        void set_refractive_index_property(std::shared_ptr<MaterialProperty<T, T>> p) noexcept
        {
            refractive_index_ = std::move(p);
        }

        void set_normal_relative_property(std::shared_ptr<MaterialProperty<T, Normal3>> p) noexcept
        {
            normal_relative_ = std::move(p);
        }

    private:
        static constexpr T clamp01(T v) noexcept
        {
            if (v < T{0}) return T{0};
            if (v > T{1}) return T{1};
            return v;
        }

        static constexpr T clamp_nonneg(T v) noexcept { return v < T{0} ? T{0} : v; }
        static constexpr T clamp_min_one(T v) noexcept { return v < T{1} ? T{1} : v; }

        std::shared_ptr<MaterialProperty<T, Color3>> albedo_{};
        std::shared_ptr<MaterialProperty<T, T>> roughness_{};
        std::shared_ptr<MaterialProperty<T, Color3>> radiance_{};
        std::shared_ptr<MaterialProperty<T, T>> transparency_{};
        std::shared_ptr<MaterialProperty<T, T>> emission_{}; // emission power/intensity multiplier
        std::shared_ptr<MaterialProperty<T, T>> refractive_index_{}; // new property
        std::shared_ptr<MaterialProperty<T, Normal3>> normal_relative_{};
        // tangent-space normal relative to geometric normal (+Z)
    };

    /**
     * @brief Streams a material in a readable form by sampling at UV=(0,0).
     * @tparam T arithmetic scalar type
     */
    export template <Arithmetic T>
    inline std::ostream& operator<<(std::ostream& os, const Material<T>& m) {
        typename Material<T>::Vector2 uv{T{0}, T{0}};
        os << "Material{albedo=" << m.albedo(uv) << ", roughness=" << m.roughness(uv)
           << ", transparency=" << m.transparency(uv) << ", radiance=" << m.radiance(uv)
           << ", emission=" << m.emission(uv) << ", ior=" << m.refractive_index(uv)
           << ", n_rel=" << m.normal_relative(uv) << '}';
        return os;
    }
}
