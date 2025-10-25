module;
#include <cmath>
#include <type_traits>
#include <ostream>

export module glimmer.color;

import glimmer.vector;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing color utilities and aliases.
     */

    /**
     * @brief Generic color alias backed by Vector.
     * @tparam T arithmetic scalar type
     * @tparam N number of channels
     */
    export template <Arithmetic T, std::size_t N>
    using Color = Vector<T, N>;

    /** @brief 3-channel float color (linear RGB). */
    export using Color3f = Color<float, 3>;
    /** @brief 4-channel float color with alpha (linear premultiplication utilities provided). */
    export using Color4f = Color<float, 4>;

    /**
     * @brief Clamps each component of color c to [lo, hi].
     * @tparam T arithmetic scalar type
     * @tparam N channels
     * @param c input color
     * @param lo lower bound
     * @param hi upper bound
     * @return component-wise clamped color
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Color<T,N> clamp(Color<T,N> c, const T& lo, const T& hi) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            if (c[i] < lo) c[i] = lo; else if (c[i] > hi) c[i] = hi;
        }
        return c;
    }

    /**
     * @brief Clamps each component of color c to [0,1].
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Color<T,N> saturate(Color<T,N> c) noexcept {
        return clamp(c, static_cast<T>(0), static_cast<T>(1));
    }

    /**
     * @brief Converts a linear RGB triplet to sRGB (gamma-encoded) in [0,1].
     * @tparam T arithmetic scalar type
     * @param lin linear RGB color (values outside [0,1] are clamped prior to conversion)
     * @return sRGB-encoded color in [0,1]
     */
    export template <Arithmetic T>
    [[nodiscard]] inline Color<T,3> linear_to_srgb(Color<T,3> lin) {
        using std::pow;
        lin = saturate(lin);
        Color<T,3> out{};
        for (std::size_t i=0;i<3;++i) {
            T c = lin[i];
            if (c <= static_cast<T>(0.0031308)) out[i] = static_cast<T>(12.92) * c;
            else out[i] = static_cast<T>(1.055) * static_cast<T>(pow(static_cast<long double>(c), 1.0L/2.4L)) - static_cast<T>(0.055);
        }
        return out;
    }

    /**
     * @brief Converts an sRGB triplet (gamma-encoded) to linear RGB.
     * @tparam T arithmetic scalar type
     * @param srgb sRGB color (values outside [0,1] are clamped prior to conversion)
     * @return linear RGB color
     */
    export template <Arithmetic T>
    [[nodiscard]] inline Color<T,3> srgb_to_linear(Color<T,3> srgb) {
        using std::pow;
        srgb = saturate(srgb);
        Color<T,3> out{};
        for (std::size_t i=0;i<3;++i) {
            T c = srgb[i];
            if (c <= static_cast<T>(0.04045)) out[i] = c / static_cast<T>(12.92);
            else out[i] = static_cast<T>(pow(static_cast<long double>((c + static_cast<T>(0.055)) / static_cast<T>(1.055)), 2.4L));
        }
        return out;
    }

    /**
     * @brief Relative luminance for linear RGB using Rec.709 coefficients.
     * @tparam T arithmetic scalar type
     * @param rgb linear RGB color
     * @return Y = 0.2126 R + 0.7152 G + 0.0722 B
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr T luminance(const Color<T,3>& rgb) noexcept {
        return static_cast<T>(0.2126) * rgb[0] + static_cast<T>(0.7152) * rgb[1] + static_cast<T>(0.0722) * rgb[2];
    }

    /**
     * @brief Combines linear RGB and alpha into a 4-channel color.
     * @tparam T arithmetic scalar type
     * @param rgb color in linear space
     * @param a alpha in [0,1]
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Color<T,4> with_alpha(const Color<T,3>& rgb, T a) noexcept {
        return Color<T,4>{rgb[0], rgb[1], rgb[2], a};
    }

    /**
     * @brief Premultiplies RGB by alpha (assumes linear space).
     * @tparam T arithmetic scalar type
     * @param c rgba color
     * @return premultiplied rgba (rgb' = a*rgb, a' = a)
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Color<T,4> premultiply(Color<T,4> c) noexcept {
        c[0] *= c[3];
        c[1] *= c[3];
        c[2] *= c[3];
        return c;
    }

    /**
     * @brief Unpremultiplies an RGBA color (division by alpha, safe when a==0 leaves rgb at 0).
     * @tparam T arithmetic scalar type
     */
    export template <Arithmetic T>
    [[nodiscard]] inline Color<T,4> unpremultiply(Color<T,4> c) {
        if (c[3] != static_cast<T>(0)) {
            if constexpr (std::is_floating_point_v<T>) {
                c[0] /= c[3]; c[1] /= c[3]; c[2] /= c[3];
            } else {
                c[0] = static_cast<T>(c[0] / c[3]);
                c[1] = static_cast<T>(c[1] / c[3]);
                c[2] = static_cast<T>(c[2] / c[3]);
            }
        } else {
            c[0] = c[1] = c[2] = static_cast<T>(0);
        }
        return c;
    }

    /**
     * @brief Porter–Duff "over" compositing for premultiplied RGBA colors.
     * @tparam T arithmetic scalar type
     * @param src_pm source color (premultiplied)
     * @param dst_pm destination color (premultiplied)
     * @return out = src + dst*(1 - src.a)
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Color<T,4> over(const Color<T,4>& src_pm, const Color<T,4>& dst_pm) noexcept {
        Color<T,4> out{};
        const T one = static_cast<T>(1);
        const T ka = one - src_pm[3];
        out[0] = src_pm[0] + dst_pm[0] * ka;
        out[1] = src_pm[1] + dst_pm[1] * ka;
        out[2] = src_pm[2] + dst_pm[2] * ka;
        out[3] = src_pm[3] + dst_pm[3] * ka;
        return out;
    }

}
