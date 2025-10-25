import glimmer.scene_object;
import glimmer.sphere;
import glimmer.mesh;
import glimmer.vector;
import glimmer.transform;
import glimmer.quaternion;
import glimmer.ray;
import glimmer.material;
import glimmer.aabb;
#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>

using glimmer::SceneObject;
using glimmer::Sphere;
using glimmer::Mesh;
using glimmer::Vector;
using glimmer::Transform;
using glimmer::Ray;
using glimmer::Material;
using glimmer::AABB;

static void test_identity_equals_direct_sphere() {
    auto geom = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 1.0);
    auto mat = Material<double>::lambertian(Vector<double,3>{1,1,1});
    Transform<double> xf{}; // identity
    SceneObject<double> obj{geom, mat, xf};

    Ray<double> r{Vector<double,3>{0,0,3}, Vector<double,3>{0,0,-1}, 0.0, 100.0};
    auto h_obj = obj.intersect(r);
    auto h_dir = geom->intersect(r);
    assert(h_obj.has_value() && h_dir.has_value());
    assert(std::abs(h_obj->t - h_dir->t) < 1e-12);
    assert(std::abs(h_obj->normal[2] - h_dir->normal[2]) < 1e-12);
}

static void test_translated_transform_hit() {
    auto geom = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 1.0);
    auto mat = Material<double>::lambertian(Vector<double,3>{0.8,0.2,0.1});
    // Build translation by (0,0,5)
    Vector<double,3> t{0,0,5};
    auto xf = Transform<double>::from_trs(t, glimmer::Quaternion<double>{}, Vector<double,3>{1,1,1});
    SceneObject<double> obj{geom, mat, xf};

    Ray<double> r{Vector<double,3>{0,0,0}, Vector<double,3>{0,0,1}, 0.0, 100.0};
    auto h = obj.intersect(r);
    assert(h.has_value());
    // First hit should be at world z ~ 4 (sphere center at 5, radius 1)
    auto p = r.at(h->t);
    assert(std::abs(p[2] - 4.0) < 1e-9);
    // Normal at the first hit point (approached from -Z toward +Z) points toward -Z on the sphere cap
    assert(p[2] < 5.0); // ensure front cap
    assert(std::abs(h->normal[2] + 1.0) < 1e-9);
}

static void test_aabb_with_scale() {
    auto geom = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 1.0);
    auto mat = Material<double>::lambertian(Vector<double,3>{0.5,0.5,0.5});
    // Non-uniform scale 2x in X, 3x in Y, 0.5x in Z and translation (1,-2,3)
    Vector<double,3> t{1,-2,3};
    auto xf = Transform<double>::from_trs(t, glimmer::Quaternion<double>{}, Vector<double,3>{2,3,0.5});
    SceneObject<double> obj{geom, mat, xf};
    auto box = obj.aabb();
    auto mn = box.min();
    auto mx = box.max();
    // Sphere radius r=1 scales to extents (2,3,0.5)
    assert(std::abs(mn[0] - (t[0] - 2.0)) < 1e-9);
    assert(std::abs(mx[0] - (t[0] + 2.0)) < 1e-9);
    assert(std::abs(mn[1] - (t[1] - 3.0)) < 1e-9);
    assert(std::abs(mx[1] - (t[1] + 3.0)) < 1e-9);
    assert(std::abs(mn[2] - (t[2] - 0.5)) < 1e-9);
    assert(std::abs(mx[2] - (t[2] + 0.5)) < 1e-9);
}

static void test_scaling_adjusts_ray_params() {
    // Sphere at origin radius 1
    auto geom = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 1.0);
    auto mat = Material<double>::lambertian(Vector<double,3>{1,1,1});
    // World transform scales by 0.5 uniformly (object appears smaller in world); inv_world scales by 2.
    auto xf = Transform<double>::from_trs(Vector<double,3>{0,0,0}, glimmer::Quaternion<double>{}, Vector<double,3>{0.5,0.5,0.5});
    SceneObject<double> obj{geom, mat, xf};
    // World ray starts at z=2 looking to origin; with scale 0.5, sphere radius is 0.5, so first hit is at z=0.5
    // World-space distance to first hit is 1.5 -> require tmax >= 1.5
    Ray<double> r{Vector<double,3>{0,0,2}, Vector<double,3>{0,0,-1}, 0.0, 2.0};
    auto h = obj.intersect(r);
    assert(h.has_value());
    // World t should be ~1.5
    assert(std::abs(h->t - 1.5) < 1e-9);
    // Normal should point outward from sphere: at (0,0,0.5) it points +Z
    assert(h->normal[2] > 0.0);
}

int main(){
    test_identity_equals_direct_sphere();
    test_translated_transform_hit();
    test_aabb_with_scale();
    test_scaling_adjusts_ray_params();
    std::cout << "All scene object tests passed.\n";
    return 0;
}
