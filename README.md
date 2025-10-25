# Glimmer

A tiny C++23 math and rendering playground using C++ named modules. It contains a compact but capable math core (vectors, matrices, quaternions, transforms), basic geometry (rays, spheres, triangle meshes, AABBs), materials, camera, images, and a simple multi‑threaded ray‑tracing renderer. The sample app renders a small scene to a PPM file.

## Features
- C++23 named modules throughout (no headers)
- Math
  - glimmer.vector: fixed‑size Vector<T,N> with dot/cross, norms, lerp, dimension resize, homogeneous helpers
  - glimmer.matrix: Matrix<T,R,C> with mul, transpose, determinant, inverse (generic NxN)
  - glimmer.quaternion: rotations, slerp, matrix conversion
  - glimmer.transform: TRS, look_at, perspective/orthographic
- Geometry
  - glimmer.ray
  - glimmer.sphere (ray intersection, AABB)
  - glimmer.mesh (triangle list, Möller–Trumbore, AABB)
  - glimmer.aabb (slabs ray intersection)
  - glimmer.geometry (abstract base interface)
  - glimmer.scene_object (geometry + material + transform, cached matrices and AABB)
  - glimmer.scene (container with background and camera)
- Rendering
  - glimmer.renderer (interface)
  - glimmer.renderer_simple_rt (simple single‑bounce renderer; multi‑threaded)
- Imaging & I/O
  - glimmer.image (format‑agnostic 2D image)
  - glimmer.color (color utils and aliases)
  - glimmer.ppm (P6 read/write of RGB images)
- Tests: assert‑based unit tests integrated with CTest for each module

## Build
This project uses CMake and requires a modern compiler with C++23 modules support (Clang is recommended). CMake is configured to scan modules automatically and will use `clang-scan-deps` when available.

Prerequisites:
- CMake ≥ 3.28 (module scanning)
- Clang ≥ 17 (named modules) and `clang-scan-deps`

Typical steps:
1) Configure your CMake profile/toolchain to use Clang.
2) Build targets (examples):
   - Main app: `glimmer`
   - Library with modules: `glimmer_vector`
   - Tests: run via CTest

If you are using an IDE (e.g., CLion), simply build the desired targets. For command‑line CMake, a typical sequence is:

```
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build --target glimmer
```

## Run the demo
The `glimmer` executable renders a minimal scene (two spheres) to `render.ppm` in the project root. After building the `glimmer` target, run the produced executable; you should see output confirming that the image was written.

## Tests
All modules have small unit tests registered with CTest.

From your build directory:
```
ctest -V
```

Targets include:
- vector_tests, matrix_tests, quaternion_tests, transform_tests
- ray_tests, sphere_tests, mesh_tests, aabb_tests
- color_tests, image_tests, ppm_tests
- sceneobject_tests, camera_tests, scene_tests
- renderer_tests

## Repository layout
- src/glimmer/*.ixx — C++23 module interfaces
- src/tests/*.cpp — simple assert‑based tests per module
- src/main.cpp — sample app to render a scene
- CMakeLists.txt — build configuration with module scanning and CTest

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE).

## Acknowledgements
- Built as a compact educational/reference project to explore C++23 modules in a rendering context.
