import glimmer.scene;
import glimmer.scene_object;
import glimmer.sphere;
import glimmer.mesh;
import glimmer.vector;
import glimmer.transform;
import glimmer.camera;
import glimmer.material;
import glimmer.aabb;
import glimmer.quaternion;
#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>

using glimmer::Scene;
using glimmer::SceneObject;
using glimmer::Sphere;
using glimmer::Mesh;
using glimmer::Vector;
using glimmer::Transform;
using glimmer::Camera;
using glimmer::Material;
using glimmer::AABB;

static void test_construct_and_props() {
    // Camera looking at -Z from origin
    auto cam = Camera<double>::from_look_at(Vector<double,3>{0,0,0}, Vector<double,3>{0,0,-1}, Vector<double,3>{0,1,0},
                                            M_PI/3, 16.0/9.0, 0.1, 100.0);
    Scene<double> scene{cam, Vector<double,3>{0.1, 0.2, 0.3}};
    assert(scene.background()[0] == 0.1);
    assert(scene.camera().aspect() == 16.0/9.0);
    assert(scene.empty());
}

static void test_add_objects_and_aabb() {
    auto cam = Camera<double>::from_look_at(Vector<double,3>{0,0,0}, Vector<double,3>{0,0,-1}, Vector<double,3>{0,1,0},
                                            M_PI/3, 1.0, 0.1, 100.0);
    Scene<double> scene{cam, Vector<double,3>{0,0,0}};

    auto geom1 = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 1.0);
    auto geom2 = std::make_shared<Sphere<double>>(Vector<double,3>{0,0,0}, 0.5);
    auto mat = Material<double>::lambertian(Vector<double,3>{1,1,1});
    auto xf1 = Transform<double>::from_trs(Vector<double,3>{0,0,0}, glimmer::Quaternion<double>{}, Vector<double,3>{1,1,1});
    auto xf2 = Transform<double>::from_trs(Vector<double,3>{5,0,0}, glimmer::Quaternion<double>{}, Vector<double,3>{2,1,1});

    SceneObject<double> o1{geom1, mat, xf1};
    SceneObject<double> o2{geom2, mat, xf2};

    scene.add_object(o1);
    scene.add_object(std::move(o2));

    assert(scene.size() == 2);
    auto box = scene.aabb();
    assert(!box.empty());
    auto mn = box.min();
    auto mx = box.max();
    // o1: sphere radius 1 at origin -> [-1,1] in X
    // o2: sphere radius 0.5 scaled by x2 at translation 5 -> extent in X is 1.0 around 5 -> [4,6]
    // Union in X should be [-1, 6]
    assert(std::abs(mn[0] - (-1.0)) < 1e-9);
    assert(std::abs(mx[0] - (6.0)) < 1e-9);
}

int main(){
    test_construct_and_props();
    test_add_objects_and_aabb();
    std::cout << "All scene tests passed.\n";
    return 0;
}
