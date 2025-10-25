module;
#include <array>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <type_traits>

/**
 * @file
 * @brief C++23 module providing glimmer::Quaternion and related operations.
 *
 * This module defines a templated Quaternion suitable for representing 3D rotations
 * and performing common quaternion algebra. Conversions to rotation matrices and
 * vector rotation helpers are provided for integration with the rest of the math stack.
 */
export module glimmer.quaternion;

import glimmer.vector;
import glimmer.matrix;

namespace glimmer
{
    /**
     * @brief Quaternion representing rotations in 3D (or general quaternion algebra).
     * @tparam T Arithmetic scalar type (float, double, etc.).
     * @details Stored as (w, x, y, z) where w is the scalar part and (x,y,z) is the vector part.
     * Identity rotation is (1, 0, 0, 0).
     */
    export template <Arithmetic T>
    class Quaternion
    {
    public:
        using value_type = T;

        /** @brief Constructs the identity quaternion (1,0,0,0). */
        constexpr Quaternion() noexcept : w_{T{1}}, x_{T{0}}, y_{T{0}}, z_{T{0}}
        {
        }

        /**
         * @brief Constructs from components.
         * @param w scalar part
         * @param x x component of vector part
         * @param y y component of vector part
         * @param z z component of vector part
         */
        constexpr Quaternion(T w, T x, T y, T z) noexcept : w_{w}, x_{x}, y_{y}, z_{z}
        {
        }

        /**
         * @brief Constructs from scalar and vector part.
         * @param w scalar part
         * @param v vector part (x,y,z)
         */
        constexpr Quaternion(T w, const Vector<T, 3>& v) noexcept : w_{w}, x_{v[0]}, y_{v[1]}, z_{v[2]}
        {
        }

        /**
         * @brief Constructs a unit quaternion from an axis and angle in radians.
         * @param axis rotation axis (need not be normalized; zero axis yields identity)
         * @param angle radians
         */
        static constexpr Quaternion from_axis_angle(const Vector<T, 3>& axis, T angle) noexcept
        {
            T ax = axis[0], ay = axis[1], az = axis[2];
            T len = static_cast<T>(std::sqrt(
                static_cast<long double>(ax) * ax + static_cast<long double>(ay) * ay + static_cast<long double>(az) *
                az));
            if (len == T{}) return Quaternion{}; // identity for zero axis
            T half = angle * static_cast<T>(0.5);
            T s = static_cast<T>(std::sin(static_cast<long double>(half)));
            T c = static_cast<T>(std::cos(static_cast<long double>(half)));
            T nx = ax / len, ny = ay / len, nz = az / len;
            return Quaternion{c, nx * s, ny * s, nz * s};
        }

        /** @brief Access scalar part. */
        [[nodiscard]] constexpr T w() const noexcept { return w_; }
        /** @brief Access x component. */
        [[nodiscard]] constexpr T x() const noexcept { return x_; }
        /** @brief Access y component. */
        [[nodiscard]] constexpr T y() const noexcept { return y_; }
        /** @brief Access z component. */
        [[nodiscard]] constexpr T z() const noexcept { return z_; }

        /** @brief Returns vector part (x,y,z). */
        [[nodiscard]] constexpr Vector<T, 3> vec() const noexcept { return Vector<T, 3>{x_, y_, z_}; }

        /** @brief Exact equality comparison. */
        [[nodiscard]] constexpr bool operator==(const Quaternion& rhs) const noexcept
        {
            return w_ == rhs.w_ && x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_;
        }

        /** @brief Logical negation of operator==. */
        [[nodiscard]] constexpr bool operator!=(const Quaternion& rhs) const noexcept { return !(*this == rhs); }

        /** @brief Unary plus (no-op). */
        [[nodiscard]] constexpr Quaternion operator+() const noexcept { return *this; }
        /** @brief Unary minus (negates all components). */
        [[nodiscard]] constexpr Quaternion operator-() const noexcept { return Quaternion{-w_, -x_, -y_, -z_}; }

        /** @brief Adds two quaternions component-wise. */
        constexpr Quaternion& operator+=(const Quaternion& q) noexcept
        {
            w_ += q.w_;
            x_ += q.x_;
            y_ += q.y_;
            z_ += q.z_;
            return *this;
        }

        /** @brief Subtracts two quaternions component-wise. */
        constexpr Quaternion& operator-=(const Quaternion& q) noexcept
        {
            w_ -= q.w_;
            x_ -= q.x_;
            y_ -= q.y_;
            z_ -= q.z_;
            return *this;
        }

        /** @brief Scales all components by s. */
        constexpr Quaternion& operator*=(const T& s) noexcept
        {
            w_ *= s;
            x_ *= s;
            y_ *= s;
            z_ *= s;
            return *this;
        }

        /** @brief Divides all components by s. */
        Quaternion& operator/=(const T& s)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if (s == T{}) throw std::domain_error("Quaternion::operator/= divide by zero");
            }
            else { if (std::abs(s) == T{}) throw std::domain_error("Quaternion::operator/= divide by zero"); }
            w_ /= s;
            x_ /= s;
            y_ /= s;
            z_ /= s;
            return *this;
        }

        /** @brief Hamilton product (this = this * q). */
        constexpr Quaternion& hamilton_in_place(const Quaternion& q) noexcept
        {
            const T nw = w_ * q.w_ - x_ * q.x_ - y_ * q.y_ - z_ * q.z_;
            const T nx = w_ * q.x_ + x_ * q.w_ + y_ * q.z_ - z_ * q.y_;
            const T ny = w_ * q.y_ - x_ * q.z_ + y_ * q.w_ + z_ * q.x_;
            const T nz = w_ * q.z_ + x_ * q.y_ - y_ * q.x_ + z_ * q.w_;
            w_ = nw;
            x_ = nx;
            y_ = ny;
            z_ = nz;
            return *this;
        }

        /** @brief Conjugate (w, -x, -y, -z). */
        [[nodiscard]] constexpr Quaternion conjugate() const noexcept { return Quaternion{w_, -x_, -y_, -z_}; }

        /** @brief Squared norm w^2 + x^2 + y^2 + z^2. */
        [[nodiscard]] constexpr T norm_sq() const noexcept { return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_; }

        /** @brief Euclidean norm (length). */
        [[nodiscard]] T norm() const noexcept
        {
            long double s = static_cast<long double>(w_) * w_ + static_cast<long double>(x_) * x_ + static_cast<long
                double>(y_) * y_ + static_cast<long double>(z_) * z_;
            if constexpr (std::is_floating_point_v<T>) return static_cast<T>(std::sqrt(s));
            else return static_cast<T>(std::llround(std::sqrt(s)));
        }

        /** @brief Returns a normalized quaternion; identity if zero-length. */
        [[nodiscard]] Quaternion normalized() const
        {
            T n = norm();
            if (n == T{}) return Quaternion{};
            if constexpr (std::is_floating_point_v<T>) return Quaternion{w_ / n, x_ / n, y_ / n, z_ / n};
            else return Quaternion{
                static_cast<T>(w_ / n), static_cast<T>(x_ / n), static_cast<T>(y_ / n), static_cast<T>(z_ / n)
            };
        }

        /** @brief Inverse quaternion q^{-1} = conjugate(q) / ||q||^2; throws if singular. */
        [[nodiscard]] Quaternion inverse() const
        {
            T ns = norm_sq();
            if (ns == T{}) throw std::domain_error("singular quaternion");
            return Quaternion{w_ / ns, -x_ / ns, -y_ / ns, -z_ / ns};
        }

        /**
         * @brief Rotates a 3D vector by this quaternion (interpreted as rotation quaternion).
         * @param v vector to rotate
         * @return rotated vector
         */
        [[nodiscard]] Vector<T, 3> rotate(const Vector<T, 3>& v) const noexcept
        {
            // Using optimized formula: v' = v + 2*q_vec x (q_vec x v + w * v)
            Vector<T, 3> qv{x_, y_, z_};
            auto t = static_cast<T>(2) * cross(qv, v);
            return v + w_ * t + cross(qv, t);
        }

        /** @brief Builds a 3x3 rotation matrix from this quaternion (assumes unit or normalizes). */
        [[nodiscard]] Matrix<T, 3, 3> to_matrix3() const
        {
            Quaternion q = this->normalized();
            const T w = q.w_, x = q.x_, y = q.y_, z = q.z_;
            const T xx = x * x, yy = y * y, zz = z * z;
            const T xy = x * y, xz = x * z, yz = y * z;
            const T wx = w * x, wy = w * y, wz = w * z;
            return Matrix<T, 3, 3>{
                T{1} - T{2} * (yy + zz), T{2} * (xy - wz), T{2} * (xz + wy),
                T{2} * (xy + wz), T{1} - T{2} * (xx + zz), T{2} * (yz - wx),
                T{2} * (xz - wy), T{2} * (yz + wx), T{1} - T{2} * (xx + yy)
            };
        }

        /** @brief Builds a 4x4 homogeneous rotation matrix (upper-left 3x3 from quaternion, last row/col form identity). */
        [[nodiscard]] Matrix<T, 4, 4> to_matrix4() const
        {
            auto R = to_matrix3();
            Matrix<T, 4, 4> M{};
            for (std::size_t r = 0; r < 3; ++r)
                for (std::size_t c = 0; c < 3; ++c)
                    M(r, c) = R(r, c);
            M(0, 3) = M(1, 3) = M(2, 3) = T{0};
            M(3, 0) = M(3, 1) = M(3, 2) = T{0};
            M(3, 3) = T{1};
            M(0, 0) += T{0}; // keep constexpr-friendly usage
            return M;
        }

    private:
        T w_{}, x_{}, y_{}, z_{};
    };

    /** @brief Component-wise addition. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Quaternion<T> operator+(Quaternion<T> a, const Quaternion<T>& b) noexcept
    {
        a += b;
        return a;
    }

    /** @brief Component-wise subtraction. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Quaternion<T> operator-(Quaternion<T> a, const Quaternion<T>& b) noexcept
    {
        a -= b;
        return a;
    }

    /** @brief Scalar right multiply. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Quaternion<T> operator*(Quaternion<T> q, const T& s) noexcept
    {
        q *= s;
        return q;
    }

    /** @brief Scalar left multiply. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Quaternion<T> operator*(const T& s, Quaternion<T> q) noexcept
    {
        q *= s;
        return q;
    }

    /** @brief Scalar divide. */
    export template <Arithmetic T>
    [[nodiscard]] Quaternion<T> operator/(Quaternion<T> q, const T& s)
    {
        q /= s;
        return q;
    }

    /**
     * @brief Hamilton product of two quaternions.
     * @param a left operand
     * @param b right operand
     * @return a ⊗ b
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Quaternion<T> operator*(const Quaternion<T>& a, const Quaternion<T>& b) noexcept
    {
        return Quaternion<T>{
            a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z(),
            a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y(),
            a.w() * b.y() - a.x() * b.z() + a.y() * b.w() + a.z() * b.x(),
            a.w() * b.z() + a.x() * b.y() - a.y() * b.x() + a.z() * b.w()
        };
    }

    /** @brief Dot product of two quaternions. */
    export template <Arithmetic T>
    [[nodiscard]] constexpr T dot(const Quaternion<T>& a, const Quaternion<T>& b) noexcept
    {
        return a.w() * b.w() + a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
    }

    /**
     * @brief Spherical linear interpolation between unit quaternions.
     * @param a start (need not be unit; will be normalized)
     * @param b end (need not be unit; will be normalized)
     * @param t interpolation parameter in [0,1]
     * @return normalized interpolated quaternion
     */
    export template <Arithmetic T>
    [[nodiscard]] Quaternion<T> slerp(Quaternion<T> a, Quaternion<T> b, T t)
    {
        a = a.normalized();
        b = b.normalized();
        T cos_omega = dot(a, b);
        // Ensure shortest path
        if (cos_omega < T{0})
        {
            b = Quaternion<T>{-b.w(), -b.x(), -b.y(), -b.z()};
            cos_omega = -cos_omega;
        }
        // If very close, fallback to lerp
        const T one = T{1};
        if (cos_omega > one - static_cast<T>(1e-6))
        {
            Quaternion<T> r{
                a.w() * (one - t) + b.w() * t,
                a.x() * (one - t) + b.x() * t,
                a.y() * (one - t) + b.y() * t,
                a.z() * (one - t) + b.z() * t
            };
            return r.normalized();
        }
        auto omega = static_cast<T>(std::acos(static_cast<long double>(cos_omega)));
        T sin_omega = static_cast<T>(std::sin(static_cast<long double>(omega)));
        T s0 = static_cast<T>(std::sin(static_cast<long double>((one - t) * omega))) / sin_omega;
        T s1 = static_cast<T>(std::sin(static_cast<long double>(t * omega))) / sin_omega;
        Quaternion<T> r{
            a.w() * s0 + b.w() * s1,
            a.x() * s0 + b.x() * s1,
            a.y() * s0 + b.y() * s1,
            a.z() * s0 + b.z() * s1
        };
        return r;
    }

    /**
     * @brief Streams a quaternion as (w, x, y, z).
     * @param os output stream
     * @param q quaternion
     * @return reference to os
     */
    export template <Arithmetic T>
    inline std::ostream& operator<<(std::ostream& os, const Quaternion<T>& q)
    {
        os << '(' << q.w() << ", " << q.x() << ", " << q.y() << ", " << q.z() << ')';
        return os;
    }
} // namespace glimmer
