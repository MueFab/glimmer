module;
#include <string>
#include <fstream>
#include <optional>
#include <vector>
#include <cctype>
#include <type_traits>
#include <cstdlib>
#include <cmath>

export module glimmer.ppm;

import glimmer.vector;
import glimmer.image;
import glimmer.color;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module providing PPM (P6) image IO helpers for 3-channel images.
     */

    /**
     * @brief Saves a 3-channel image to a binary PPM (P6) file.
     * @tparam T arithmetic scalar per-channel type
     * @param img image with 3 channels (RGB)
     * @param path filesystem path to write to
     * @return true on success, false on failure
     * @details For floating-point T, channels are clamped to [0,1] and scaled to 8-bit. For integral T, channels
     * are clamped to [0,255] and truncated to 8-bit. This function delegates to Image::write_ppm.
     */
    export template <Arithmetic T>
    [[nodiscard]] inline bool save_ppm(const Image<T,3>& img, const std::string& path) {
        std::ofstream f(path, std::ios::binary);
        if (!f.good()) return false;
        f << "P6\n" << img.width() << ' ' << img.height() << "\n255\n";
        using size_type = typename Image<T,3>::size_type;
        std::vector<unsigned char> row(3 * img.width());
        for (size_type y = 0; y < img.height(); ++y) {
            for (size_type x = 0; x < img.width(); ++x) {
                const auto& c = img(x, y);
                auto to_u8 = [](T v) -> unsigned char {
                    if constexpr (std::is_floating_point_v<T>) {
                        T vv = v;
                        if (vv < T{0}) vv = T{0};
                        if (vv > T{1}) vv = T{1};
                        return static_cast<unsigned char>(std::lround(static_cast<long double>(vv) * 255.0L));
                    } else {
                        long long vv = static_cast<long long>(v);
                        if (vv < 0) vv = 0;
                        if (vv > 255) vv = 255;
                        return static_cast<unsigned char>(vv);
                    }
                };
                row[3*x + 0] = to_u8(c[0]);
                row[3*x + 1] = to_u8(c[1]);
                row[3*x + 2] = to_u8(c[2]);
            }
            f.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
            if (!f.good()) return false;
        }
        return true;
    }

    /**
     * @brief Loads a binary PPM (P6) file into a 3-channel image.
     * @tparam T arithmetic scalar per-channel type
     * @param path filesystem path to read from
     * @return std::optional<Image<T,3>> containing the loaded image on success; std::nullopt on failure
     * @details Supports P6 with maxval 255. Header comments starting with '#' are skipped per the PPM spec.
     * For floating-point T, values are returned in [0,1]; for integral T, values are returned as 0..255.
     */
    export template <Arithmetic T>
    [[nodiscard]] std::optional<Image<T,3>> load_ppm(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) return std::nullopt;

        auto skip_ws_and_comments = [&f]() {
            int c;
            while ((c = f.peek()) != EOF) {
                if (std::isspace(c)) { f.get(); continue; }
                if (c == '#') {
                    // Skip comment line
                    f.get();
                    while ((c = f.get()) != '\n' && c != EOF) {}
                    continue;
                }
                break;
            }
        };

        auto read_token = [&]() -> std::optional<std::string> {
            skip_ws_and_comments();
            std::string tok;
            int c;
            while ((c = f.peek()) != EOF && !std::isspace(c)) {
                tok.push_back(static_cast<char>(f.get()));
            }
            if (tok.empty()) return std::nullopt;
            return tok;
        };

        // Magic
        auto magic = read_token();
        if (!magic || *magic != "P6") return std::nullopt;
        // Width, height, maxval
        auto wtok = read_token();
        auto htok = read_token();
        auto mtok = read_token();
        if (!wtok || !htok || !mtok) return std::nullopt;
        long w = std::strtol(wtok->c_str(), nullptr, 10);
        long h = std::strtol(htok->c_str(), nullptr, 10);
        long maxv = std::strtol(mtok->c_str(), nullptr, 10);
        if (w <= 0 || h <= 0 || maxv <= 0) return std::nullopt;
        if (maxv != 255) return std::nullopt; // only 8-bit supported
        // Consume single whitespace after header before binary data
        f.get();
        if (!f) return std::nullopt;
        const std::size_t width = static_cast<std::size_t>(w);
        const std::size_t height = static_cast<std::size_t>(h);
        std::vector<unsigned char> buf(width * height * 3);
        f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
        if (static_cast<std::size_t>(f.gcount()) != buf.size()) return std::nullopt;

        Image<T,3> img{width, height};
        auto to_T = [](unsigned char v) -> T {
            if constexpr (std::is_floating_point_v<T>) {
                return static_cast<T>(v) / static_cast<T>(255);
            } else {
                return static_cast<T>(v);
            }
        };
        std::size_t idx = 0;
        for (std::size_t y=0; y<height; ++y) {
            for (std::size_t x=0; x<width; ++x) {
                auto& p = img(x,y);
                p[0] = to_T(buf[idx++]);
                p[1] = to_T(buf[idx++]);
                p[2] = to_T(buf[idx++]);
            }
        }
        return img;
    }
}
