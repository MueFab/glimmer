import glimmer.sphere;
import glimmer.ray;
import glimmer.vector;
import glimmer.aabb;
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>

using glimmer::Sphere;
using glimmer::Ray;
using glimmer::Vector;

static void test_aabb() {
    Sphere<double> s{Vector<double,3>{1,2,3}, 2.0};
    auto box = s.aabb();
    auto mn = box.min();
    auto mx = box.max();
    assert(std::abs(mn[0] - (1.0 - 2.0)) < 1e-12);
    assert(std::abs(mn[1] - (2.0 - 2.0)) < 1e-12);
    assert(std::abs(mn[2] - (3.0 - 2.0)) < 1e-12);
    assert(std::abs(mx[0] - (1.0 + 2.0)) < 1e-12);
    assert(std::abs(mx[1] - (2.0 + 2.0)) < 1e-12);
    assert(std::abs(mx[2] - (3.0 + 2.0)) < 1e-12);
}

static void test_basic_hit() {
    Sphere<double> s{Vector<double,3>{0,0,0}, 1.0};
    Ray<double> r{Vector<double,3>{0,0,3}, Vector<double,3>{0,0,-1}, 0.0, 100.0};
    auto hit = s.intersect(r);
    assert(hit.has_value());
    assert(std::abs(hit->t - 2.0) < 1e-12);
    assert(std::abs(hit->normal[0]) < 1e-12 && std::abs(hit->normal[1]) < 1e-12 && std::abs(hit->normal[2] - 1.0) < 1e-12);
}

static void test_miss() {
    Sphere<double> s{Vector<double,3>{0,0,0}, 1.0};
    Ray<double> r{Vector<double,3>{0,0,3}, Vector<double,3>{0,1,0}, 0.0, 100.0};
    auto hit = s.intersect(r);
    assert(!hit.has_value());
}

static void test_inside() {
    Sphere<double> s{Vector<double,3>{0,0,0}, 1.0};
    Ray<double> r{Vector<double,3>{0,0,0}, Vector<double,3>{0,0,1}, 0.0, 100.0};
    auto hit = s.intersect(r);
    assert(hit.has_value());
    assert(hit->t > 0.0);
    assert(std::abs(hit->normal[2] - 1.0) < 1e-12);
}

static void test_tangent() {
    Sphere<double> s{Vector<double,3>{0,0,0}, 1.0};
    Ray<double> r{Vector<double,3>{1,0,0}, Vector<double,3>{0,1,0}, 0.0, 100.0};
    auto hit = s.intersect(r);
    assert(hit.has_value());
    assert(std::abs(hit->t - 0.0) < 1e-12);
    assert(std::abs(hit->normal[0] - 1.0) < 1e-12);
}

int main() {
    test_aabb();
    test_basic_hit();
    test_miss();
    test_inside();
    test_tangent();
    std::cout << "All sphere tests passed.\n";
    return 0;
}
