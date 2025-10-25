cmake_minimum_required(VERSION 4.0)

# Try to locate clang-scan-deps early so CMake wires it into the toolchain
find_program(CLANG_SCAN_DEPS_EXECUTABLE
    NAMES clang-scan-deps clang-scan-deps-19 clang-scan-deps-18 clang-scan-deps-17
    HINTS /usr/lib/llvm-19/bin /usr/lib/llvm-18/bin /usr/lib/llvm-17/bin
)
if(CLANG_SCAN_DEPS_EXECUTABLE)
    set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${CLANG_SCAN_DEPS_EXECUTABLE}" CACHE FILEPATH "Path to clang-scan-deps" FORCE)
endif()

project(glimmer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable module scanning for compilers requiring it
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

include(CTest)

# Library exposing the C++23 module
add_library(glimmer_vector)
set_target_properties(glimmer_vector PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_sources(glimmer_vector
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/glimmer/vector.ixx
            src/glimmer/matrix.ixx
            src/glimmer/quaternion.ixx
            src/glimmer/transform.ixx
            src/glimmer/ray.ixx
            src/glimmer/sphere.ixx
            src/glimmer/mesh.ixx
            src/glimmer/color.ixx
            src/glimmer/image.ixx
            src/glimmer/aabb.ixx
            src/glimmer/material.ixx
            src/glimmer/geometry.ixx
            src/glimmer/scene_object.ixx
            src/glimmer/camera.ixx
            src/glimmer/ppm.ixx
            src/glimmer/scene.ixx
            src/glimmer/renderer.ixx
            src/glimmer/renderer_simple_rt.ixx
            src/glimmer/obj.ixx
)

# AABB tests
add_executable(aabb_tests
    src/tests/aabb_tests.cpp
)
set_target_properties(aabb_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(aabb_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME aabb_tests COMMAND aabb_tests)

# Main executable
add_executable(glimmer
    src/main.cpp
)
set_target_properties(glimmer PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(glimmer PRIVATE glimmer_vector stdc++ m pthread)

# Tests
add_executable(vector_tests
    src/tests/vector_tests.cpp
)
set_target_properties(vector_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(vector_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME vector_tests COMMAND vector_tests)

add_executable(matrix_tests
    src/tests/matrix_tests.cpp
)
set_target_properties(matrix_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(matrix_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME matrix_tests COMMAND matrix_tests)

add_executable(quaternion_tests
    src/tests/quaternion_tests.cpp
)
set_target_properties(quaternion_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(quaternion_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME quaternion_tests COMMAND quaternion_tests)

add_executable(transform_tests
    src/tests/transform_tests.cpp
)
set_target_properties(transform_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(transform_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME transform_tests COMMAND transform_tests)

# Ray tests
add_executable(ray_tests
    src/tests/ray_tests.cpp
)
set_target_properties(ray_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(ray_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME ray_tests COMMAND ray_tests)

# Sphere tests
add_executable(sphere_tests
    src/tests/sphere_tests.cpp
)
set_target_properties(sphere_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(sphere_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME sphere_tests COMMAND sphere_tests)

# Mesh tests
add_executable(mesh_tests
    src/tests/mesh_tests.cpp
)
set_target_properties(mesh_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(mesh_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME mesh_tests COMMAND mesh_tests)

# OBJ tests
add_executable(obj_tests
    src/tests/obj_tests.cpp
)
set_target_properties(obj_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(obj_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME obj_tests COMMAND obj_tests)

# Color tests
add_executable(color_tests
    src/tests/color_tests.cpp
)
set_target_properties(color_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(color_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME color_tests COMMAND color_tests)

# Image tests
add_executable(image_tests
    src/tests/image_tests.cpp
)
set_target_properties(image_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(image_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME image_tests COMMAND image_tests)

# Material tests
add_executable(material_tests
    src/tests/material_tests.cpp
)
set_target_properties(material_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(material_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME material_tests COMMAND material_tests)

# SceneObject tests
add_executable(sceneobject_tests
    src/tests/sceneobject_tests.cpp
)
set_target_properties(sceneobject_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(sceneobject_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME sceneobject_tests COMMAND sceneobject_tests)

# Camera tests
add_executable(camera_tests
    src/tests/camera_tests.cpp
)
set_target_properties(camera_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(camera_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME camera_tests COMMAND camera_tests)

# PPM tests
add_executable(ppm_tests
    src/tests/ppm_tests.cpp
)
set_target_properties(ppm_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(ppm_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME ppm_tests COMMAND ppm_tests)

# Scene tests
add_executable(scene_tests
    src/tests/scene_tests.cpp
)
set_target_properties(scene_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(scene_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME scene_tests COMMAND scene_tests)

# Renderer tests
add_executable(renderer_tests
    src/tests/renderer_tests.cpp
)
set_target_properties(renderer_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_link_libraries(renderer_tests PRIVATE glimmer_vector stdc++ m pthread)

add_test(NAME renderer_tests COMMAND renderer_tests)


