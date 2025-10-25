import glimmer.quaternion;
import glimmer.vector;
import glimmer.matrix;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::Quaternion;
using glimmer::Vector;
using glimmer::Matrix;

static void test_identity_construction() {
    Quaternion<double> q{}; // identity
    assert(q.w() == 1.0);
    assert(q.x() == 0.0 && q.y() == 0.0 && q.z() == 0.0);

    Quaternion<double> p{0.5, 1.0, 2.0, -3.0};
    auto sum = q + p;
    assert(std::abs(sum.w() - 1.5) < 1e-12);
}

static void test_conjugate_inverse() {
    Quaternion<double> q{0.9238795325, 0.3826834324, 0.0, 0.0}; // ~ 45 deg about x
    auto qc = q.conjugate();
    assert(std::abs(qc.x() + q.x()) < 1e-12);
    auto qi = q.inverse();
    // For unit quaternion, inverse equals conjugate
    assert(std::abs(qc.w() - qi.w()) < 1e-9);
    assert(std::abs(qc.x() - qi.x()) < 1e-9);
}

static void test_axis_angle_and_rotation() {
    auto q = Quaternion<double>::from_axis_angle(Vector<double,3>{0,0,1}, M_PI/2); // 90 deg z
    Vector<double,3> x{1,0,0};
    auto y = q.rotate(x);
    assert(std::abs(y[0]) < 1e-12);
    assert(std::abs(y[1] - 1.0) < 1e-12);
    assert(std::abs(y[2]) < 1e-12);
}

static void test_matrix_conversion() {
    auto q = Quaternion<double>::from_axis_angle(Vector<double,3>{0,1,0}, M_PI); // 180 deg y
    auto R = q.to_matrix3();
    // Rotation around Y by 180 flips X and Z
    assert(std::abs(R(0,0) + 1.0) < 1e-12);
    assert(std::abs(R(2,2) + 1.0) < 1e-12);
    assert(std::abs(R(1,1) - 1.0) < 1e-12);

    auto M4 = q.to_matrix4();
    assert(std::abs(M4(3,3) - 1.0) < 1e-12);
}

static void test_multiplication_composition() {
    // Apply 90 deg about Y then 90 deg about X; Z -> X
    auto qx = Quaternion<double>::from_axis_angle(Vector<double,3>{1,0,0}, M_PI/2);
    auto qy = Quaternion<double>::from_axis_angle(Vector<double,3>{0,1,0}, M_PI/2);
    auto q = qx * qy; // apply qy then qx
    Vector<double,3> v{0,0,1};
    auto r = q.rotate(v);
    assert(std::abs(r[0] - 1.0) < 1e-12);
    assert(std::abs(r[1]) < 1e-12);
    assert(std::abs(r[2]) < 1e-12);
}

static void test_slerp() {
    auto qa = Quaternion<double>::from_axis_angle(Vector<double,3>{0,0,1}, 0.0);
    auto qb = Quaternion<double>::from_axis_angle(Vector<double,3>{0,0,1}, M_PI);
    auto qm = slerp(qa, qb, 0.5);
    // Rotating x by qm should be ~90 deg about Z
    Vector<double,3> x{1,0,0};
    auto y = qm.rotate(x);
    assert(std::abs(y[0]) < 1e-6);
    assert(std::abs(y[1] - 1.0) < 1e-6);
}

int main() {
    test_identity_construction();
    test_conjugate_inverse();
    test_axis_angle_and_rotation();
    test_matrix_conversion();
    test_multiplication_composition();
    test_slerp();

    std::cout << "All quaternion tests passed.\n";
    return 0;
}
