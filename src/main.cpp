import glimmer.scene;
import glimmer.scene_object;
import glimmer.sphere;
import glimmer.camera;
import glimmer.transform;
import glimmer.vector;
import glimmer.color;
import glimmer.image;
import glimmer.material;
import glimmer.renderer;
import glimmer.renderer_simple_rt;
import glimmer.ppm;
import glimmer.quaternion;
#include <iostream>
#include <memory>
#include <numbers>

int main() {
    using T = double;
    using glimmer::Vector;
    using glimmer::Scene;
    using glimmer::SceneObject;
    using glimmer::Sphere;
    using glimmer::Camera;
    using glimmer::Transform;
    using glimmer::Image;
    using glimmer::Material;

    // Image settings
    const std::size_t width = 1920;
    const std::size_t height = 1080;

    // Camera: look at origin from +Z
    Vector<T,3> eye{0,0,5};
    Vector<T,3> target{0,0,0};
    Vector<T,3> up{0,1,0};
    auto cam = Camera<T>::from_look_at(eye, target, up, std::numbers::pi_v<T>/3, static_cast<T>(width)/static_cast<T>(height), 0.1, 100.0);

    // Scene with a subtle sky-blue background
    Scene<T> scene{cam, Vector<T,3>{0.1, 0.2, 0.4}};

    // Create two spheres with different materials
    auto geom = std::make_shared<Sphere<T>>(Vector<T,3>{0,0,0}, 1.0);
    auto red_diffuse = Material<T>::lambertian(Vector<T,3>{0.9, 0.1, 0.1});
    auto green_metal = Material<T>::metal(Vector<T,3>{0.1, 0.9, 0.1}, 0.1);

    // Place spheres to the left and right
    auto xf_left  = Transform<T>::from_trs(Vector<T,3>{-1.25, 0.0, 0.0}, glimmer::Quaternion<T>{}, Vector<T,3>{1,1,1});
    auto xf_right = Transform<T>::from_trs(Vector<T,3>{ 1.25, 0.0, 0.0}, glimmer::Quaternion<T>{}, Vector<T,3>{1,1,1});

    SceneObject<T> left{geom, red_diffuse, xf_left};
    SceneObject<T> right{geom, green_metal, xf_right};

    scene.add_object(left);
    scene.add_object(right);

    // Render
    Image<T,3> img{width, height};
    glimmer::RendererSimpleRT<T> renderer;
    renderer.render(scene, img, width, height);

    // Save to PPM
    const char* out_path = "render.ppm";
    if (glimmer::save_ppm(img, out_path)) {
        std::cout << "Wrote PPM image to " << out_path << " (" << width << "x" << height << ")\n";
    } else {
        std::cerr << "Failed to write PPM image to " << out_path << "\n";
        return 1;
    }

    return 0;
}
