module;
#include <vector>
#include <cstddef>

export module glimmer.scene;

import glimmer.vector;
import glimmer.color;
import glimmer.aabb;
import glimmer.scene_object;
import glimmer.camera;

namespace glimmer {
    /**
     * @file
     * @brief C++23 module defining a simple Scene container with objects, background color, and a camera.
     */

    /**
     * @brief Simple scene consisting of a list of objects, a background color, and a camera.
     * @tparam T arithmetic scalar type (float/double recommended)
     * @details The scene owns a list of SceneObject<T> by value, a background color (RGB, linear), and a Camera<T>.
     * It provides convenience methods to add/clear objects, query counts, access the camera and background, and
     * compute the world-space union AABB of all contained objects.
     */
    export template <Arithmetic T>
    class Scene {
    public:
        using Vec3 = Vector<T,3>;
        using Color3 = Color<T,3>;

        /** @brief Constructs an empty scene with black background and identity camera. */
        Scene() : bg_{T{0},T{0},T{0}}, cam_{Camera<T>::from_look_at(Vec3{0,0,0}, Vec3{0,0,-1}, Vec3{0,1,0},
                                                                    static_cast<T>(60.0 * 3.14159265358979323846 / 180.0),
                                                                    T{1}, T{0.1}, T{1000})} {}

        /** @brief Constructs a scene with given camera and background color. */
        Scene(const Camera<T>& camera, const Color3& background) : bg_{background}, cam_{camera} {}

        /** @brief Background color (RGB, linear). */
        [[nodiscard]] const Color3& background() const noexcept { return bg_; }
        /** @brief Mutable background color setter. */
        void set_background(const Color3& c) noexcept { bg_ = c; }

        /** @brief Access camera. */
        [[nodiscard]] const Camera<T>& camera() const noexcept { return cam_; }
        /** @brief Set camera. */
        void set_camera(const Camera<T>& c) noexcept { cam_ = c; }

        /** @brief Number of objects in the scene. */
        [[nodiscard]] std::size_t size() const noexcept { return objects_.size(); }
        /** @brief Returns true if there are no objects. */
        [[nodiscard]] bool empty() const noexcept { return objects_.empty(); }

        /** @brief Removes all objects. */
        void clear() { objects_.clear(); }

        /** @brief Adds an object by value. */
        void add_object(const SceneObject<T>& obj) { objects_.push_back(obj); }
        /** @brief Adds an object by moving. */
        void add_object(SceneObject<T>&& obj) { objects_.push_back(std::move(obj)); }

        /** @brief Access list of objects. */
        [[nodiscard]] const std::vector<SceneObject<T>>& objects() const noexcept { return objects_; }
        /** @brief Mutable access to list of objects. */
        [[nodiscard]] std::vector<SceneObject<T>>& objects() noexcept { return objects_; }

        /** @brief Computes the union AABB of all objects in world space. Returns empty if no objects. */
        [[nodiscard]] AABB<T> aabb() const noexcept {
            AABB<T> box; // empty
            for (const auto& o : objects_) box.expand(o.aabb());
            return box;
        }

    private:
        std::vector<SceneObject<T>> objects_{};
        Color3 bg_{};
        Camera<T> cam_;
    };
}
