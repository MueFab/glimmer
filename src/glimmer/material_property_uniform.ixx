export module glimmer.material_property.uniform;

import glimmer.material_property;
import glimmer.vector;

namespace glimmer
{
    /**
     * @brief Concrete property that returns a constant value (ignores UV).
     */
    export template <Arithmetic T, typename Out>
    class UniformMaterialProperty final : public MaterialProperty<T, Out>
    {
    public:
        using output_type = Out;

        constexpr explicit UniformMaterialProperty(const Out& v) noexcept : value_{v}
        {
        }

        [[nodiscard]] Out get(const Vector<T, 2>&) const noexcept override { return value_; }

    private:
        Out value_{};
    };
}