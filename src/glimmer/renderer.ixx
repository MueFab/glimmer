module;
#include <cstddef>

export module glimmer.renderer;

import glimmer.vector;
import glimmer.color;
import glimmer.ray;
import glimmer.scene;
import glimmer.image;

namespace glimmer {
    /**
     * @file
     * @brief Renderer interface for pluggable rendering backends.
     */

    /**
     * @brief Abstract renderer interface.
     * @tparam T arithmetic scalar type (float/double recommended)
     * @details Defines the minimum API for rendering a Scene to an Image and tracing a ray.
     */
    export template <Arithmetic T>
    class Renderer {
    public:
        using Color3 = Color<T,3>;
        using Vec3 = Vector<T,3>;

        virtual ~Renderer() = default;

        /** @brief Evaluates radiance for a primary ray in the given scene. */
        [[nodiscard]] virtual Color3 trace_ray(const Scene<T>& scene, const Ray<T>& ray) const noexcept = 0;
        /** @brief Renders the scene to the given RGB image (resizes if needed). */
        virtual void render(const Scene<T>& scene, Image<T,3>& out, std::size_t width, std::size_t height) const = 0;
    };
}
