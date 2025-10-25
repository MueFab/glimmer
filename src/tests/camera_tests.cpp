import glimmer.camera;
import glimmer.vector;
import glimmer.matrix;
import glimmer.transform;
import glimmer.ray;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::Camera;
using glimmer::Vector;
using glimmer::Transform;
using glimmer::Ray;

static void test_center_ray_points_to_target() {
    Vector<double,3> eye{0,0,5};
    Vector<double,3> target{0,0,0};
    Vector<double,3> up{0,1,0};
    auto cam = Camera<double>::from_look_at(eye, target, up, M_PI/2, 1.0, 0.1, 100.0);
    auto r = cam.generate_ray(0, 0, 1, 1); // single pixel image, center ray at (0,0)
    // Ray origin should be near eye
    auto o = r.origin();
    assert(std::abs(o[0]-eye[0]) < 1e-12 && std::abs(o[1]-eye[1]) < 1e-12 && std::abs(o[2]-eye[2]) < 1e-12);
    // Direction should point roughly towards target (0,0,-1)
    auto d = r.direction();
    assert(std::abs(d[0]) < 1e-12);
    assert(std::abs(d[1]) < 1e-12);
    assert(d[2] < 0.0);
}

static void test_corner_rays_with_aspect() {
    Vector<double,3> eye{0,0,0};
    Vector<double,3> target{0,0,-1};
    Vector<double,3> up{0,1,0};
    auto cam = Camera<double>::from_look_at(eye, target, up, 60.0 * M_PI/180.0, 16.0/9.0, 0.1, 100.0);
    // Top-left corner pixel in a 1920x1080 image
    std::size_t W = 1920, H = 1080;
    auto r_tl = cam.generate_ray(0, 0, W, H);
    auto r_br = cam.generate_ray(W-1, H-1, W, H);
    // y should be positive for top-left (since image y=0 is top)
    assert(r_tl.direction()[1] > 0.0);
    // y should be negative for bottom-right
    assert(r_br.direction()[1] < 0.0);
    // x should be negative for left and positive for right
    assert(r_tl.direction()[0] < 0.0);
    assert(r_br.direction()[0] > 0.0);
}

static void test_viewproj_basic() {
    Vector<double,3> eye{0,0,5};
    Vector<double,3> target{0,0,0};
    Vector<double,3> up{0,1,0};
    auto cam = Camera<double>::from_look_at(eye, target, up, M_PI/2, 1.0, 1.0, 10.0);
    auto VP = cam.viewproj_matrix();
    // A point on -Z axis from camera (0,0, -1 in camera space) is at world (0,0,4)
    Vector<double,3> p_world{0,0,4};
    // Transform to clip space
    Vector<double,4> p4{p_world[0], p_world[1], p_world[2], 1.0};
    auto clip = VP * p4;
    // After perspective divide, z should be in [-1,1]; just check w != 0 and finite
    assert(std::abs(clip[3]) > 1e-12);
}

int main(){
    test_center_ray_points_to_target();
    test_corner_rays_with_aspect();
    test_viewproj_basic();
    std::cout << "All camera tests passed.\n";
    return 0;
}
