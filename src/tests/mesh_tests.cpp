import glimmer.mesh;
import glimmer.ray;
import glimmer.vector;
import glimmer.aabb;
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>

using glimmer::Mesh;
using glimmer::Ray;
using glimmer::Vector;
using glimmer::intersect_triangle;

static void test_intersect_triangle_standalone() {
    Vector<double,3> p0{0,0,0};
    Vector<double,3> p1{1,0,0};
    Vector<double,3> p2{0,1,0};
    Ray<double> r{Vector<double,3>{0.25,0.25,1}, Vector<double,3>{0,0,-1}, 0.0, 100.0};
    auto hit = intersect_triangle(p0,p1,p2,r);
    assert(hit.has_value());
    assert(std::abs(hit->t - 1.0) < 1e-12);
    assert(hit->u > 0 && hit->v > 0 && hit->u + hit->v < 1);
    assert(std::abs(hit->normal[2] - 1.0) < 1e-12);
}

static void test_mesh_two_tris() {
    Mesh<double> m;
    auto i0 = m.add_vertex(Vector<double,3>{0,0,0});
    auto i1 = m.add_vertex(Vector<double,3>{1,0,0});
    auto i2 = m.add_vertex(Vector<double,3>{1,1,0});
    auto i3 = m.add_vertex(Vector<double,3>{0,1,0});
    m.add_triangle(i0,i1,i2);
    m.add_triangle(i0,i2,i3);

    // AABB should cover [0,1]x[0,1]x{0}
    auto box = m.aabb();
    auto mn = box.min();
    auto mx = box.max();
    assert(std::abs(mn[0] - 0.0) < 1e-12 && std::abs(mn[1] - 0.0) < 1e-12 && std::abs(mn[2] - 0.0) < 1e-12);
    assert(std::abs(mx[0] - 1.0) < 1e-12 && std::abs(mx[1] - 1.0) < 1e-12 && std::abs(mx[2] - 0.0) < 1e-12);

    Ray<double> r1{Vector<double,3>{0.25,0.25, 1}, Vector<double,3>{0,0,-1}, 0.0, 100.0};
    auto h1 = m.intersect(r1);
    assert(h1.has_value());
    assert(h1->t > 0);

    Ray<double> r2{Vector<double,3>{2,2, 1}, Vector<double,3>{0,0,-1}, 0.0, 100.0};
    auto h2 = m.intersect(r2);
    assert(!h2.has_value());
}

static void test_aabb_empty_mesh() {
    Mesh<double> m;
    auto box = m.aabb();
    assert(box.empty());
}

int main(){
    test_intersect_triangle_standalone();
    test_mesh_two_tris();
    test_aabb_empty_mesh();
    std::cout << "All mesh tests passed.\n";
    return 0;
}
