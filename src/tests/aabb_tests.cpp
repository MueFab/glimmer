import glimmer.aabb;
import glimmer.vector;
import glimmer.ray;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::AABB;
using glimmer::Vector;
using glimmer::Ray;

static void test_empty_and_expand() {
    AABB<double> b; // empty
    assert(b.empty());
    b.expand(Vector<double,3>{1,2,3});
    assert(!b.empty());
    assert(std::abs(b.min()[0] - 1.0) < 1e-12);
    assert(std::abs(b.max()[2] - 3.0) < 1e-12);

    b.expand(Vector<double,3>{-1,0,5});
    auto e = b.extent();
    assert(std::abs(b.min()[0] + 1.0) < 1e-12);
    assert(std::abs(b.max()[2] - 5.0) < 1e-12);
    assert(std::abs(e[0] - 2.0) < 1e-12);
}

static void test_union_and_contains_overlaps() {
    AABB<double> a{ Vector<double,3>{0,0,0}, Vector<double,3>{1,1,1} };
    AABB<double> b{ Vector<double,3>{0.5,0.5,0.5}, Vector<double,3>{2,2,2} };
    auto u = a.united(b);
    assert(std::abs(u.min()[0] - 0.0) < 1e-12);
    assert(std::abs(u.max()[2] - 2.0) < 1e-12);

    assert(a.contains(Vector<double,3>{0.25,0.25,0.25}));
    assert(!a.contains(Vector<double,3>{-0.1,0,0}));

    assert(u.contains(a));
    assert(u.overlaps(b));
    assert(a.overlaps(b));
}

static void test_ray_intersect_hits() {
    AABB<double> box{ Vector<double,3>{-1,-1,-1}, Vector<double,3>{1,1,1} };
    // Straight through center
    Ray<double> r1{ Vector<double,3>{0,0,-5}, Vector<double,3>{0,0,1}, 0.0, 100.0 };
    auto h1 = box.intersect(r1);
    assert(h1.has_value());
    assert(h1->t_near > 0 && h1->t_far > h1->t_near);

    // Parallel to X axis, inside YZ slabs
    Ray<double> r2{ Vector<double,3>{-5,0,0}, Vector<double,3>{1,0,0}, 0.0, 100.0 };
    auto h2 = box.intersect(r2);
    assert(h2.has_value());
    assert(std::abs(h2->t_near - 4.0) < 1e-12);

    // Miss (outside parallel)
    Ray<double> r3{ Vector<double,3>{-5,2,0}, Vector<double,3>{1,0,0}, 0.0, 100.0 };
    auto h3 = box.intersect(r3);
    assert(!h3.has_value());

    // Starting inside
    Ray<double> r4{ Vector<double,3>{0,0,0}, Vector<double,3>{0,1,0}, 0.0, 100.0 };
    auto h4 = box.intersect(r4);
    assert(h4.has_value());
    assert(h4->t_near <= 0.0 && h4->t_far > 0.0);
}

int main() {
    test_empty_and_expand();
    test_union_and_contains_overlaps();
    test_ray_intersect_hits();
    std::cout << "All AABB tests passed.\n";
    return 0;
}
