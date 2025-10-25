import glimmer.matrix;
import glimmer.vector;
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

using glimmer::Matrix;
using glimmer::Vector;

static void test_construction_access() {
    Matrix<int, 2, 3> a{}; // zeros
    for (std::size_t r = 0; r < a.rows(); ++r)
        for (std::size_t c = 0; c < a.cols(); ++c)
            assert(a(r,c) == 0);

    Matrix<double, 2, 2> b{1.0, 2.0,
                           3.0, 4.0};
    assert(b(0,0) == 1.0 && b(0,1) == 2.0);
    assert(b(1,0) == 3.0 && b(1,1) == 4.0);

    bool threw = false;
    try { (void)b.at(2,0); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

static void test_identity_fill() {
    auto I = Matrix<int, 3, 3>::identity();
    for (std::size_t r = 0; r < 3; ++r)
        for (std::size_t c = 0; c < 3; ++c)
            assert(I(r,c) == (r==c ? 1 : 0));

    auto F = Matrix<float, 2, 3>::fill(2.5f);
    for (auto v : F) assert(std::abs(v - 2.5f) < 1e-6f);
}

static void test_arithmetic_scalar() {
    Matrix<double, 2, 2> a{1,2,3,4};
    Matrix<double, 2, 2> b{5,6,7,8};
    auto c = a + b;
    assert(std::abs(c(0,0) - 6) < 1e-12);
    assert(std::abs(c(1,1) - 12) < 1e-12);
    c -= a;
    assert(std::abs(c(0,1) - 6) < 1e-12);

    auto d = 2.0 * a;
    assert(std::abs(d(1,0) - 6) < 1e-12);
    d /= 2.0;
    assert(std::abs(d(1,0) - 3) < 1e-12);
}

static void test_matmul_and_matvec() {
    Matrix<double, 2, 3> A{1,2,3,
                           4,5,6};
    Matrix<double, 3, 2> B{7,8,
                           9,10,
                           11,12};
    auto C = A * B; // 2x2
    assert(std::abs(C(0,0) - (1*7 + 2*9 + 3*11)) < 1e-12);
    assert(std::abs(C(1,1) - (4*8 + 5*10 + 6*12)) < 1e-12);

    Vector<double, 3> x{1,2,3};
    auto y = A * x; // 2-vector
    assert(std::abs(y[0] - (1*1 + 2*2 + 3*3)) < 1e-12);
    assert(std::abs(y[1] - (4*1 + 5*2 + 6*3)) < 1e-12);
}

static void test_transpose() {
    Matrix<int, 2, 3> A{1,2,3,
                        4,5,6};
    auto AT = A.transposed(); // 3x2
    assert(AT(0,0) == 1 && AT(1,0) == 2 && AT(2,0) == 3);
    assert(AT(0,1) == 4 && AT(1,1) == 5 && AT(2,1) == 6);

    Matrix<int, 3, 3> B{1,2,3,
                        4,5,6,
                        7,8,9};
    B.transpose_in_place();
    assert(B(0,1) == 4 && B(1,0) == 2);
}

static void test_det_inverse_2x2() {
    Matrix<double, 2, 2> A{4,7,
                           2,6};
    auto det = A.det();
    assert(std::abs(det - (4*6 - 7*2)) < 1e-12);
    auto inv = A.inverse();
    auto I = A * inv;
    // Check approximately identity
    assert(std::abs(I(0,0) - 1.0) < 1e-9);
    assert(std::abs(I(1,1) - 1.0) < 1e-9);
    assert(std::abs(I(0,1)) < 1e-9);
    assert(std::abs(I(1,0)) < 1e-9);
}

static void test_det_inverse_3x3() {
    Matrix<double,3,3> A{3,0,2,
                         2,0,-2,
                         0,1,1};
    auto det = A.det();
    assert(std::abs(det - 10.0) < 1e-12);
    auto inv = A.inverse();
    auto I = A * inv;
    assert(std::abs(I(0,0) - 1.0) < 1e-9);
    assert(std::abs(I(1,1) - 1.0) < 1e-9);
    assert(std::abs(I(2,2) - 1.0) < 1e-9);
}

static void test_det_inverse_4x4() {
    Matrix<double,4,4> A{
        5, 2, 1, -1,
        0, 3, -1, 2,
        2, 0, 4, 1,
        1, -2, 0, 3
    };
    auto det = A.det();
    // Determinant computed externally (e.g., Python/NumPy): 5*3*4*3 and interactions; just ensure non-zero
    assert(std::abs(det) > 1e-9);
    auto inv = A.inverse();
    auto I = A * inv;
    for (std::size_t r = 0; r < 4; ++r)
        for (std::size_t c = 0; c < 4; ++c)
            assert(std::abs(I(r,c) - (r==c ? 1.0 : 0.0)) < 1e-7);
}

static void test_singular_throws() {
    Matrix<int,2,2> Z{}; // zero matrix
    bool threw = false;
    try { (void)Z.inverse(); }
    catch (const std::domain_error&) { threw = true; }
    assert(threw);
}

static void test_det_inverse_5x5() {
    Matrix<double,5,5> A{
        2,  1,  0, -1,  3,
        4, -2,  1,  0,  5,
        0,  3,  2,  1, -1,
        1,  0, -1,  4,  2,
        3,  2,  1,  0,  1
    };
    auto det = A.det();
    assert(std::abs(det) > 1e-9);
    auto inv = A.inverse();
    auto I = A * inv;
    for (std::size_t r = 0; r < 5; ++r)
        for (std::size_t c = 0; c < 5; ++c)
            assert(std::abs(I(r,c) - (r==c ? 1.0 : 0.0)) < 1e-7);
}

static void test_singular_5x5_throws() {
    // Duplicate row to ensure singularity
    Matrix<double,5,5> S{
        1, 2, 3, 4, 5,
        0, 1, 0, 1, 0,
        1, 2, 3, 4, 5, // same as row 0
        2, 0, 1, 0, 2,
        0, 0, 0, 0, 0
    };
    bool threw = false;
    try { (void)S.inverse(); }
    catch (const std::domain_error&) { threw = true; }
    assert(threw);
    // determinant should be zero
    assert(std::abs(S.det()) == 0.0);
}

int main() {
    test_construction_access();
    test_identity_fill();
    test_arithmetic_scalar();
    test_matmul_and_matvec();
    test_transpose();
    test_det_inverse_2x2();
    test_det_inverse_3x3();
    test_det_inverse_4x4();
    test_det_inverse_5x5();
    test_singular_throws();
    test_singular_5x5_throws();

    std::cout << "All matrix tests passed.\n";
    return 0;
}