import glimmer.vector;
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

using glimmer::Vector;

static void test_basic_construction() {
    Vector<int, 3> a{};
    for (std::size_t i = 0; i < a.size(); ++i) assert(a[i] == 0);

    Vector<double, 3> b{1.0, 2.0, 3.0};
    assert(b[0] == 1.0 && b[1] == 2.0 && b[2] == 3.0);

    auto c = Vector<float, 3>::ones();
    assert(c[0] == 1.0f && c[1] == 1.0f && c[2] == 1.0f);

    auto z = Vector<float, 4>::zeros();
    for (std::size_t i = 0; i < z.size(); ++i) assert(z[i] == 0.0f);

    auto e1 = Vector<int, 3>::unit(1);
    assert(e1[0] == 0 && e1[1] == 1 && e1[2] == 0);
}

static void test_element_access() {
    Vector<int, 2> v{3, 4};
    v[0] = 5;
    assert(v[0] == 5);
    bool threw = false;
    try { (void)v.at(2); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

static void test_arithmetic() {
    Vector<double, 3> a{1.0, 2.0, 3.0};
    Vector<double, 3> b{4.0, -2.0, 0.5};

    auto c = a + b;
    assert(std::abs(c[0] - 5.0) < 1e-12);
    assert(std::abs(c[1] - 0.0) < 1e-12);
    assert(std::abs(c[2] - 3.5) < 1e-12);

    c -= a;
    assert(std::abs(c[0] - 4.0) < 1e-12);
    assert(std::abs(c[1] + 2.0) < 1e-12);
    assert(std::abs(c[2] - 0.5) < 1e-12);

    auto d = 2.0 * a;
    assert(std::abs(d[0] - 2.0) < 1e-12);
    assert(std::abs(d[1] - 4.0) < 1e-12);
    assert(std::abs(d[2] - 6.0) < 1e-12);

    auto e = d / 2.0;
    assert(std::abs(e[0] - 1.0) < 1e-12);
    assert(std::abs(e[1] - 2.0) < 1e-12);
    assert(std::abs(e[2] - 3.0) < 1e-12);
}

static void test_dot_cross_norm() {
    Vector<double, 3> x{1.0, 0.0, 0.0};
    Vector<double, 3> y{0.0, 1.0, 0.0};
    Vector<double, 3> z{0.0, 0.0, 1.0};

    auto d = dot(x, x + 2.0 * y + 3.0 * z);
    assert(std::abs(d - 1.0) < 1e-12);

    auto c = cross(x, y);
    assert(std::abs(c[0] - 0.0) < 1e-12);
    assert(std::abs(c[1] - 0.0) < 1e-12);
    assert(std::abs(c[2] - 1.0) < 1e-12);

    Vector<double, 3> a{3.0, 4.0, 12.0};
    auto n = a.norm();
    assert(std::abs(n - 13.0) < 1e-12);

    auto u = a.normalized();
    assert(std::abs(u.norm() - 1.0) < 1e-9);
}

static void test_min_max_eq() {
    Vector<int, 3> a{1, 5, 3};
    Vector<int, 3> b{2, 1, 3};
    auto mn = Vector<int,3>::min(a, b);
    auto mx = Vector<int,3>::max(a, b);
    assert(mn[0] == 1 && mn[1] == 1 && mn[2] == 3);
    assert(mx[0] == 2 && mx[1] == 5 && mx[2] == 3);

    assert(a != b);
    assert(a == a);
}

static void test_zero_normalization() {
    Vector<double, 2> z{};
    auto u = z.normalized();
    assert(u[0] == 0.0 && u[1] == 0.0);
}

static void test_lerp() {
    Vector<double, 3> a{0.0, 0.0, 0.0};
    Vector<double, 3> b{10.0, -10.0, 20.0};

    auto p0 = lerp(a, b, 0.0);
    assert(std::abs(p0[0] - 0.0) < 1e-12);
    assert(std::abs(p0[1] - 0.0) < 1e-12);
    assert(std::abs(p0[2] - 0.0) < 1e-12);

    auto p1 = lerp(a, b, 1.0);
    assert(std::abs(p1[0] - 10.0) < 1e-12);
    assert(std::abs(p1[1] + 10.0) < 1e-12);
    assert(std::abs(p1[2] - 20.0) < 1e-12);

    auto pm = lerp(a, b, 0.5);
    assert(std::abs(pm[0] - 5.0) < 1e-12);
    assert(std::abs(pm[1] + 5.0) < 1e-12);
    assert(std::abs(pm[2] - 10.0) < 1e-12);

    auto pe = lerp(a, b, 1.5);
    assert(std::abs(pe[0] - 15.0) < 1e-12);
    assert(std::abs(pe[1] + 15.0) < 1e-12);
    assert(std::abs(pe[2] - 30.0) < 1e-12);

    // Integer vector lerp truncates components after computation
    Vector<int, 2> ai{0, 0};
    Vector<int, 2> bi{3, 5};
    auto mi = lerp(ai, bi, 0.5); // common_type is double, cast back to int
    assert(mi[0] == 1); // 1.5 -> 1
    assert(mi[1] == 2); // 2.5 -> 2
}

static void test_resize_dim_and_homogeneous() {
    // Extend 3 -> 5 with fill
    Vector<int,3> a{1,2,3};
    auto a5 = glimmer::resize_dim<int,3,5>(a, 9);
    assert(a5[0]==1 && a5[1]==2 && a5[2]==3 && a5[3]==9 && a5[4]==9);
    // Truncate 4 -> 2
    Vector<int,4> b{7,8,9,10};
    auto b2 = glimmer::resize_dim<int,4,2>(b);
    assert(b2[0]==7 && b2[1]==8);
    // Homogeneous helpers
    Vector<double,3> p{1,2,3};
    auto p4 = glimmer::to_homogeneous_point(p);
    assert(std::abs(p4[0]-1.0)<1e-12 && std::abs(p4[1]-2.0)<1e-12 && std::abs(p4[2]-3.0)<1e-12 && std::abs(p4[3]-1.0)<1e-12);
    Vector<double,3> d{4,5,6};
    auto d4 = glimmer::to_homogeneous_dir(d);
    assert(std::abs(d4[0]-4.0)<1e-12 && std::abs(d4[1]-5.0)<1e-12 && std::abs(d4[2]-6.0)<1e-12 && std::abs(d4[3]-0.0)<1e-12);
}

int main() {
    test_basic_construction();
    test_element_access();
    test_arithmetic();
    test_dot_cross_norm();
    test_min_max_eq();
    test_zero_normalization();
    test_lerp();
    test_resize_dim_and_homogeneous();

    std::cout << "All vector tests passed.\n";
    return 0;
}
