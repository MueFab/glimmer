module;
#include <cmath>
#include <cstddef>
#include <ostream>
#include <stdexcept>

/**
 * @file
 * @brief C++23 module providing affine transform utilities for 3D rendering.
 */
export module glimmer.transform;

import glimmer.vector;
import glimmer.matrix;
import glimmer.quaternion;

namespace glimmer
{
    /**
     * @brief Rigid/affine transform with cached inverse.
     * @tparam T arithmetic scalar type
     * @details Stores a 4x4 matrix and its inverse. Provides composition and application to points/dirs.
     */
    export template <Arithmetic T>
    class Transform
    {
    public:
        using Mat4 = Matrix<T, 4, 4>;

        /** @brief Constructs identity transform. */
        constexpr Transform() noexcept : m_{identity4()}, inv_{identity4()} {}

        /**
         * @brief Constructs from matrix and (optionally) its inverse.
         * @param m forward 4x4 matrix
         * @param inv inverse 4x4 matrix (if not provided, computed)
         */
        explicit Transform(const Mat4& m) : m_{m}, inv_{m.inverse()} {}
        Transform(const Mat4& m, const Mat4& inv) : m_{m}, inv_{inv} {}

        /** @brief Returns forward matrix. */
        [[nodiscard]] constexpr const Mat4& matrix() const noexcept { return m_; }
        /** @brief Returns inverse matrix. */
        [[nodiscard]] constexpr const Mat4& inverse_matrix() const noexcept { return inv_; }

        /** @brief Returns the inverse transform (swapping matrices). */
        [[nodiscard]] constexpr Transform inverse() const noexcept { return Transform{inv_, m_}; }

        /** @brief Composition: this followed by other. */
        [[nodiscard]] constexpr Transform operator*(const Transform& rhs) const noexcept
        {
            return Transform{m_ * rhs.m_, rhs.inv_ * inv_};
        }

        /** @brief Applies to a 3D point (homogeneous with w=1). */
        [[nodiscard]] constexpr Vector<T, 3> apply_point(const Vector<T, 3>& p) const noexcept
        {
            const auto& M = m_;
            Vector<T, 3> r{};
            r[0] = M(0,0) * p[0] + M(0,1) * p[1] + M(0,2) * p[2] + M(0,3);
            r[1] = M(1,0) * p[0] + M(1,1) * p[1] + M(1,2) * p[2] + M(1,3);
            r[2] = M(2,0) * p[0] + M(2,1) * p[1] + M(2,2) * p[2] + M(2,3);
            return r;
        }

        /** @brief Applies to a 3D direction (homogeneous with w=0). */
        [[nodiscard]] constexpr Vector<T, 3> apply_direction(const Vector<T, 3>& v) const noexcept
        {
            const auto& M = m_;
            Vector<T, 3> r{};
            r[0] = M(0,0) * v[0] + M(0,1) * v[1] + M(0,2) * v[2];
            r[1] = M(1,0) * v[0] + M(1,1) * v[1] + M(1,2) * v[2];
            r[2] = M(2,0) * v[0] + M(2,1) * v[1] + M(2,2) * v[2];
            return r;
        }

        /**
         * @brief Transforms a normal using the inverse-transpose 3x3 of the transform.
         * @details Appropriate for non-uniform scaling.
         */
        [[nodiscard]] Vector<T, 3> apply_normal(const Vector<T, 3>& n) const
        {
            // Use the 3x3 of inverse^T
            Matrix<T,3,3> A{
                inv_(0,0), inv_(1,0), inv_(2,0),
                inv_(0,1), inv_(1,1), inv_(2,1),
                inv_(0,2), inv_(1,2), inv_(2,2)
            };
            return A * n;
        }

        /** @brief Factory: identity transform. */
        [[nodiscard]] static constexpr Transform identity() noexcept { return Transform{}; }

        /**
         * @brief Builds a transform from translation, rotation (quaternion), and scale.
         * @param t translation vector
         * @param r rotation quaternion
         * @param s scale vector (non-uniform supported)
         */
        [[nodiscard]] static Transform from_trs(const Vector<T,3>& t, const Quaternion<T>& r, const Vector<T,3>& s)
        {
            // Rotation matrix
            Matrix<T,3,3> R = r.to_matrix3();
            // Rotate then scale: M3 = S * R (scale in rotated/local axes)
            Matrix<T,3,3> SR{
                s[0]*R(0,0), s[0]*R(0,1), s[0]*R(0,2),
                s[1]*R(1,0), s[1]*R(1,1), s[1]*R(1,2),
                s[2]*R(2,0), s[2]*R(2,1), s[2]*R(2,2)
            };
            Mat4 M{};
            // upper-left 3x3
            for (std::size_t r0=0;r0<3;++r0)
                for (std::size_t c0=0;c0<3;++c0)
                    M(r0,c0) = SR(r0,c0);
            // translation
            M(0,3) = t[0]; M(1,3) = t[1]; M(2,3) = t[2];
            // last row/col
            M(3,0)=T{0}; M(3,1)=T{0}; M(3,2)=T{0}; M(3,3)=T{1};
            // Inverse: inverse of TRS is S^-1 R^T T^-1
            Vector<T,3> invS{ T{1}/s[0], T{1}/s[1], T{1}/s[2] };
            Matrix<T,3,3> Rt{ R.transposed() };
            Matrix<T,3,3> RtSinv{
                Rt(0,0)*invS[0], Rt(0,1)*invS[1], Rt(0,2)*invS[2],
                Rt(1,0)*invS[0], Rt(1,1)*invS[1], Rt(1,2)*invS[2],
                Rt(2,0)*invS[0], Rt(2,1)*invS[1], Rt(2,2)*invS[2]
            };
            Mat4 Minv{};
            for (std::size_t r0=0;r0<3;++r0)
                for (std::size_t c0=0;c0<3;++c0)
                    Minv(r0,c0) = RtSinv(r0,c0);
            // translation inverse: -R^T S^{-1} t
            Vector<T,3> tinv = Vector<T,3>{
                -(RtSinv(0,0)*t[0] + RtSinv(0,1)*t[1] + RtSinv(0,2)*t[2]),
                -(RtSinv(1,0)*t[0] + RtSinv(1,1)*t[1] + RtSinv(1,2)*t[2]),
                -(RtSinv(2,0)*t[0] + RtSinv(2,1)*t[1] + RtSinv(2,2)*t[2])
            };
            Minv(0,3)=tinv[0]; Minv(1,3)=tinv[1]; Minv(2,3)=tinv[2];
            Minv(3,0)=T{0}; Minv(3,1)=T{0}; Minv(3,2)=T{0}; Minv(3,3)=T{1};
            return Transform{M, Minv};
        }

        /** @brief Builds a transform from a matrix; checks invertibility. */
        [[nodiscard]] static Transform from_matrix_checked(const Mat4& m)
        {
            auto inv = m.inverse();
            return Transform{m, inv};
        }

        /** @brief Creates a right-handed look-at view transform (camera to world inverse). */
        [[nodiscard]] static Transform look_at(const Vector<T,3>& eye, const Vector<T,3>& target, const Vector<T,3>& up)
        {
            Vector<T,3> f = (target - eye).normalized();
            Vector<T,3> s = cross(f, up).normalized();
            Vector<T,3> u = cross(s, f);
            // View matrix (world to camera), right-handed, camera looks along -Z
            Mat4 V{};
            V(0,0) = s[0]; V(0,1) = s[1]; V(0,2) = s[2]; V(0,3) = -dot(s, eye);
            V(1,0) = u[0]; V(1,1) = u[1]; V(1,2) = u[2]; V(1,3) = -dot(u, eye);
            V(2,0) = -f[0]; V(2,1) = -f[1]; V(2,2) = -f[2]; V(2,3) = dot(f, eye);
            V(3,0) = T{0}; V(3,1) = T{0}; V(3,2) = T{0}; V(3,3) = T{1};
            // Its inverse is camera to world
            auto Vinv = V.inverse();
            return Transform{Vinv, V}; // store as world-from-camera for typical 'apply_point' from local to world
        }

        /**
         * @brief Perspective projection matrix (right-handed, depth in [-1,1] like OpenGL by default).
         * @param fov_y vertical field of view in radians
         * @param aspect width/height
         * @param z_near near plane (>0)
         * @param z_far far plane (> z_near)
         */
        [[nodiscard]] static Matrix<T,4,4> perspective(T fov_y, T aspect, T z_near, T z_far)
        {
            if (aspect <= T{0}) throw std::invalid_argument("aspect must be > 0");
            if (!(z_near > T{0}) || !(z_far > z_near)) throw std::invalid_argument("invalid near/far");
            T f = T{1} / static_cast<T>(std::tan(static_cast<long double>(fov_y) / 2.0L));
            Matrix<T,4,4> M{};
            M(0,0) = f / aspect;
            M(1,1) = f;
            M(2,2) = (z_far + z_near) / (z_near - z_far);
            M(2,3) = (T{2} * z_far * z_near) / (z_near - z_far);
            M(3,2) = -T{1};
            M(3,3) = T{0};
            return M;
        }

        /**
         * @brief Orthographic projection matrix (right-handed, OpenGL-style clip range [-1,1]).
         */
        [[nodiscard]] static Matrix<T,4,4> orthographic(T left, T right, T bottom, T top, T z_near, T z_far)
        {
            Matrix<T,4,4> M{};
            M(0,0) = T{2} / (right - left);
            M(1,1) = T{2} / (top - bottom);
            M(2,2) = T{-2} / (z_far - z_near);
            M(0,3) = -(right + left) / (right - left);
            M(1,3) = -(top + bottom) / (top - bottom);
            M(2,3) = -(z_far + z_near) / (z_far - z_near);
            M(3,3) = T{1};
            return M;
        }

    private:
        static constexpr Mat4 identity4() noexcept
        {
            Mat4 I{}; I(0,0)=T{1}; I(1,1)=T{1}; I(2,2)=T{1}; I(3,3)=T{1}; return I;
        }

        Mat4 m_{};
        Mat4 inv_{};
    };

    /** @brief Applies a transform to a point. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Vector<T,3> transform_point(const Transform<T>& tr, const Vector<T,3>& p) noexcept
    { return tr.apply_point(p); }

    /** @brief Applies a transform to a direction. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Vector<T,3> transform_direction(const Transform<T>& tr, const Vector<T,3>& v) noexcept
    { return tr.apply_direction(v); }

    /** @brief Applies a transform to a normal vector using inverse-transpose. */
    export template <Arithmetic T>
    [[nodiscard]] inline Vector<T,3> transform_normal(const Transform<T>& tr, const Vector<T,3>& n)
    { return tr.apply_normal(n); }
}
