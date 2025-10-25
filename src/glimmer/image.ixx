module;
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <algorithm>

export module glimmer.image;

import glimmer.vector;
import glimmer.color;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing a simple Image class for 2D pixel buffers.
     */

    /**
     * @brief 2D image with N-channel pixels of arithmetic type T.
     * @tparam T arithmetic scalar per-channel type
     * @tparam N number of channels (e.g., 3 for RGB, 4 for RGBA)
     * @details Stores pixels in a contiguous row-major buffer of size width*height.
     * Pixel type is Color<T,N> which aliases Vector<T,N>.
     */
    export template <Arithmetic T, std::size_t N>
    class Image {
    public:
        using value_type = T;
        using pixel_type = Color<T, N>;
        using size_type = std::size_t;

        /** @brief Constructs an empty image with zero dimensions. */
        constexpr Image() noexcept = default;

        /**
         * @brief Constructs an image with given dimensions; pixels value-initialized.
         * @param w width in pixels
         * @param h height in pixels
         */
        explicit Image(size_type w, size_type h)
            : w_{w}, h_{h}, data_(w * h) {}

        /**
         * @brief Constructs an image with given dimensions and fill value.
         * @param w width in pixels
         * @param h height in pixels
         * @param fill pixel value used to initialize all pixels
         */
        Image(size_type w, size_type h, const pixel_type& fill)
            : w_{w}, h_{h}, data_(w * h, fill) {}

        /** @brief Image width. */
        [[nodiscard]] constexpr size_type width() const noexcept { return w_; }
        /** @brief Image height. */
        [[nodiscard]] constexpr size_type height() const noexcept { return h_; }
        /** @brief Total pixel count. */
        [[nodiscard]] constexpr size_type size() const noexcept { return data_.size(); }
        /** @brief Returns true if image is empty (w==0 || h==0). */
        [[nodiscard]] constexpr bool empty() const noexcept { return data_.empty(); }

        /** @brief Pointer to pixel data (contiguous). */
        [[nodiscard]] pixel_type* data() noexcept { return data_.data(); }
        /** @brief Const pointer to pixel data (contiguous). */
        [[nodiscard]] const pixel_type* data() const noexcept { return data_.data(); }

        /**
         * @brief Unchecked pixel access (x,y).
         * @param x column in [0, width)
         * @param y row in [0, height)
         * @return reference to pixel
         */
        [[nodiscard]] pixel_type& operator()(size_type x, size_type y) noexcept { return data_[index(x,y)]; }
        /** @brief Unchecked const pixel access (x,y). */
        [[nodiscard]] const pixel_type& operator()(size_type x, size_type y) const noexcept { return data_[index(x,y)]; }

        /**
         * @brief Bounds-checked pixel access.
         * @throws std::out_of_range for out-of-bounds coordinates
         */
        [[nodiscard]] pixel_type& at(size_type x, size_type y) {
            if (x >= w_ || y >= h_) throw std::out_of_range("Image::at out of range");
            return data_[y * w_ + x];
        }
        /** @brief Bounds-checked const pixel access. */
        [[nodiscard]] const pixel_type& at(size_type x, size_type y) const {
            if (x >= w_ || y >= h_) throw std::out_of_range("Image::at out of range");
            return data_[y * w_ + x];
        }

        /** @brief Fills all pixels with the given color. */
        void clear(const pixel_type& value) {
            std::fill(data_.begin(), data_.end(), value);
        }

        /**
         * @brief Resizes the image and re-initializes pixels with fill value.
         * @param w new width
         * @param h new height
         * @param fill value assigned to all pixels after resize
         * @note Existing contents are not preserved to keep the API simple.
         */
        void resize(size_type w, size_type h, const pixel_type& fill = pixel_type{}) {
            w_ = w; h_ = h; data_.assign(w * h, fill);
        }

        /** @brief Returns true if (x,y) lies within image bounds. */
        [[nodiscard]] constexpr bool in_bounds(size_type x, size_type y) const noexcept {
            return x < w_ && y < h_;
        }

    private:
        [[nodiscard]] constexpr size_type index(size_type x, size_type y) const noexcept { return y * w_ + x; }

        size_type w_{};
        size_type h_{};
        std::vector<pixel_type> data_{};
    };
}
