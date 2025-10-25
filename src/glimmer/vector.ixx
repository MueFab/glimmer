module;

#include <array>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <type_traits>

/**
 * @file
 * @brief C++23 module providing glimmer::Vector and related algorithms.
 *
 * This module defines a fixed-size geometric vector template and common operations
 * such as arithmetic, dot- and cross-products. It follows modern C++23 best practices
 * and is intended for graphics and math utilities.
 */
export module glimmer.vector;


namespace glimmer
{
    /**
     * @brief Concept constraining a type to arithmetic scalars.
     * @tparam T candidate type
     * @details Satisfied if std::is_arithmetic_v<T> is true. Used to restrict vector
     * templates and operations to integral and floating-point types.
     */
    export template <class T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    /**
     * @brief Fixed-size geometric vector.
     * @tparam T Arithmetic scalar type.
     * @tparam N Dimension (N > 0).
     * @details Provides element access, iteration, arithmetic operations, norms,
     * and common helpers. Designed for graphics and math. Storage is contiguous
     * (std::array<T, N>), row-vector semantics.
     */
    export template <Arithmetic T, std::size_t N>
    class Vector
    {
        static_assert(N > 0, "Vector dimension N must be greater than 0");

    public:
        using value_type = T;
        using size_type = std::size_t;
        using iterator = std::array<T, N>::iterator;
        using const_iterator = std::array<T, N>::const_iterator;

        /**
         * @brief Constructs a zero-initialized vector.
         * @note Elements are value-initialized (0 for arithmetic types).
         */
        constexpr Vector() noexcept = default;

        /**
         * @brief Constructs a vector with all components set to a single value.
         * @param value value assigned to each component
         */
        explicit constexpr Vector(const T& value) noexcept
        {
            data_.fill(value);
        }

        /**
         * @brief Constructs from an initializer list with exactly N elements.
         * @param init list of N values in component order
         * @throws std::invalid_argument if init.size() != N
         */
        constexpr Vector(std::initializer_list<T> init)
        {
            if (init.size() != N)
            {
                throw std::invalid_argument("Initializer list size must match vector dimension");
            }
            std::ranges::copy(init, data_.begin());
        }

        /**
             * @brief Constructs from an existing std::array.
             * @param arr array with N elements used to initialize storage
             */
        explicit constexpr Vector(const std::array<T, N>& arr) noexcept : data_{arr}
        {
        }

        /**
         * @brief Unchecked element access.
         * @param i index in [0, N)
         * @return reference to element i
         * @warning No bounds checking; use at() for checked access.
         */
        [[nodiscard]] constexpr T& operator[](size_type i) noexcept { return data_[i]; }
        /**
         * @brief Unchecked element access (const overload).
         * @param i index in [0, N)
         * @return const reference to element i
         * @warning No bounds checking; use at() for checked access.
         */
        [[nodiscard]] constexpr const T& operator[](size_type i) const noexcept { return data_[i]; }

        /**
         * @brief Bounds-checked element access.
         * @param i index in [0, N)
         * @return reference to element i
         * @throws std::out_of_range if i >= N
         */
        [[nodiscard]] constexpr T& at(size_type i)
        {
            if (i >= N) throw std::out_of_range("Vector index out of range");
            return data_[i];
        }

        /**
         * @brief Bounds-checked element access (const overload).
         * @param i index in [0, N)
         * @return const reference to element i
         * @throws std::out_of_range if i >= N
         */
        [[nodiscard]] constexpr const T& at(size_type i) const
        {
            if (i >= N) throw std::out_of_range("Vector index out of range");
            return data_[i];
        }

        /**
             * @brief Number of components in the vector.
             * @return N
             */
        [[nodiscard]] static consteval size_type size() noexcept { return N; }

        /** @brief Pointer to underlying contiguous storage. */
        [[nodiscard]] constexpr T* data() noexcept { return data_.data(); }
        /** @brief Const pointer to underlying contiguous storage. */
        [[nodiscard]] constexpr const T* data() const noexcept { return data_.data(); }

        /** @brief Iterator to the first component. */
        [[nodiscard]] constexpr iterator begin() noexcept { return data_.begin(); }
        /** @brief Iterator past the last component. */
        [[nodiscard]] constexpr iterator end() noexcept { return data_.end(); }
        /** @brief Const iterator to the first component. */
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_.begin(); }
        /** @brief Const iterator past the last component. */
        [[nodiscard]] constexpr const_iterator end() const noexcept { return data_.end(); }
        /** @brief Const iterator to the first component. */
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
        /** @brief Const iterator past the last component. */
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_.cend(); }

        /** @brief Returns a copy of this vector (no-op). */
        [[nodiscard]] constexpr Vector operator+() const noexcept { return *this; }
        /** @brief Returns the negation of this vector (component-wise). */
        [[nodiscard]] constexpr Vector operator-() const noexcept
        {
            Vector r{};
            for (size_type i = 0; i < N; ++i) r[i] = -data_[i];
            return r;
        }

        /** @brief Component-wise addition. */
        constexpr Vector& operator+=(const Vector& rhs) noexcept
        {
            for (size_type i = 0; i < N; ++i) data_[i] += rhs[i];
            return *this;
        }

        /** @brief Component-wise subtraction. */
        constexpr Vector& operator-=(const Vector& rhs) noexcept
        {
            for (size_type i = 0; i < N; ++i) data_[i] -= rhs[i];
            return *this;
        }

        /** @brief Scales the vector by s (component-wise multiply). */
        constexpr Vector& operator*=(const T& s) noexcept
        {
            for (size_type i = 0; i < N; ++i) data_[i] *= s;
            return *this;
        }

        /** @brief Divides the vector by s (component-wise divide). */
        constexpr Vector& operator/=(const T& s)
        {
            for (size_type i = 0; i < N; ++i) data_[i] /= s; // integral T will integer-divide by design
            return *this;
        }

        /**
             * @brief Computes the Euclidean norm (length).
             * @return ||v||_2. For floating T, precise sqrt; for integral T, rounded.
             */
        [[nodiscard]] constexpr auto norm() const noexcept -> T
        {
            long double sum = 0.0L;
            for (size_type i = 0; i < N; ++i) sum += static_cast<long double>(data_[i]) * static_cast<long double>(data_
                [i]);
            if constexpr (std::is_floating_point_v<T>)
            {
                return static_cast<T>(std::sqrt(sum));
            }
            else
            {
                return static_cast<T>(std::llround(std::sqrt(sum)));
            }
        }

        /**
             * @brief Returns a normalized (unit length) copy of this vector.
             * @return v / ||v|| if ||v|| != 0; otherwise a zero vector.
             */
        [[nodiscard]] constexpr Vector normalized() const
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                T n = norm();
                if (n == static_cast<T>(0)) return Vector{};
                Vector r = *this;
                r /= n;
                return r;
            }
            else
            {
                T n = norm();
                if (n == static_cast<T>(0)) return Vector{};
                Vector r{};
                for (size_type i = 0; i < N; ++i) r[i] = static_cast<T>(data_[i] / n);
                return r;
            }
        }

        /** @brief Returns the zero vector. */
        [[nodiscard]] static constexpr Vector zeros() noexcept { return Vector{}; }
        /** @brief Returns a vector of all ones. */
        [[nodiscard]] static constexpr Vector ones() noexcept { return Vector(static_cast<T>(1)); }

        /**
             * @brief Unit basis vector e_i.
             * @param i index of the component to set to 1
             * @return vector with v[i] = 1 and others 0
             * @throws std::out_of_range if i >= N
             */
        [[nodiscard]] static constexpr Vector unit(size_type i)
        {
            if (i >= N) throw std::out_of_range("Vector::unit index out of range");
            Vector v{};
            v[i] = static_cast<T>(1);
            return v;
        }

        /**
             * @brief Component-wise minimum of two vectors.
             * @param a first vector
             * @param b second vector
             * @return Vector r with r[i] = min(a[i], b[i])
             */
        [[nodiscard]] static constexpr Vector min(const Vector& a, const Vector& b) noexcept
        {
            Vector r{};
            for (size_type i = 0; i < N; ++i) r[i] = a[i] < b[i] ? a[i] : b[i];
            return r;
        }

        /**
         * @brief Component-wise maximum of two vectors.
         * @param a first vector
         * @param b second vector
         * @return Vector r with r[i] = max(a[i], b[i])
         */
        [[nodiscard]] static constexpr Vector max(const Vector& a, const Vector& b) noexcept
        {
            Vector r{};
            for (size_type i = 0; i < N; ++i) r[i] = a[i] > b[i] ? a[i] : b[i];
            return r;
        }

        /** @brief Exact component-wise equality comparison. */
        [[nodiscard]] constexpr bool operator==(const Vector& rhs) const noexcept
        {
            for (size_type i = 0; i < N; ++i) if (data_[i] != rhs[i]) return false;
            return true;
        }

        /** @brief Logical negation of operator==. */
        [[nodiscard]] constexpr bool operator!=(const Vector& rhs) const noexcept { return !(*this == rhs); }

    private:
        std::array<T, N> data_{};
    };

    /**
     * @brief Component-wise vector addition.
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param lhs left operand (copied)
     * @param rhs right operand
     * @return lhs + rhs
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Vector<T, N> operator+(Vector<T, N> lhs, const Vector<T, N>& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    /**
     * @brief Component-wise vector subtraction.
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param lhs left operand (copied)
     * @param rhs right operand
     * @return lhs - rhs
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Vector<T, N> operator-(Vector<T, N> lhs, const Vector<T, N>& rhs) noexcept
    {
        lhs -= rhs;
        return lhs;
    }

    /**
     * @brief Scales a vector by a scalar (right multiply).
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param v vector (copied)
     * @param s scalar
     * @return v * s
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Vector<T, N> operator*(Vector<T, N> v, const T& s) noexcept
    {
        v *= s;
        return v;
    }

    /**
     * @brief Scales a vector by a scalar (left multiply).
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param s scalar
     * @param v vector (copied)
     * @return s * v
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Vector<T, N> operator*(const T& s, Vector<T, N> v) noexcept
    {
        v *= s;
        return v;
    }

    /**
     * @brief Divides a vector by a scalar (component-wise).
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param v vector (copied)
     * @param s scalar divisor
     * @return v / s
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr Vector<T, N> operator/(Vector<T, N> v, const T& s)
    {
        v /= s;
        return v;
    }

    /**
     * @brief Dot (inner) product of two vectors.
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param a first operand
     * @param b second operand
     * @return sum_i a[i] * b[i]
     */
    export template <Arithmetic T, std::size_t N>
    [[nodiscard]] constexpr T dot(const Vector<T, N>& a, const Vector<T, N>& b) noexcept
    {
        T sum = static_cast<T>(0);
        for (std::size_t i = 0; i < N; ++i) sum += a[i] * b[i];
        return sum;
    }

    /**
     * @brief 3D cross product.
     * @tparam T arithmetic scalar type
     * @param a left vector
     * @param b right vector
     * @return a × b
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Vector<T, 3> cross(const Vector<T, 3>& a, const Vector<T, 3>& b) noexcept
    {
        return Vector<T, 3>{
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0]
        };
    }

    /**
     * @brief Linear interpolation between two vectors.
     * @tparam T arithmetic scalar type of vectors
     * @tparam N dimension
     * @tparam U arithmetic type of interpolation parameter t
     * @param a start vector
     * @param b end vector
     * @param t interpolation factor; typically in [0,1]. No clamping is performed.
     * @return a vector equal to a + (b - a) * t, component-wise
     * @details For integral T, the result is truncated after computing in a common type.
     */
    export template <Arithmetic T, std::size_t N, Arithmetic U>
    [[nodiscard]] constexpr Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, const U& t) noexcept
    {
        using CT = std::common_type_t<T, U>;
        Vector<T, N> r{};
        const CT tt = static_cast<CT>(t);
        for (std::size_t i = 0; i < N; ++i)
        {
            const CT ai = static_cast<CT>(a[i]);
            const CT bi = static_cast<CT>(b[i]);
            r[i] = static_cast<T>(ai + (bi - ai) * tt);
        }
        return r;
    }

    /**
     * @brief Streams a vector in the form [x0, x1, ..., x{N-1}].
     * @tparam T arithmetic scalar type
     * @tparam N dimension
     * @param os output stream
     * @param v vector to print
     * @return reference to os
     */
    export template <Arithmetic T, std::size_t N>
    inline std::ostream& operator<<(std::ostream& os, const Vector<T, N>& v)
    {
        os << '[';
        for (std::size_t i = 0; i < N; ++i)
        {
            os << v[i];
            if (i + 1 < N) os << ',' << ' ';
        }
        os << ']';
        return os;
    }

    /**
     * @brief Converts a vector to a different dimensionality by truncating or extending.
     * @tparam T arithmetic scalar type
     * @tparam From source dimension
     * @tparam To target dimension
     * @param v input vector of size From
     * @param fill value used to populate new components when To > From (default 0)
     * @return Vector<T, To> where the first min(From,To) components are copied from v and the rest set to fill
     */
    export template <Arithmetic T, std::size_t From, std::size_t To>
    [[nodiscard]] constexpr Vector<T, To> resize_dim(const Vector<T, From>& v, const T& fill = T{}) noexcept
    {
        Vector<T, To> out{};
        constexpr std::size_t K = (From < To ? From : To);
        for (std::size_t i = 0; i < K; ++i) out[i] = v[i];
        if constexpr (To > From)
        {
            for (std::size_t i = From; i < To; ++i) out[i] = fill;
        }
        return out;
    }

    /**
     * @brief Pads a 3D point to homogeneous 4D by appending w=1.
     * @tparam T arithmetic scalar type
     * @param p 3D point (x,y,z)
     * @return 4D homogeneous point (x,y,z,1)
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Vector<T,4> to_homogeneous_point(const Vector<T,3>& p) noexcept
    {
        return Vector<T,4>{p[0], p[1], p[2], static_cast<T>(1)};
        // Alternatively: return resize_dim<T,3,4>(p, T{1}); but we explicitly set w=1 for clarity
    }

    /**
     * @brief Pads a 3D direction to homogeneous 4D by appending w=0.
     * @tparam T arithmetic scalar type
     * @param d 3D direction (x,y,z)
     * @return 4D homogeneous direction (x,y,z,0)
     */
    export template <Arithmetic T>
    [[nodiscard]] constexpr Vector<T,4> to_homogeneous_dir(const Vector<T,3>& d) noexcept
    {
        return Vector<T,4>{d[0], d[1], d[2], static_cast<T>(0)};
    }

} // namespace glimmer
