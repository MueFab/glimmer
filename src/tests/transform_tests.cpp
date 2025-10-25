import glimmer.transform;
import glimmer.vector;
import glimmer.matrix;
import glimmer.quaternion;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::Transform;
using glimmer::Vector;
using glimmer::Matrix;
using glimmer::Quaternion;

static void test_identity_and_inverse() {
    Transform<double> T{}; // identity
    Vector<double,3> p{1,2,3};
    auto q = glimmer::transform_point(T, p);
    assert(std::abs(q[0]-1) < 1e-12 && std::abs(q[1]-2) < 1e-12 && std::abs(q[2]-3) < 1e-12);
    auto Tinverse = T.inverse();
    auto r = glimmer::transform_point(Tinverse, q);
    assert(std::abs(r[0]-1) < 1e-12 && std::abs(r[1]-2) < 1e-12 && std::abs(r[2]-3) < 1e-12);
}

static void test_trs_point_dir_normal() {
    Vector<double,3> t{3,-2,1};
    auto q = Quaternion<double>::from_axis_angle(Vector<double,3>{0,0,1}, M_PI/2);
    Vector<double,3> s{2, 1, 3};
    auto Trs = Transform<double>::from_trs(t, q, s);

    Vector<double,3> p{1,0,0};
    auto p_world = glimmer::transform_point(Trs, p);
    // rotate 90 about Z: (1,0,0) -> (0,1,0), then scale then translate
    // After scale (2,1,3) applied after rotation: (0,1,0) -> (0,1,0)
    assert(std::abs(p_world[0] - 3.0) < 1e-12);
    assert(std::abs(p_world[1] - (-1.0)) < 1e-12);
    assert(std::abs(p_world[2] - 1.0) < 1e-12);

    Vector<double,3> vx{1,0,0};
    auto v_world = glimmer::transform_direction(Trs, vx);
    // rotation then scale on direction: (1,0,0)->(0,1,0)->(0,1,0)
    assert(std::abs(v_world[0]) < 1e-12);
    assert(std::abs(v_world[1] - 1.0) < 1e-12);
    assert(std::abs(v_world[2]) < 1e-12);

    // Normal for a plane with normal along X in object space should transform with inverse-transpose
    Vector<double,3> n{1,0,0};
    auto n_world = glimmer::transform_normal(Trs, n);
    // With non-uniform scale, normal will rotate by -90 about Z to world Y and scale by inverse factors
    // Normal direction check (sign/scale ignored): should align with world Y
    // We just check that X,Z ~ 0 and Y != 0
    assert(std::abs(n_world[0]) < 1e-12);
    assert(std::abs(n_world[2]) < 1e-12);
    assert(std::abs(n_world[1]) > 1e-12);
}

static void test_compose() {
    auto T1 = Transform<double>::from_trs(Vector<double,3>{1,0,0}, Quaternion<double>{}, Vector<double,3>{1,1,1});
    auto T2 = Transform<double>::from_trs(Vector<double,3>{0,2,0}, Quaternion<double>{}, Vector<double,3>{2,2,2});
    auto Tc = T1 * T2; // apply T2 then T1
    Vector<double,3> p{1,1,1};
    auto pc = glimmer::transform_point(Tc, p);
    auto p_manual = glimmer::transform_point(T1, glimmer::transform_point(T2, p));
    assert(std::abs(pc[0]-p_manual[0]) < 1e-12);
    assert(std::abs(pc[1]-p_manual[1]) < 1e-12);
    assert(std::abs(pc[2]-p_manual[2]) < 1e-12);
}

static void test_look_at() {
    Vector<double,3> eye{0,0,5};
    Vector<double,3> target{0,0,0};
    Vector<double,3> up{0,1,0};
    auto Vcw = Transform<double>::look_at(eye, target, up); // stores camera-to-world
    // A forward camera ray (0,0,-1) in camera space should map to world direction (0,0,-1)
    Vector<double,3> cam_dir{0,0,-1};
    auto world_dir = glimmer::transform_direction(Vcw, cam_dir);
    assert(std::abs(world_dir[0]) < 1e-12);
    assert(std::abs(world_dir[1]) < 1e-12);
    assert(std::abs(world_dir[2] + 1.0) < 1e-12);
    // Camera origin maps to world eye
    Vector<double,3> cam_origin{0,0,0};
    auto world_origin = glimmer::transform_point(Vcw, cam_origin);
    assert(std::abs(world_origin[0]-eye[0]) < 1e-12);
    assert(std::abs(world_origin[1]-eye[1]) < 1e-12);
    assert(std::abs(world_origin[2]-eye[2]) < 1e-12);
}

static void test_projection_helpers() {
    auto P = Transform<float>::perspective(60.0f * (float)M_PI/180.0f, 16.0f/9.0f, 0.1f, 100.0f);
    // Basic sanity: bottom-right corner non-zero and M(3,2) == -1 for OpenGL-style
    assert(std::abs(P(3,2) + 1.0f) < 1e-6);
    assert(std::abs(P(0,0)) > 1e-6);

    auto O = Transform<float>::orthographic(-1,1,-1,1,0.1f, 10.0f);
    // Orthographic should have last row [0,0,0,1]
    assert(std::abs(O(3,3) - 1.0f) < 1e-6);
}

int main() {
    test_identity_and_inverse();
    test_trs_point_dir_normal();
    test_compose();
    test_look_at();
    test_projection_helpers();

    std::cout << "All transform tests passed.\n";
    return 0;
}
