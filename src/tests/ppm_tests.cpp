import glimmer.ppm;
import glimmer.image;
import glimmer.color;
#include <cassert>
#include <cmath>
#include <iostream>
#include <filesystem>

using glimmer::Image;
using glimmer::Color3f;
using glimmer::save_ppm;
using glimmer::load_ppm;

static void test_round_trip_float() {
    Image<float,3> img{3,2};
    // Fill with a pattern
    img(0,0) = Color3f{0.0f, 0.0f, 0.0f};
    img(1,0) = Color3f{1.0f, 0.5f, 0.25f};
    img(2,0) = Color3f{0.2f, 0.4f, 0.6f};
    img(0,1) = Color3f{1.0f, 1.0f, 1.0f};
    img(1,1) = Color3f{0.3f, 0.7f, 0.9f};
    img(2,1) = Color3f{0.5f, 0.5f, 0.5f};

    const char* fname = "ppm_rt.ppm";
    bool ok = save_ppm(img, fname);
    assert(ok);
    auto loaded = load_ppm<float>(fname);
    assert(loaded.has_value());
    assert(loaded->width() == img.width() && loaded->height() == img.height());
    // Compare within 1/255 + small epsilon due to quantization
    const float eps = 1.0f/255.0f + 1e-5f;
    for (std::size_t y=0; y<img.height(); ++y) {
        for (std::size_t x=0; x<img.width(); ++x) {
            auto a = img(x,y);
            auto b = (*loaded)(x,y);
            for (int c=0;c<3;++c) {
                assert(std::abs(a[c] - b[c]) <= eps);
            }
        }
    }
}

static void test_load_nonexistent() {
    auto none = load_ppm<float>("this_file_does_not_exist.ppm");
    assert(!none.has_value());
}

int main() {
    test_round_trip_float();
    test_load_nonexistent();
    std::cout << "All ppm tests passed.\n";
    return 0;
}
