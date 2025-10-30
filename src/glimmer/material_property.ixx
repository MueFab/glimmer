export module glimmer.material_property;

import glimmer.vector;

namespace glimmer
{
    /**
     * @brief Abstract interface for a material property sampled by UV coordinates.
     * @tparam T arithmetic scalar type for UV coordinates
     * @tparam Out return type of the property (e.g., Color<T,3> or T)
     */
    export template <Arithmetic T, typename Out>
    class MaterialProperty
    {
    public:
        using value_type = T;
        using output_type = Out;
        virtual ~MaterialProperty() = default;
        [[nodiscard]] virtual Out get(const Vector<T, 2>& uv) const noexcept = 0;
    };
}