import glimmer.obj;
import glimmer.mesh;
import glimmer.vector;
import glimmer.aabb;
#include <cassert>
#include <sstream>
#include <iostream>

using glimmer::Mesh;
using glimmer::Vector;
using glimmer::load_obj;

static void test_load_simple_triangle(){
    const char* obj = R"OBJ(
# a single triangle
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)OBJ";
    std::istringstream ss(obj);
    auto m = load_obj<double>(ss);
    assert(m.vertex_count() == 3);
    assert(m.triangle_count() == 1);
    auto box = m.aabb();
    auto mn = box.min(); auto mx = box.max();
    assert(mn[0]==0 && mn[1]==0 && mn[2]==0);
    assert(mx[0]==1 && mx[1]==1 && mx[2]==0);
}

static void test_quads_and_negative_indices(){
    const char* obj = R"OBJ(
# square as quad, then the same using negative indices
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
f 1 2 3 4
f -4 -3 -2 -1
)OBJ";
    std::istringstream ss(obj);
    auto m = load_obj<double>(ss);
    assert(m.vertex_count() == 4);
    // quad triangulated => 2 tris, and another 2 from negative-indices quad => total 4
    assert(m.triangle_count() == 4);
}

int main(){
    test_load_simple_triangle();
    test_quads_and_negative_indices();
    std::cout << "All obj tests passed.\n";
    return 0;
}
