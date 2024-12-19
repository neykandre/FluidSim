#pragma once

#include <concepts>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace Fluid {

template <typename T, typename U>
concept constructible_to = std::constructible_from<U, T>;

template <unsigned N>
struct TypeTraits {
    using FixedType =
    std::conditional_t<N == 8, int8_t,
    std::conditional_t<N == 16, int16_t,
    std::conditional_t<N == 32, int32_t,
    std::conditional_t<N == 64, int64_t, void>>>>;

    using FastFixedType = 
    std::conditional_t<N <= 8, int8_t,
    std::conditional_t<N <= 16, int16_t,
    std::conditional_t<N <= 32, int32_t,
    std::conditional_t<N <= 64, int64_t, void>>>>;
};

template <unsigned N, unsigned K, bool Fast = false>
struct Fixed {
    using type = std::conditional_t<Fast, typename TypeTraits<N>::FastFixedType,
                                    typename TypeTraits<N>::FixedType>;
    using up_type =
    std::conditional_t<N <= 32, std::conditional_t<Fast, 
                                typename TypeTraits<2 * N>::FastFixedType,
                                typename TypeTraits<2 * N>::FixedType>,
    __int128_t>;

    constexpr Fixed(int v)
        : v(v << K) {
    }

    template <std::floating_point T>
    constexpr Fixed(T f)
        : v(f * (1 << K)) {
    }

    constexpr Fixed()
        : v(0) {
    }

    template <unsigned N2, unsigned K2, bool F2>
    constexpr Fixed(const Fixed<N2, K2, F2>& other) {
        if constexpr (K > K2) {
            v = other.v << (K - K2);
        } else {
            v = other.v >> (K2 - K);
        }
    }

    Fixed& operator+=(const Fixed& other) {
        v += other.v;
        return *this;
    }

    Fixed& operator-=(const Fixed& other) {
        v -= other.v;
        return *this;
    }

    Fixed& operator*=(const Fixed& other) {
        v = ((up_type)v * other.v) >> K;
        return *this;
    }

    Fixed& operator/=(const Fixed& other) {
        if (other.v == 0) {
            throw std::runtime_error("Division by zero in Fixed-point arithmetic");
        }
        v = ((up_type)v << K) / other.v;
        return *this;
    }

    constexpr auto operator-() const {
        return Fixed::from_raw(-v);
    }

    auto abs() const {
        return v > 0 ? *this : -*this;
    }

    template <std::floating_point T>
    explicit operator T() const {
        return static_cast<T>(v) / (1 << K);
    }

    static auto from_raw(type x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    static auto random01(type x) {
        return from_raw(( x & ((up_type) 1 << K) - 1));
    }

    auto operator<=>(const Fixed&) const = default;
    bool operator==(const Fixed&) const  = default;

    type v;
};

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
auto operator+(Fixed<N, K, F> a, T b) {
    return a += Fixed<N, K, F>(b);
}

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
auto operator-(Fixed<N, K, F> a, T b) {
    return a -= Fixed<N, K, F>(b);
}

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
auto operator*(Fixed<N, K, F> a, T b) {
    return a *= Fixed<N, K, F>(b);
}

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
auto operator/(Fixed<N, K, F> a, T b) {
    return a /= Fixed<N, K, F>(b);
}

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
auto operator<=>(const Fixed<N, K, F>& a, T b) {
    return a.v <=> Fixed<N, K, F>(b).v;
}

template <unsigned N, unsigned K, bool F, constructible_to<Fixed<N, K, F>> T>
bool operator==(const Fixed<N, K, F>& a, T b) {
    return a.v == Fixed<N, K, F>(b).v;
}

template <unsigned N, unsigned K, bool F>
std::ostream& operator<<(std::ostream& out, Fixed<N, K, F> x) {
    return out << x.v / (double)(1 << K);
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T& operator+=(T& x, const Fixed<N, K, F>& y) {
    return x += (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T& operator-=(T& x, const Fixed<N, K, F>& y) {
    return x -= (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T& operator*=(T& x, const Fixed<N, K, F>& y) {
    return x *= (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T& operator/=(T& x, const Fixed<N, K, F>& y) {
    return x /= (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
auto operator<=>(const T& x, const Fixed<N, K, F>& y) {
    return x <=> (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
bool operator==(const T& x, const Fixed<N, K, F>& y) {
    return x == (T)y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T operator+(T x, const Fixed<N, K, F>& y) {
    return x += y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T operator-(T x, const Fixed<N, K, F>& y) {
    return x -= y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T operator*(T x, const Fixed<N, K, F>& y) {
    return x *= y;
}

template <std::floating_point T, unsigned N, unsigned K, bool F>
T operator/(T x, const Fixed<N, K, F>& y) {
    return x /= y;
}

} // namespace Fluid