import glimmer.material;
import glimmer.color;
import glimmer.vector;
#include <cassert>
#include <iostream>

using glimmer::Material;
using glimmer::Color3f;
using glimmer::Vector;
using M = Material<float>;

static void test_default_and_lambertian()
{
    Material<float> m{}; // default black opaque
    auto a = m.albedo();
    assert(a[0] == 0.0f && a[1] == 0.0f && a[2] == 0.0f);

    auto red = Color3f{1.0f, 0.0f, 0.0f};
    auto l = Material<float>::lambertian(red);
    auto al = l.albedo();
    assert(al[0] == 1.0f && al[1] == 0.0f && al[2] == 0.0f);
    assert(std::abs(l.roughness() - 1.0f) < 1e-7f);
    assert(std::abs(l.transparency() - 0.0f) < 1e-7f);
}

static void test_metal_clamp_roughness()
{
    auto gray = Color3f{0.8f, 0.8f, 0.8f};
    auto m0 = Material<float>::metal(gray, -1.0f);
    assert(std::abs(m0.roughness() - 0.0f) < 1e-7f);
    auto m1 = Material<float>::metal(gray, 0.5f);
    assert(std::abs(m1.roughness() - 0.5f) < 1e-7f);
    auto m2 = Material<float>::metal(gray, 2.0f);
    assert(std::abs(m2.roughness() - 1.0f) < 1e-7f);
}

static void test_emissive_and_equality()
{
    auto e = Material<float>::emissive(Color3f{10.0f, 1.0f, 0.0f});
    auto r = e.radiance();
    assert(std::abs(r[0] - 10.0f) < 1e-6f);
    // Default emissive power should be 1
    assert(std::abs(e.emission() - 1.0f) < 1e-6f);
    // Explicit power variant
    auto e_pow = Material<float>::emissive(Color3f{10.0f, 1.0f, 0.0f}, 3.5f);
    assert(std::abs(e_pow.emission() - 3.5f) < 1e-6f);
    // Equality should include emission
    auto e2 = Material<float>::emissive(Color3f{10.0f, 1.0f, 0.0f});
    assert(e == e2);
    assert(e != e_pow);
    auto l = Material<float>::lambertian(Color3f{0.5f, 0.5f, 0.5f});
    assert(e != l);
}

static void test_glass_and_transparency()
{
    auto g = M::glass(Color3f{1, 1, 1}, 0.25f, 1.2f); // transparency clamped to 1
    assert(std::abs(g.roughness() - 0.25f) < 1e-6f);
    assert(std::abs(g.transparency() - 1.0f) < 1e-6f);
    auto g2 = M::glass(Color3f{0.2f, 0.3f, 0.4f}, -0.5f, -1.0f);
    assert(std::abs(g2.roughness() - 0.0f) < 1e-6f);
    assert(std::abs(g2.transparency() - 0.0f) < 1e-6f);
}

static void test_generic_params()
{
    auto m = M::from_params(Color3f{0.1f, 0.2f, 0.3f}, 0.7f, 0.4f, Color3f{2.0f, 0.0f, 0.0f});
    assert(std::abs(m.roughness() - 0.7f) < 1e-6f);
    assert(std::abs(m.transparency() - 0.4f) < 1e-6f);
    assert(std::abs(m.radiance()[0] - 2.0f) < 1e-6f);
}

int main()
{
    test_default_and_lambertian();
    test_metal_clamp_roughness();
    test_emissive_and_equality();
    test_glass_and_transparency();
    test_generic_params();
    std::cout << "All material tests passed.\n";
    return 0;
}