import glimmer.scene;
import glimmer.scene_object;
import glimmer.sphere;
import glimmer.plane;
import glimmer.camera;
import glimmer.transform;
import glimmer.vector;
import glimmer.color;
import glimmer.image;
import glimmer.material;
import glimmer.material_property.checkerboard;
import glimmer.renderer;
import glimmer.renderer_path_tracer;
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
    const std::size_t width = 800;
    const std::size_t height = 400;

    // Camera: look at origin from +Z
    Vector<T,3> eye{0,0,5};
    Vector<T,3> target{0,0,0};
    Vector<T,3> up{0,1,0};
    auto cam = Camera<T>::from_look_at(eye, target, up, std::numbers::pi_v<T>/3, static_cast<T>(width)/static_cast<T>(height), 0.1, 100.0);

    // Scene with a dim background to emphasize emissive lighting
    Scene<T> scene{cam, Vector<T,3>{0.1, 0.1, 0.1}};

    // Create two spheres with different materials
    auto sphereGeom = std::make_shared<Sphere<T>>(Vector<T, 3>{0, 0, 0}, 1.0);

    // Solid colored sphere (diffuse)
    auto solid_mat = Material<T>::lambertian(Vector<T, 3>{0.9, 0.2, 0.2}); // red-ish

    // Glass sphere (slightly tinted)
    auto glass_mat = Material<T>::glass(Vector<T, 3>{0.95, 0.95, 1.0}, /*roughness*/0.0, /*transparency*/0.95);

    // Place spheres to the left and right, plus a mirror sphere further back
    auto xf_left  = Transform<T>::from_trs(Vector<T,3>{-1.25, 0.0, 0.0}, glimmer::Quaternion<T>{}, Vector<T,3>{1,1,1});
    auto xf_right = Transform<T>::from_trs(Vector<T,3>{ -2.0, 0.0, 2.0}, glimmer::Quaternion<T>{}, Vector<T,3>{1,1, 1});
    auto xf_mirror = Transform<T>::from_trs(Vector<T,3>{ 0.0, 0.0, -2.0}, glimmer::Quaternion<T>{}, Vector<T,3>{1,1,1});

    // Materials
    auto mirror_mat = Material<T>::metal(Vector<T, 3>{0.95, 0.95, 0.95}, /*roughness*/0.0);

    SceneObject<T> left{sphereGeom, solid_mat, xf_left};
    SceneObject<T> right{sphereGeom, glass_mat, xf_right};
    SceneObject<T> mirror{sphereGeom, mirror_mat, xf_mirror};

    // Add an infinite checkerboard plane at y = -1 facing up
    using glimmer::Plane;
    auto planeGeom = std::make_shared<Plane<T>>(Vector<T, 3>{0, -1, 0}, Vector<T, 3>{0, 1, 0});

    // Build a checkerboard property for the plane's albedo
    Vector<T, 3> white{1, 1, 1};
    Vector<T, 3> black{0.05, 0.05, 0.05};
    auto checker = std::make_shared<glimmer::CheckerboardMaterialProperty<T, 3>>(
        white, black, /*tiles_u*/6, /*tiles_v*/6);

    Material<T> plane_mat = Material<T>::lambertian(white);
    plane_mat.set_albedo_property(checker);

    glimmer::Transform<T> xf_plane; // identity
    SceneObject<T> ground{planeGeom, plane_mat, xf_plane};

    // Emissive light sphere far away to create shadows
    Vector<T,3> light_radiance{8.0, 8.0, 8.0};
    auto light_mat = Material<T>::emissive(light_radiance, /*power*/30.0);
    auto xf_light = Transform<T>::from_trs(Vector<T,3>{3.0, 5.0, -2.0}, glimmer::Quaternion<T>{}, Vector<T,3>{0.5, 0.5, 0.5});
    SceneObject<T> light{sphereGeom, light_mat, xf_light};

    // Add objects to the scene
    scene.add_object(left);
    scene.add_object(right);
    scene.add_object(mirror);
    scene.add_object(ground);
    scene.add_object(light);

    // Render
    Image<T,3> img{width, height};
    glimmer::RendererPathTracer<T> renderer;
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
