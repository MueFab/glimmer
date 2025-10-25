import glimmer.image;
import glimmer.color;
import glimmer.vector;
import glimmer.ppm;
#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>

using glimmer::Image;
using glimmer::Color3f;
using glimmer::Vector;

static void test_construct_and_access() {
    Image<float,3> img{4,3};
    assert(img.width() == 4 && img.height() == 3);
    // default init to zeros
    for (std::size_t y=0; y<img.height(); ++y)
        for (std::size_t x=0; x<img.width(); ++x) {
            auto p = img(x,y);
            assert(p[0] == 0.0f && p[1] == 0.0f && p[2] == 0.0f);
        }
    // set/get
    img(2,1) = Color3f{1.0f, 0.5f, 0.25f};
    auto q = img.at(2,1);
    assert(std::abs(q[0]-1.0f) < 1e-6f);
    assert(std::abs(q[1]-0.5f) < 1e-6f);
    assert(std::abs(q[2]-0.25f) < 1e-6f);
    bool threw=false; try { (void)img.at(9,9); } catch(...) { threw=true; }
    assert(threw);
}

static void test_clear_and_resize() {
    Image<float,3> img{2,2};
    img.clear(Color3f{0.2f, 0.3f, 0.4f});
    for (std::size_t y=0; y<img.height(); ++y)
        for (std::size_t x=0; x<img.width(); ++x) {
            auto p = img(x,y);
            assert(std::abs(p[0]-0.2f) < 1e-6f);
            assert(std::abs(p[1]-0.3f) < 1e-6f);
            assert(std::abs(p[2]-0.4f) < 1e-6f);
        }
    img.resize(3,1, Color3f{1,0,0});
    assert(img.width()==3 && img.height()==1);
    for (std::size_t x=0; x<3; ++x) {
        auto p = img(x,0);
        assert(std::abs(p[0]-1.0f) < 1e-6f && std::abs(p[1]-0.0f) < 1e-6f && std::abs(p[2]-0.0f) < 1e-6f);
    }
}

static void test_write_ppm() {
    Image<float,3> img{2,2};
    img(0,0) = Color3f{1,0,0};
    img(1,0) = Color3f{0,1,0};
    img(0,1) = Color3f{0,0,1};
    img(1,1) = Color3f{1,1,1};
    const char* fname = "image_test.ppm";
    bool ok = glimmer::save_ppm(img, fname);
    assert(ok);
    namespace fs = std::filesystem;
    assert(fs::exists(fname));
    assert(fs::file_size(fname) > 0);
    std::ifstream f(fname, std::ios::binary);
    std::string header;
    std::getline(f, header);
    assert(header == std::string("P6"));
}

int main(){
    test_construct_and_access();
    test_clear_and_resize();
    test_write_ppm();
    std::cout << "All image tests passed.\n";
    return 0;
}
