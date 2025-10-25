module;
#include <array>
#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <cmath>

/**
 * @file
 * @brief C++23 module providing glimmer::Matrix and related operations.
 *
 * This module defines a fixed-size Matrix template with support for common
 * linear algebra operations, including multiplication, transpose, determinant,
 * and inverse. Designed for graphics (up to 4x4) and general-purpose sizes.
 */
export module glimmer.matrix;

import glimmer.vector; // reuse Vector and Arithmetic concept

namespace glimmer
{
    // Forward-declare exported Matrix so helpers can reference it
    export template <Arithmetic T, std::size_t R, std::size_t C>
    class Matrix;

    namespace detail
    {
        /**
         * @brief Bareiss fraction-free determinant for a generic N x N matrix.
         * @tparam T Arithmetic scalar type of the matrix elements.
         * @tparam N Matrix dimension (square).
         * @param A input square matrix (size N x N).
         * @return Determinant of A.
         * @details Implements the Bareiss algorithm (fraction-free Gaussian elimination)
         * with partial pivoting. Compared to naive cofactor expansion or standard
         * elimination with division at each step, the Bareiss method reduces intermediate
         * expression growth and, for integral types, performs exact division in each step
         * when the matrix is non-singular.
         *
         * Algorithm outline:
         * - Copy A into a working buffer b.
         * - For k = 0..N-2:
         *   - Choose a pivot row by maximizing |b[r][k]| for numerical stability and swap rows.
         *   - If the pivot in column k is zero, the matrix is singular and the determinant is 0.
         *   - Eliminate entries below the pivot using fraction-free update:
         *       b[i][j] = (b[i][j] * pivot - b[i][k] * b[k][j]) / prev,
         *     where prev is the previous pivot (prev = 1 for the first step).
         * - The determinant is the last diagonal element b[N-1][N-1] up to the row-swap sign.
         *
         * Numerical notes:
         * - For floating-point T, divisions are performed in T and subject to rounding.
         * - For integral T, divisions are exact under the algorithm’s invariants; however,
         *   singular or ill-conditioned inputs may still lead to division by zero detection,
         *   in which case 0 is returned.
         */
        template <Arithmetic T, std::size_t N>
        [[nodiscard]] T det_bareiss(const Matrix<T, N, N>& A)
        {
            T b[N][N]{};
            for (std::size_t r = 0; r < N; ++r)
                for (std::size_t c = 0; c < N; ++c)
                    b[r][c] = A(r, c);
            T prev = T{1};
            int sign = 1;
            for (std::size_t k = 0; k < N - 1; ++k)
            {
                std::size_t piv = k;
                for (std::size_t r = k + 1; r < N; ++r)
                    if (std::abs(b[r][k]) > std::abs(b[piv][k])) piv = r;
                if (b[piv][k] == T{}) return T{}; // singular
                if (piv != k)
                {
                    for (std::size_t c = 0; c < N; ++c) std::swap(b[piv][c], b[k][c]);
                    sign = -sign;
                }
                const T pivot = b[k][k];
                for (std::size_t i = k + 1; i < N; ++i)
                {
                    for (std::size_t j = k + 1; j < N; ++j)
                    {
                        b[i][j] = (b[i][j] * pivot - b[i][k] * b[k][j]) / prev;
                    }
                    b[i][k] = T{};
                }
                prev = pivot;
            }
            T det = b[N - 1][N - 1];
            if (sign < 0) det = -det;
            return det;
        }


        /**
         * @brief Generic Gauss–Jordan matrix inverse for size N x N.
         * @tparam T Arithmetic scalar type of the matrix elements.
         * @tparam N Matrix dimension (square).
         * @param A input square matrix to invert (size N x N).
         * @return The inverse matrix A^{-1}.
         * @throws std::domain_error if the matrix is singular (non-invertible).
         */
        template <Arithmetic T, std::size_t N>
        [[nodiscard]] Matrix<T, N, N> inverse_gauss_jordan_N(const Matrix<T, N, N>& A)
        {
            std::array<std::array<T, 2 * N>, N> aug{};
            for (std::size_t r = 0; r < N; ++r)
            {
                for (std::size_t c = 0; c < N; ++c) aug[r][c] = A(r, c);
                for (std::size_t c = 0; c < N; ++c) aug[r][N + c] = (r == c) ? T{1} : T{0};
            }
            for (std::size_t col = 0; col < N; ++col)
            {
                std::size_t piv = col;
                for (std::size_t r = col + 1; r < N; ++r)
                    if (std::abs(aug[r][col]) > std::abs(aug[piv][col])) piv = r;
                if (aug[piv][col] == T{}) throw std::domain_error("singular matrix");
                if (piv != col)
                {
                    for (std::size_t c = 0; c < 2 * N; ++c) std::swap(aug[piv][c], aug[col][c]);
                }
                const T pivot = aug[col][col];
                for (std::size_t c = 0; c < 2 * N; ++c) aug[col][c] /= pivot;
                for (std::size_t r = 0; r < N; ++r)
                {
                    if (r == col) continue;
                    const T factor = aug[r][col];
                    if (factor != T{})
                        for (std::size_t c = 0; c < 2 * N; ++c) aug[r][c] -= factor * aug[col][c];
                }
            }
            Matrix<T, N, N> inv{};
            for (std::size_t r = 0; r < N; ++r)
                for (std::size_t c = 0; c < N; ++c)
                    inv(r, c) = aug[r][N + c];
            return inv;
        }

    } // namespace detail
    /**
     * @brief Fixed-size matrix with R rows and C columns.
     * @tparam T Arithmetic scalar type.
     * @tparam R Number of rows.
     * @tparam C Number of columns.
     * @details Row-major contiguous storage. Provides element access, iteration,
     * arithmetic, matrix/vector multiplication, transpose, determinant, and inverse.
     */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    class Matrix
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using storage_type = std::array<T, R * C>;
        using iterator = storage_type::iterator;
        using const_iterator = storage_type::const_iterator;

        /** @brief Constructs a zero-initialized matrix. */
        constexpr Matrix() noexcept = default;

        /** @brief Constructs a matrix with all elements set to value. */
        constexpr explicit Matrix(const T& value) noexcept { data_.fill(value); }
        /**
         * @brief Constructs a matrix from exactly R*C arguments in row-major order.
         * @tparam Args parameter pack convertible to T
         * @note Enforced at compile time to have exactly R*C arguments.
         */
        template <class... Args>
            requires (sizeof...(Args) == R * C) && (std::conjunction_v<std::is_convertible<Args, T>...>)
        constexpr Matrix(Args... args) noexcept : data_{static_cast<T>(args)...}
        {
        }

        /** @brief Constructs from an existing storage array. */
        constexpr explicit Matrix(const storage_type& arr) noexcept : data_{arr}
        {
        }

        /**
         * @brief Unchecked element access in row-major order.
         * @param r row index in [0, R)
         * @param c column index in [0, C)
         * @return reference to element (r,c)
         * @warning No bounds checking; use at() if checks are desired.
         */
        [[nodiscard]] constexpr T& operator()(size_type r, size_type c) noexcept { return data_[index(r, c)]; }
        /**
         * @brief Unchecked const element access in row-major order.
         * @param r row index in [0, R)
         * @param c column index in [0, C)
         * @return const reference to element (r,c)
         * @warning No bounds checking; use at() if checks are desired.
         */
        [[nodiscard]] constexpr const T& operator()(size_type r, size_type c) const noexcept
        {
            return data_[index(r, c)];
        }

        /**
         * @brief Bounds-checked element access.
         * @param r row index in [0, R)
         * @param c column index in [0, C)
         * @return reference to element (r,c)
         * @throws std::out_of_range if r>=R or c>=C
         */
        [[nodiscard]] T& at(size_type r, size_type c)
        {
            if (r >= R || c >= C) throw std::out_of_range("Matrix::at index out of range");
            return data_[r * C + c];
        }

        /**
         * @brief Bounds-checked const element access.
         * @param r row index in [0, R)
         * @param c column index in [0, C)
         * @return const reference to element (r,c)
         * @throws std::out_of_range if r>=R or c>=C
         */
        [[nodiscard]] const T& at(size_type r, size_type c) const
        {
            if (r >= R || c >= C) throw std::out_of_range("Matrix::at index out of range");
            return data_[r * C + c];
        }

        /** @brief Number of rows. */
        [[nodiscard]] static consteval size_type rows() noexcept { return R; }
        /** @brief Number of columns. */
        [[nodiscard]] static consteval size_type cols() noexcept { return C; }
        /** @brief Total number of elements (R*C). */
        [[nodiscard]] static consteval size_type size() noexcept { return R * C; }

        /** @brief Pointer to underlying contiguous storage (row-major). */
        [[nodiscard]] constexpr T* data() noexcept { return data_.data(); }
        /** @brief Const pointer to underlying contiguous storage (row-major). */
        [[nodiscard]] constexpr const T* data() const noexcept { return data_.data(); }

        /** @brief Iterator to the first element (row-major). */
        [[nodiscard]] constexpr iterator begin() noexcept { return data_.begin(); }
        /** @brief Iterator past the last element. */
        [[nodiscard]] constexpr iterator end() noexcept { return data_.end(); }
        /** @brief Const iterator to the first element. */
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_.begin(); }
        /** @brief Const iterator past the last element. */
        [[nodiscard]] constexpr const_iterator end() const noexcept { return data_.end(); }
        /** @brief Const iterator to the first element. */
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
        /** @brief Const iterator past the last element. */
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_.cend(); }

        /** @brief Returns a copy of this matrix (no-op). */
        [[nodiscard]] constexpr Matrix operator+() const noexcept { return *this; }
        /** @brief Returns the element-wise negation of this matrix. */
        [[nodiscard]] constexpr Matrix operator-() const noexcept
        {
            Matrix out;
            for (size_type i = 0; i < R * C; ++i) out.data_[i] = -data_[i];
            return out;
        }

        /** @brief Component-wise addition. */
        constexpr Matrix& operator+=(const Matrix& rhs) noexcept
        {
            for (size_type i = 0; i < R * C; ++i) data_[i] += rhs.data_[i];
            return *this;
        }

        /** @brief Component-wise subtraction. */
        constexpr Matrix& operator-=(const Matrix& rhs) noexcept
        {
            for (size_type i = 0; i < R * C; ++i) data_[i] -= rhs.data_[i];
            return *this;
        }

        /** @brief Scales all elements by s. */
        constexpr Matrix& operator*=(const T& s) noexcept
        {
            for (auto& x : data_) x *= s;
            return *this;
        }

        /**
         * @brief Divides all elements by s.
         * @throws std::domain_error if s==0 (or abs(s)==0 for floating types)
         */
        Matrix& operator/=(const T& s)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if (s == 0) throw std::domain_error("Matrix::operator/= divide by zero");
            }
            else
            {
                if (std::abs(s) == T{}) throw std::domain_error("Matrix::operator/= divide by zero");
            }
            for (auto& x : data_) x /= s;
            return *this;
        }

        /** @brief Returns the zero matrix. */
        [[nodiscard]] static constexpr Matrix zeros() noexcept { return Matrix{}; }
        /** @brief Returns a matrix with all elements set to v. */
        [[nodiscard]] static constexpr Matrix fill(const T& v) noexcept { return Matrix{v}; }
        /**
         * @brief Returns the identity matrix.
         * @note Only available for square matrices (R==C);
         * diagonal set to 1 and off-diagonal to 0.
         */
        [[nodiscard]] static constexpr Matrix identity()
        {
            static_assert(R == C, "identity is defined only for square matrices");
            Matrix I{};
            for (size_type i = 0; i < R; ++i) I(i, i) = T{1};
            return I;
        }

        /**
         * @brief Returns the transpose of this matrix.
         * @return Matrix with rows and columns swapped (C x R)
         */
        [[nodiscard]] constexpr Matrix<T, C, R> transposed() const noexcept
        {
            Matrix<T, C, R> t;
            for (size_type r = 0; r < R; ++r)
                for (size_type c = 0; c < C; ++c)
                    t(c, r) = (*this)(r, c);
            return t;
        }

        /**
         * @brief Transposes the matrix in place.
         * @note Only available for square matrices (R==C).
         */
        constexpr void transpose_in_place() noexcept
        {
            static_assert(R == C, "transpose_in_place requires a square matrix");
            for (size_type r = 0; r < R; ++r)
            {
                for (size_type c = r + 1; c < C; ++c)
                {
                    auto tmp = (*this)(r, c);
                    (*this)(r, c) = (*this)(c, r);
                    (*this)(c, r) = tmp;
                }
            }
        }

        /**
         * @brief Determinant of a square matrix.
         * @return det(A)
         * @details Uses specialized formulas for sizes 1..3 and a generic
         * fraction-free elimination with partial pivoting for size >= 4.
         * Returns zero for singular matrices.
         */
        [[nodiscard]] T det() const
        {
            static_assert(R == C, "determinant is defined only for square matrices");
            if constexpr (R == 1)
            {
                return (*this)(0, 0);
            }
            else if constexpr (R == 2)
            {
                return (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
            }
            else if constexpr (R == 3)
            {
                const T a = (*this)(0, 0), b = (*this)(0, 1), c = (*this)(0, 2);
                const T d = (*this)(1, 0), e = (*this)(1, 1), f = (*this)(1, 2);
                const T g = (*this)(2, 0), h = (*this)(2, 1), i = (*this)(2, 2);
                return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
            }
            else
            {
                return detail::det_bareiss<T, R>(*this);
            }
        }

        /**
                 * @brief Inverse of a square matrix.
                 * @return A^{-1}
                 * @throws std::domain_error if the matrix is singular (non-invertible)
                 * @details Uses closed forms for sizes 1..3, and a generic Gauss–Jordan
                 * elimination with partial pivoting for size >= 4.
                 */
        [[nodiscard]] Matrix inverse() const
        {
            static_assert(R == C, "inverse is defined only for square matrices");
            if constexpr (R == 1)
            {
                if ((*this)(0, 0) == T{}) throw std::domain_error("singular matrix");
                Matrix inv{};
                inv(0, 0) = T{1} / (*this)(0, 0);
                return inv;
            }
            else if constexpr (R == 2)
            {
                const T d = det();
                if (d == T{}) throw std::domain_error("singular matrix");
                Matrix inv{};
                inv(0, 0) = (*this)(1, 1) / d;
                inv(0, 1) = -(*this)(0, 1) / d;
                inv(1, 0) = -(*this)(1, 0) / d;
                inv(1, 1) = (*this)(0, 0) / d;
                return inv;
            }
            else if constexpr (R == 3)
            {
                const T d = det();
                if (d == T{}) throw std::domain_error("singular matrix");
                Matrix adj{};
                adj(0, 0) = (*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1);
                adj(0, 1) = -((*this)(1, 0) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 0));
                adj(0, 2) = (*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0);
                adj(1, 0) = -((*this)(0, 1) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 1));
                adj(1, 1) = (*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0);
                adj(1, 2) = -((*this)(0, 0) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 0));
                adj(2, 0) = (*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1);
                adj(2, 1) = -((*this)(0, 0) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 0));
                adj(2, 2) = (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
                return (T{1} / d) * adj.transposed(); // inverse = adj(A)^T / det
            }
            else
            {
                return detail::inverse_gauss_jordan_N<T, R>(*this);
            }
        }

        // Comparisons
        /** @brief Exact element-wise equality comparison. */
        [[nodiscard]] constexpr bool operator==(const Matrix& rhs) const noexcept { return data_ == rhs.data_; }
        /** @brief Logical negation of operator==. */
        [[nodiscard]] constexpr bool operator!=(const Matrix& rhs) const noexcept { return !(*this == rhs); }

    private:
        /** @brief Converts 2D indices (r,c) to a row-major linear index. */
        [[nodiscard]] static constexpr size_type index(size_type r, size_type c) noexcept { return r * C + c; }
        storage_type data_{};
    };

    /** @brief Component-wise matrix addition. */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] constexpr Matrix<T, R, C> operator+(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    /** @brief Component-wise matrix subtraction. */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] constexpr Matrix<T, R, C> operator-(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs) noexcept
    {
        lhs -= rhs;
        return lhs;
    }

    /** @brief Scales a matrix by a scalar (right multiply). */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] constexpr Matrix<T, R, C> operator*(Matrix<T, R, C> m, const T& s) noexcept
    {
        m *= s;
        return m;
    }

    /** @brief Scales a matrix by a scalar (left multiply). */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] constexpr Matrix<T, R, C> operator*(const T& s, Matrix<T, R, C> m) noexcept
    {
        m *= s;
        return m;
    }

    /** @brief Divides a matrix by a scalar (component-wise). */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] Matrix<T, R, C> operator/(Matrix<T, R, C> m, const T& s)
    {
        m /= s;
        return m;
    }

    /**
     * @brief Matrix multiplication (row-by-column).
     * @tparam T scalar type
     * @tparam R left rows
     * @tparam K inner dimension
     * @tparam C right columns
     * @param a left matrix (R x K)
     * @param b right matrix (K x C)
     * @return product (R x C)
     */
    export template <Arithmetic T, std::size_t R, std::size_t K, std::size_t C>
    [[nodiscard]] constexpr Matrix<T, R, C> operator*(const Matrix<T, R, K>& a, const Matrix<T, K, C>& b) noexcept
    {
        Matrix<T, R, C> out{};
        for (std::size_t r = 0; r < R; ++r)
        {
            for (std::size_t c = 0; c < C; ++c)
            {
                T acc{};
                for (std::size_t k = 0; k < K; ++k) acc += a(r, k) * b(k, c);
                out(r, c) = acc;
            }
        }
        return out;
    }

    /**
     * @brief Matrix-vector multiplication.
     * @param a matrix (R x C)
     * @param x vector (size C)
     * @return vector (size R)
     */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    [[nodiscard]] constexpr Vector<T, R> operator*(const Matrix<T, R, C>& a, const Vector<T, C>& x) noexcept
    {
        Vector<T, R> out{};
        for (std::size_t r = 0; r < R; ++r)
        {
            T acc{};
            for (std::size_t c = 0; c < C; ++c) acc += a(r, c) * x[c];
            out[r] = acc;
        }
        return out;
    }

    /**
     * @brief Streams a matrix as [[row0], [row1], ...] without spaces.
     * @return reference to os
     */
    export template <Arithmetic T, std::size_t R, std::size_t C>
    std::ostream& operator<<(std::ostream& os, const Matrix<T, R, C>& m)
    {
        os << '[';
        for (std::size_t r = 0; r < R; ++r)
        {
            os << '[';
            for (std::size_t c = 0; c < C; ++c)
            {
                os << m(r, c);
                if (c + 1 < C) os << ',';
            }
            os << ']';
            if (r + 1 < R) os << ',';
        }
        os << ']';
        return os;
    }
} // namespace glimmer
