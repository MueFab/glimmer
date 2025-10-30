module;
#include <cmath>

export module glimmer.material_property.checkerboard;

import glimmer.material_property;
import glimmer.vector;
import glimmer.color;
import glimmer.material_property.image; // for AddressMode

namespace glimmer
{
    /**
     * @brief UV-driven checkerboard material property between two colors.
     * @tparam T arithmetic scalar type
     * @tparam N number of channels for the output color
     *
     * Semantics:
     *  - UVs are expected in [0,1], but handled per AddressMode when out of range.
     *  - The checker index is computed as:
     *      i = floor((u + offset.x) * tiles_u)
     *      j = floor((v + offset.y) * tiles_v)
     *      parity = (i + j) & 1
     *  - Returns color_a for even parity, color_b for odd parity.
     */
    export template <Arithmetic T, std::size_t N>
    class CheckerboardMaterialProperty final : public MaterialProperty<T, Color<T, N>>
    {
    public:
        using Out = Color<T, N>;
        using Vector2 = Vector<T, 2>;

        constexpr CheckerboardMaterialProperty(
            const Out& color_a,
            const Out& color_b,
            T tiles_u = T{8},
            T tiles_v = T{8},
            Vector2 offset = Vector2{T{0}, T{0}},
            AddressMode mode = AddressMode::Repeat
        ) noexcept
            : a_{color_a}, b_{color_b}, tiles_u_{tiles_u}, tiles_v_{tiles_v}, offset_{offset}, mode_{mode}
        {
        }

        [[nodiscard]] Out get(const Vector2& uv) const noexcept override
        {
            // Prepare UVs
            T u = uv[0];
            T v = uv[1];

            if (mode_ == AddressMode::Repeat)
            {
                u = u - static_cast<T>(static_cast<long long>(std::floor(static_cast<double>(u))));
                v = v - static_cast<T>(static_cast<long long>(std::floor(static_cast<double>(v))));
            }
            else
            {
                // Clamp
                if (u < T{0}) u = T{0};
                if (u > T{1}) u = T{1};
                if (v < T{0}) v = T{0};
                if (v > T{1}) v = T{1};
            }

            // Apply offset
            u += offset_[0];
            v += offset_[1];

            // Effective tile counts
            const T tu = (tiles_u_ > T{0}) ? tiles_u_ : T{1};
            const T tv = (tiles_v_ > T{0}) ? tiles_v_ : T{1};

            // Compute cell indices (floor)
            const long long i = static_cast<long long>(std::floor(static_cast<double>(u * tu)));
            const long long j = static_cast<long long>(std::floor(static_cast<double>(v * tv)));

            const bool odd = ((i + j) & 1LL) != 0LL;
            return odd ? b_ : a_;
        }

        // Accessors
        [[nodiscard]] constexpr const Out& color_a() const noexcept { return a_; }
        [[nodiscard]] constexpr const Out& color_b() const noexcept { return b_; }
        [[nodiscard]] constexpr T tiles_u() const noexcept { return tiles_u_; }
        [[nodiscard]] constexpr T tiles_v() const noexcept { return tiles_v_; }
        [[nodiscard]] constexpr Vector2 offset() const noexcept { return offset_; }
        [[nodiscard]] constexpr AddressMode address_mode() const noexcept { return mode_; }

    private:
        Out a_{};
        Out b_{};
        T tiles_u_{T{8}};
        T tiles_v_{T{8}};
        Vector2 offset_{T{0}, T{0}};
        AddressMode mode_{AddressMode::Repeat};
    };
}
