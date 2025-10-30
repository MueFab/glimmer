module;
#include <cmath>

export module glimmer.material_property.image;

import glimmer.material_property;
import glimmer.image;
import glimmer.vector;
import glimmer.color;

namespace glimmer
{
    /**
     * @brief Addressing (UV wrap) modes for sampling images.
     */
    export enum class AddressMode
    {
        Clamp,
        Repeat
    };

    /**
     * @brief Material property sourced from an Image<T,N>, sampled by UV.
     * @tparam T arithmetic scalar (image channel type and UV scalar)
     * @tparam N number of channels in the image and returned color
     * @details UV coordinates are expected in [0,1]. When outside, handling depends on AddressMode.
     * Sampling uses nearest-neighbor selection.
     */
    export template <Arithmetic T, std::size_t N>
    class ImageMaterialProperty final : public MaterialProperty<T, Color<T, N>>
    {
    public:
        using Out = Color<T, N>;
        using ImageType = Image<T, N>;
        using Vector2 = Vector<T, 2>;

        /**
         * @brief Construct from an image reference and address mode.
         * @param img const reference to image to sample. Caller must ensure lifetime covers uses of this property.
         * @param mode addressing mode for UV outside [0,1]
         */
        explicit ImageMaterialProperty(const ImageType& img, AddressMode mode = AddressMode::Clamp) noexcept
            : img_{&img}, mode_{mode}
        {
        }

        [[nodiscard]] Out get(const Vector2& uv) const noexcept override
        {
            if (!img_ || img_->empty())
            {
                return Out{}; // zero color if no image
            }
            auto u = uv[0];
            auto v = uv[1];
            // Apply address mode
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
            // Convert to texel coordinates (nearest neighbor)
            const auto w = static_cast<T>(img_->width());
            const auto h = static_cast<T>(img_->height());
            // Guard against degenerate dims
            if (w <= T{0} || h <= T{0}) return Out{};
            // Map [0,1] to [0, w-1] and [0,h-1]
            const T fx = u * (w - T{1});
            const T fy = v * (h - T{1});
            // Round to nearest
            const auto xi = static_cast<std::size_t>(std::llround(static_cast<double>(fx)));
            const auto yi = static_cast<std::size_t>(std::llround(static_cast<double>(fy)));
            // Clamp just in case of numerical blips
            const auto x = (xi >= img_->width()) ? (img_->width() - 1) : xi;
            const auto y = (yi >= img_->height()) ? (img_->height() - 1) : yi;
            return (*img_)(x, y);
        }

        /** @brief Returns the underlying image pointer (may be null). */
        [[nodiscard]] const ImageType* image() const noexcept { return img_; }
        /** @brief Addressing mode accessor. */
        [[nodiscard]] AddressMode address_mode() const noexcept { return mode_; }

    private:
        const ImageType* img_{};
        AddressMode mode_{AddressMode::Clamp};
    };
}
