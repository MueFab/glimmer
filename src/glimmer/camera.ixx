module;
#include <cmath>
#include <cstddef>
#include <stdexcept>

export module glimmer.camera;

import glimmer.vector;
import glimmer.matrix;
import glimmer.transform;
import glimmer.ray;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module defining a simple pinhole camera with ray generation.
     */

    /**
     * @brief Pinhole camera with perspective/orthographic helpers and ray generation.
     * @tparam T arithmetic scalar type (float/double recommended)
     * @details Stores a camera-to-world transform and perspective parameters. Provides
     * methods to obtain view/projection matrices and to generate primary rays for
     * pixel coordinates.
     */
    export template <Arithmetic T>
    class Camera {
    public:
        using Vec3 = Vector<T,3>;
        using Mat4 = Matrix<T,4,4>;
        using Xform = Transform<T>;

        /** @brief Construct a camera from camera-to-world transform and perspective params. */
        Camera(const Xform& cam_to_world, T fov_y_radians, T aspect, T z_near, T z_far)
            : c2w_{cam_to_world}, fov_y_{fov_y_radians}, aspect_{aspect}, z_near_{z_near}, z_far_{z_far}
        {
            if (!(aspect_ > T{0})) throw std::invalid_argument("Camera: aspect must be > 0");
            if (!(z_near_ > T{0}) || !(z_far_ > z_near_)) throw std::invalid_argument("Camera: invalid near/far");
        }

        /**
         * @brief Factory: constructs from look-at parameters and perspective settings.
         * @param eye camera position in world space
         * @param target point the camera looks at
         * @param up approximate up direction
         * @param fov_y vertical field of view (radians)
         * @param aspect width/height
         * @param z_near near plane (>0)
         * @param z_far far plane (> z_near)
         */
        [[nodiscard]] static Camera from_look_at(const Vec3& eye, const Vec3& target, const Vec3& up,
                                                 T fov_y, T aspect, T z_near, T z_far)
        {
            auto cam_to_world = Xform::look_at(eye, target, up);
            return Camera{cam_to_world, fov_y, aspect, z_near, z_far};
        }

        /** @brief Camera-to-world transform. */
        [[nodiscard]] const Xform& cam_to_world() const noexcept { return c2w_; }
        /** @brief Vertical field of view in radians. */
        [[nodiscard]] T fov_y() const noexcept { return fov_y_; }
        /** @brief Aspect ratio (width/height). */
        [[nodiscard]] T aspect() const noexcept { return aspect_; }
        /** @brief Near plane distance. */
        [[nodiscard]] T z_near() const noexcept { return z_near_; }
        /** @brief Far plane distance. */
        [[nodiscard]] T z_far() const noexcept { return z_far_; }

        /** @brief Returns the world-to-camera view matrix. */
        [[nodiscard]] Mat4 view_matrix() const noexcept { return c2w_.inverse_matrix(); }
        /** @brief Returns the perspective projection matrix (OpenGL style). */
        [[nodiscard]] Mat4 proj_matrix() const { return Xform::perspective(fov_y_, aspect_, z_near_, z_far_); }
        /** @brief Returns the view-projection matrix (world to clip). */
        [[nodiscard]] Mat4 viewproj_matrix() const { return proj_matrix() * view_matrix(); }

        /**
         * @brief Generates a primary ray through pixel center (px,py) in image space.
         * @param px pixel x in [0, width)
         * @param py pixel y in [0, height)
         * @param width image width (pixels)
         * @param height image height (pixels)
         * @return world-space ray originating at the camera position going through the pixel center.
         * @details Right-handed convention: camera looks along -Z in its local space. The image plane is
         * at z = -1 with vertical extent determined by tan(fov_y/2), and horizontal extent by aspect.
         * Pixel centers are mapped to NDC via (x+0.5)/width and (y+0.5)/height with origin at top-left.
         */
        [[nodiscard]] Ray<T> generate_ray(T px, T py, std::size_t width, std::size_t height) const noexcept {
            // Map pixel center to NDC [0,1]
            const T nx = (px + T{0.5}) / static_cast<T>(width);
            const T ny = (py + T{0.5}) / static_cast<T>(height);
            // Map to screen space [-1,1], flip Y so that py=0 is top
            const T sx = nx * T{2} - T{1};
            const T sy = T{1} - ny * T{2};
            // Compute direction in camera space
            const T tan_half = static_cast<T>(std::tan(static_cast<long double>(fov_y_) / 2.0L));
            const T x_cam = sx * tan_half * aspect_;
            const T y_cam = sy * tan_half;
            Vec3 dir_cam{ x_cam, y_cam, static_cast<T>(-1) };
            // Normalize direction
            dir_cam = dir_cam.normalized();
            // Transform to world
            const Vec3 o_world = c2w_.apply_point(Vec3{T{0},T{0},T{0}});
            const Vec3 d_world = c2w_.apply_direction(dir_cam).normalized();
            return Ray<T>{ o_world, d_world, z_near_, z_far_ };
        }

    private:
        Xform c2w_{}; // camera to world
        T fov_y_{};
        T aspect_{};
        T z_near_{};
        T z_far_{};
    };
}
