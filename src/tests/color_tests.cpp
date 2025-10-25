import glimmer.color;
import glimmer.vector;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::Color3f;
using glimmer::Color4f;
using glimmer::Color;
using glimmer::Vector;
using glimmer::linear_to_srgb;
using glimmer::srgb_to_linear;
using glimmer::luminance;
using glimmer::saturate;
using glimmer::clamp;
using glimmer::with_alpha;
using glimmer::premultiply;
using glimmer::unpremultiply;
using glimmer::over;

static void test_alias_and_construct() {
    Color3f c{0.1f, 0.2f, 0.3f};
    assert(std::abs(c[0] - 0.1f) < 1e-6f);
    Color4f a = with_alpha(c, 0.5f);
    assert(std::abs(a[3] - 0.5f) < 1e-6f);
}

static void test_saturate_and_clamp() {
    Color3f c{-1.0f, 0.5f, 2.0f};
    auto s = saturate(c);
    assert(s[0] == 0.0f && s[1] == 0.5f && s[2] == 1.0f);
    auto k = clamp(c, -0.5f, 1.5f);
    assert(k[0] == -0.5f && k[1] == 0.5f && k[2] == 1.5f);
}

static void test_transfer_functions() {
    // Known points: 0 -> 0, 1 -> 1
    Color3f z{0.0f, 0.0f, 0.0f};
    auto z_srgb = linear_to_srgb(z);
    for (int i=0;i<3;++i) assert(std::abs(z_srgb[i] - z[i]) < 1e-7f);
    Color3f o{1.0f, 1.0f, 1.0f};
    auto o_srgb = linear_to_srgb(o);
    for (int i=0;i<3;++i) assert(std::abs(o_srgb[i] - o[i]) < 1e-7f);
    // Mid gray linear 0.5 -> sRGB ~ 0.73535698
    Color3f m{0.5f, 0.5f, 0.5f};
    auto m_srgb = linear_to_srgb(m);
    assert(std::abs(m_srgb[0] - 0.735357f) < 1e-5f);
    auto back = srgb_to_linear(m_srgb);
    assert(std::abs(back[0] - 0.5f) < 1e-5f);
}

static void test_luminance() {
    Color3f g{0.3f, 0.3f, 0.3f};
    auto Y = luminance(g);
    // For gray, Y ~= component (weights sum to 1)
    assert(std::abs(Y - 0.3f) < 1e-6f);
}

static void test_premultiply_over() {
    Color4f red = with_alpha(Color3f{1,0,0}, 0.5f);
    Color4f blue = with_alpha(Color3f{0,0,1}, 1.0f);
    auto red_pm = premultiply(red);
    auto blue_pm = premultiply(blue); // unchanged since a=1
    auto out_pm = over(red_pm, blue_pm);
    // Result alpha = 0.5 + 1*(1-0.5) = 1.0
    assert(std::abs(out_pm[3] - 1.0f) < 1e-6f);
    // Unpremultiply to inspect color; should be blend with 50% red over blue
    auto out = unpremultiply(out_pm);
    assert(std::abs(out[0] - 0.5f) < 1e-6f);
    assert(std::abs(out[1] - 0.0f) < 1e-6f);
    assert(std::abs(out[2] - 0.5f) < 1e-6f);
}

int main() {
    test_alias_and_construct();
    test_saturate_and_clamp();
    test_transfer_functions();
    test_luminance();
    test_premultiply_over();
    std::cout << "All color tests passed.\n";
    return 0;
}
