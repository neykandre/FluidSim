#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace Fluid {
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

template <unsigned N, unsigned K>
struct Fixed {
    using type = TypeTraits<N>::FixedType;

    constexpr Fixed(int v)
        : v(v << K) {
    }
    constexpr Fixed(float f)
        : v(f * (1 << K)) {
    }
    constexpr Fixed(double f)
        : v(f * (1 << K)) {
    }
    constexpr Fixed()
        : v(0) {
    }

    static auto from_raw(type x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    type v;

    auto operator-() const {
        return from_raw(-v);
    }
    auto operator<=>(const Fixed&) const = default;
    bool operator==(const Fixed&) const  = default;
};

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator+(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    Fixed<std::max(N1, N2), std::max(K1, K2)> ret;
    if (K1 > K2) {
        ret.v = b.v;
        ret.v <<= K1 - K2;
        ret.v += a.v;
    } else {
        ret.v = a.v;
        ret.v <<= K2 - K1;
        ret.v += b.v;
    }
    return ret;
}

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator-(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    return a + (-b);
}

// template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
// auto operator*(Fixed<N1, K1> a, Fixed<N2, K2> b) {
//     using doubled_type = TypeTraits<N1 + N2>::FastFixedType;
//     doubled_type c = a.v * b.v;

// }

// Fixed operator/(Fixed a, Fixed b) {
//     return Fixed::from_raw(((int64_t)a.v << 16) / b.v);
// }

// Fixed& operator+=(Fixed& a, Fixed b) {
//     return a = a + b;
// }

// Fixed& operator-=(Fixed& a, Fixed b) {
//     return a = a - b;
// }

// Fixed& operator*=(Fixed& a, Fixed b) {
//     return a = a * b;
// }

// Fixed& operator/=(Fixed& a, Fixed b) {
//     return a = a / b;
// }

// Fixed operator-(Fixed x) {
//     return Fixed::from_raw(-x.v);
// }

// Fixed abs(Fixed x) {
//     if (x.v < 0) {
//         x.v = -x.v;
//     }
//     return x;
// }

template <unsigned N, unsigned K>
std::ostream& operator<<(std::ostream& out, Fixed<N, K> x) {
    return out << x.v / (double)(1 << K);
}

// template <int N, int K>
// struct Fast_Fixed {};
} // namespace Fluid