#ifndef POLY_H
#define POLY_H

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T, std::size_t N>
class poly;

namespace poly_detail {
    /////////////////////////////////// TO_POLY_CONVERTIBLE ///////////////////////////////////

    template <typename T, typename Tp, std::size_t N>
    struct to_poly_convertible : std::bool_constant<std::is_convertible_v<std::decay_t<T>, Tp> && (N > 0)> {};

    template <typename T, typename Tp, std::size_t N>
    constexpr bool to_poly_convertible_v = to_poly_convertible<T, Tp, N>::value;

    /////////////////////////////////// IS_POLY ///////////////////////////////////

    template <typename>
    struct is_poly : std::false_type {};

    template <typename T, std::size_t N>
    struct is_poly<poly<T, N>> : std::true_type {};

    template <typename T>
    constexpr bool is_poly_v = is_poly<std::decay_t<T>>::value;

    /////////////////////////////////// MUL_TYPE ///////////////////////////////////

    template <typename T, typename U>
    struct mul_type {
        using type = std::common_type_t<T, U>;
    };

    template <typename U, std::size_t N, typename V, std::size_t M>
    struct mul_type<poly<U, N>, poly<V, M>> {
        static constexpr std::size_t poly_size = N * M > 0 ? N + M - 1 : 0;
        using type = poly<typename mul_type<U, V>::type, poly_size>;
    };

    template <typename T, typename U>
    using mul_type_t = typename mul_type<T, U>::type;

    /////////////////////////////////// SHIFT ///////////////////////////////////

    // Shift of the second polynomial by number of coefficients of the first polynomial.
    template <typename T, typename U>
    requires (!is_poly_v<T>)
    constexpr auto shift(T, U p) {
        return p;
    }

    template <typename Tp, std::size_t N, typename T>
    constexpr auto shift(const poly<Tp, N>&, const T& p) {
        return const_poly(shift(std::decay_t<Tp>{}, p));
    }

} // namespace poly_detail

namespace std {
    /////////////////////////////////// COMMON_TYPE ///////////////////////////////////

    template <typename T, typename Tp, size_t N>
    struct common_type<T, poly<Tp, N>> {
        using type = poly<common_type_t<decay_t<T>, decay_t<Tp>>, (N > 0 ? N : 1)>;
    };

    template <typename Tp, size_t N, typename T>
    struct common_type<poly<Tp, N>, T> {
        using type = poly<common_type_t<decay_t<Tp>, decay_t<T>>, (N > 0 ? N : 1)>;
    };

    template <typename Tp1, size_t N1, typename Tp2, size_t N2>
    struct common_type<poly<Tp1, N1>, poly<Tp2, N2>> {
        using type = poly<common_type_t<decay_t<Tp1>, decay_t<Tp2>>, max(N1, N2)>;
    };

    /////////////////////////////////// IS_CONVERTIBLE ///////////////////////////////////

    template <typename T, typename Tp, size_t N>
    struct is_convertible<T, poly<Tp, N>> : bool_constant<is_convertible_v<T, Tp> && (N > 0)> {};

    template <typename Tp1, std::size_t N1, typename Tp2, size_t N2>
    struct is_convertible<poly<Tp1, N1>, poly<Tp2, N2>> : bool_constant<(is_convertible_v<Tp1, Tp2> && (N1 <= N2))> {};
} // namespace std

template <typename T, std::size_t N = 0>
class poly {
 private:
    std::array<T, N> coefficients{};

 public:

    /////////////////////////////////// CONSTRUCTORS ///////////////////////////////////

    constexpr poly() {}

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly(const poly<U, M>& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] = other[i];
    }

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly(poly<U, M>&& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] = std::move(other[i]);
    }

    template <typename U>
    requires poly_detail::to_poly_convertible_v<std::decay_t<U>, T, N>
    constexpr poly(U&& arg) {
        coefficients[0] = std::forward<U>(arg);
    }

    template <typename... U>
    requires (
        sizeof...(U) >= 2 && sizeof...(U) <= N
        && (poly_detail::to_poly_convertible_v<std::decay_t<U>, T, N> && ...)
    )
    constexpr poly(U&&... args) {
        std::size_t index = 0;
        ((coefficients[index++] = std::forward<U>(args)), ...);
    }

    /////////////////////////////////// OPERATOR= ///////////////////////////////////

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly& operator=(const poly<U, M>& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] = other[i];

        for (std::size_t i = M; i < N; ++i)
            coefficients[i] = T{};

        return *this;
    }

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly& operator=(poly<U, M>&& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] = std::move(other[i]);

        for (std::size_t i = M; i < N; ++i)
            coefficients[i] = T{};

        return *this;
    }

    /////////////////////////////////// OPERATOR+= ///////////////////////////////////

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly& operator+=(const poly<U, M>& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] += other[i];
        return *this;
    }

    template <typename U>
    requires std::is_convertible_v<std::decay_t<U>, poly>
    constexpr poly& operator+=(const U& other) {
        coefficients[0] += other;
        return *this;
    }

    /////////////////////////////////// OPERATOR-= ///////////////////////////////////

    template <typename U, std::size_t M>
    requires std::is_convertible_v<poly<U, M>, poly>
    constexpr poly& operator-=(const poly<U, M>& other) {
        for (std::size_t i = 0; i < M; ++i)
            coefficients[i] -= other[i];
        return *this;
    }

    template <typename U>
    requires std::is_convertible_v<std::decay_t<U>, poly>
    constexpr poly& operator-=(const U& other) {
        coefficients[0] -= other;
        return *this;
    }

    /////////////////////////////////// OPERATOR*= ///////////////////////////////////

    template <typename U>
    requires (!poly_detail::is_poly_v<U>)
    constexpr poly& operator*=(const U& x) {
        for (std::size_t i = 0; i < N; ++i)
            coefficients[i] *= x;
        return *this;
    }

    /////////////////////////////////// OPERATOR[] ///////////////////////////////////

    constexpr T& operator[](std::size_t i) {
        return coefficients[i];
    }

    constexpr const T& operator[](std::size_t i) const {
        return coefficients[i];
    }

    /////////////////////////////////// SIZE ///////////////////////////////////

    constexpr std::size_t size() const {
        return N;
    }

    /////////////////////////////////// AT ///////////////////////////////////
    constexpr auto at() const {
        return *this;
    }

    template <typename U, typename... V>
    constexpr auto at(const U& arg, const V&... args) const {
        if constexpr (N == 0) {
            if constexpr (poly_detail::is_poly_v<T>)
                return T{}.at(args...);
            else
                return T{};
        } else {
            if constexpr (poly_detail::is_poly_v<T>)
                return at_loop<U>(arg).at(args...);
            else
                return at_loop<U>(arg);
        }
    }

    template <typename U, std::size_t K>
    constexpr auto at(const std::array<U, K>& arr) const {
        return at_array(arr, std::make_index_sequence<K>{});
    }

 private:
    template <typename U, std::size_t I = 0>
    constexpr auto at_loop(const U& arg) const {
        if constexpr (I == N - 1)
            return coefficients[I];
        else
            return coefficients[I] + arg * at_loop<U, I + 1>(arg);
    }

    template <typename U, std::size_t... I>
    constexpr auto at_array(const std::array<U, sizeof...(I)>& arr, std::index_sequence<I...>) const {
        return at(arr[I]...);
    }
}; // class poly

/////////////////////////////////// CONST_POLY ///////////////////////////////////
template <typename T>
requires poly_detail::is_poly_v<T>
constexpr auto const_poly(T&& p) {
    return poly<std::decay_t<T>, 1>(std::forward<T>(p));
}


/////////////////////////////////// OPERATOR+ ///////////////////////////////////

template <typename T, std::size_t N, typename U, std::size_t M>
constexpr auto operator+(const poly<T, N>& a, const poly<U, M>& b) {
    return std::common_type_t<poly<T, N>, poly<U, M>>(a) += b;
}

template <typename Tp, std::size_t N, typename U>
constexpr auto operator+(const poly<Tp, N>& polynomial, const U& value) {
    return std::common_type_t<poly<Tp, N>, U>(polynomial) += value;
}

template <typename U, typename Tp, std::size_t N>
constexpr auto operator+(const U& value, const poly<Tp, N>& polynomial) {
    return polynomial + value;
}

/////////////////////////////////// OPERATOR- ///////////////////////////////////

template <typename T, std::size_t N, typename U, std::size_t M>
constexpr auto operator-(const poly<T, N>& a, const poly<U, M>& b) {
    return std::common_type_t<poly<T, N>, poly<U, M>>(a) -= b;
}

template <typename Tp, std::size_t N, typename U>
constexpr auto operator-(const poly<Tp, N>& polynomial, const U& value) {
    return std::common_type_t<poly<Tp, N>, U>(polynomial) -= value;
}

template <typename U, typename Tp, std::size_t N>
constexpr auto operator-(const U& value, const poly<Tp, N>& polynomial) {
    return poly(value) - polynomial;
}

template <typename T, std::size_t N>
constexpr auto operator-(const poly<T, N>& p) {
    poly<T, N> result{};
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = -p[i];
    }
    return result;
}

/////////////////////////////////// OPERATOR* ///////////////////////////////////

template <typename T, std::size_t N, typename U, std::size_t M>
constexpr auto operator*(const poly<T, N>& a, const poly<U, M>& b) {
    if constexpr (N == 0 || M == 0) {
        return poly<std::common_type_t<T, U>, 0>{};
    } else {
        poly_detail::mul_type_t<poly<T, N>, poly<U, M>> result{};
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < M; ++j) {
                result[i + j] += a[i] * b[j];
            }
        }
        return result;
    }
}

template <typename T, std::size_t N, typename U>
constexpr auto operator*(const poly<T, N>& polynomial, const U& value) {
    if constexpr (N == 0)
        return poly<std::common_type_t<T, U>, 0>{};

    return std::common_type_t<poly<T, N>, U>(polynomial) *= value;
}

template <typename U, typename T, std::size_t N>
constexpr auto operator*(const U& value, const poly<T, N>& polynomial) {
    return polynomial * value;
}

/////////////////////////////////// CROSS ///////////////////////////////////

template <typename T, typename U>
requires (poly_detail::is_poly_v<T> && poly_detail::is_poly_v<U>)
constexpr auto cross(T a, U b){
    return a * poly_detail::shift(a, b);
}

/////////////////////////////////// GUIDES ///////////////////////////////////

template <typename U, std::size_t M>
poly(const poly<U, M>&) -> poly<U, M>;

template <typename U, std::size_t M>
poly(poly<U, M>&&) -> poly<U, M>;

template <typename U>
requires (!poly_detail::is_poly_v<U>)
poly(U&&) -> poly<std::decay_t<U>, 1>;

template <typename... U>
requires (sizeof...(U) >= 2)
poly(U&&...) -> poly<std::common_type_t<std::decay_t<U>...>, sizeof...(U)>;

#endif // POLY_H
