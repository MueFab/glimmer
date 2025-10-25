import glimmer.renderer;
import glimmer.renderer_simple_rt;
import glimmer.scene;
import glimmer.scene_object;
import glimmer.sphere;
import glimmer.camera;
import glimmer.transform;
import glimmer.vector;
import glimmer.color;
import glimmer.image;
import glimmer.material;
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

using glimmer::Scene;
using glimmer::SceneObject;
using glimmer::Sphere;
using glimmer::Camera;
using glimmer::Vector;
using glimmer::Color;
using glimmer::Image;
using glimmer::Material;

static void test_render_emissive_center() {
    using T = double;
    // Camera looking at origin from +Z
    Vector<T,3> eye{0,0,5};
    Vector<T,3> target{0,0,0};
    Vector<T,3> up{0,1,0};
    auto cam = Camera<T>::from_look_at(eye, target, up, M_PI/3, 1.0, 0.1, 100.0);
    Scene<T> scene{cam, Color<T,3>{0,0,0}}; // black background

    // Emissive red sphere at origin
    auto geom = std::make_shared<Sphere<T>>(Vector<T,3>{0,0,0}, 1.0);
    auto mat = Material<T>::emissive(Color<T,3>{1.0, 0.0, 0.0}, 2.0); // power 2
    SceneObject<T> obj{geom, mat, glimmer::Transform<T>{}}; // identity transform
    scene.add_object(obj);

    const std::size_t W = 9, H = 9; // odd sizes so there is a true center pixel (4,4)
    Image<T,3> img{W,H};
    glimmer::RendererSimpleRT<T> renderer;
    renderer.render(scene, img, W, H);

    // Center pixel should see the sphere and be bright red (close to radiance*power)
    auto c = img(W/2, H/2);
    assert(c[0] > 0.9 && c[1] < 1e-6 && c[2] < 1e-6);
    // Corner pixel should see background (black)
    auto k = img(0,0);
    assert(std::abs(k[0]) < 1e-12 && std::abs(k[1]) < 1e-12 && std::abs(k[2]) < 1e-12);
}

static void test_render_diffuse_lambert() {
    using T = double;
    // Camera
    Vector<T,3> eye{0,0,5};
    Vector<T,3> target{0,0,0};
    Vector<T,3> up{0,1,0};
    auto cam = Camera<T>::from_look_at(eye, target, up, M_PI/3, 1.0, 0.1, 100.0);
    Scene<T> scene{cam, Color<T,3>{0,0,0}};

    // Diffuse gray sphere at origin
    auto geom = std::make_shared<Sphere<T>>(Vector<T,3>{0,0,0}, 1.0);
    auto mat = Material<T>::lambertian(Color<T,3>{0.5, 0.5, 0.5});
    SceneObject<T> obj{geom, mat, glimmer::Transform<T>{}};
    scene.add_object(obj);

    const std::size_t W = 9, H = 9;
    Image<T,3> img{W,H};
    glimmer::RendererSimpleRT<T> renderer;
    renderer.render(scene, img, W, H);

    // Center pixel should be non-black due to Lambertian term
    auto c = img(W/2, H/2);
    assert(c[0] > 0.0 && c[1] > 0.0 && c[2] > 0.0);
}

int main() {
    test_render_emissive_center();
    test_render_diffuse_lambert();
    std::cout << "All renderer tests passed.\n";
    return 0;
}
