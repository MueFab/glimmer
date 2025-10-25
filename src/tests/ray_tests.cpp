import glimmer.ray;
import glimmer.vector;
#include <cassert>
#include <cmath>
#include <iostream>

using glimmer::Ray;
using glimmer::Vector;

static void test_construction_and_at() {
    Ray<double> r{Vector<double,3>{1,2,3}, Vector<double,3>{0,0,1}};
    auto p = r.at(5.0);
    assert(std::abs(p[0] - 1.0) < 1e-12);
    assert(std::abs(p[1] - 2.0) < 1e-12);
    assert(std::abs(p[2] - 8.0) < 1e-12);
}

static void test_normalized_dir() {
    Ray<double> r{Vector<double,3>{0,0,0}, Vector<double,3>{0,3,4}};
    auto rn = r.normalized_dir();
    auto d = rn.direction();
    assert(std::abs(d[0]) < 1e-12);
    assert(std::abs(d[1] - 0.6) < 1e-12);
    assert(std::abs(d[2] - 0.8) < 1e-12);
}

static void test_range_and_validity() {
    Ray<double> r{Vector<double,3>{0,0,0}, Vector<double,3>{1,0,0}, 0.5, 10.0};
    assert(r.is_valid());
    auto p0 = r.at(r.tmin());
    auto p1 = r.at(r.tmax());
    assert(std::abs(p0[0] - 0.5) < 1e-12);
    assert(std::abs(p1[0] - 10.0) < 1e-12);
}

int main() {
    test_construction_and_at();
    test_normalized_dir();
    test_range_and_validity();
    std::cout << "All ray tests passed.\n";
    return 0;
}
