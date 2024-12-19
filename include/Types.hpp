#pragma once

#include <concepts>
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
    template<std::floating_point T>
    constexpr Fixed(T f)
        : v(f * (1 << K)) {
    }
    constexpr Fixed()
        : v(0) {
    }

    template <unsigned N2, unsigned K2>
    explicit constexpr Fixed(const Fixed<N2, K2>& other) {
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
        v = (v * other.v) >> K;
        return *this;
    }

    Fixed& operator/=(const Fixed& other) {
        if (other.v == 0) {
            throw std::runtime_error("Division by zero in Fixed-point arithmetic");
        }
        v = (v << K) / other.v;
        return *this;
    }

    constexpr auto operator-() const {
        return Fixed::from_raw(-v);
    }

    template<std::floating_point T>
    explicit operator T() const {
        return static_cast<T>(v) / (1 << K);
    }

    static auto from_raw(type x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }
    auto operator<=>(const Fixed&) const = default;
    bool operator==(const Fixed&) const  = default;

    type v;
};

template <unsigned N, unsigned K, std::floating_point T>
Fixed<N, K> operator+(Fixed<N, K> a, T b) {
    return a += Fixed<N, K>(b);
}

template <unsigned N, unsigned K, std::floating_point T>
Fixed<N, K> operator+(T a, Fixed<N, K> b) {
    return Fixed<N, K>(a) += b;
}

template <unsigned N, unsigned K>
Fixed<N, K> operator-(Fixed<N, K> a, double b) {
    return a -= Fixed<N, K>(b);
}

template <unsigned N, unsigned K>
Fixed<N, K> operator-(double a, Fixed<N, K> b) {
    return Fixed<N, K>(a) -= b;
}

template <unsigned N, unsigned K>
Fixed<N, K> operator*(Fixed<N, K> a, double b) {
    return a *= Fixed<N, K>(b);
}

template <unsigned N, unsigned K>
Fixed<N, K> operator*(double a, Fixed<N, K> b) {
    return Fixed<N, K>(a) *= b;
}

template <unsigned N, unsigned K>
Fixed<N, K> operator/(Fixed<N, K> a, double b) {
    return a /= Fixed<N, K>(b);
}

template <unsigned N, unsigned K>
Fixed<N, K> operator/(double a, Fixed<N, K> b) {
    return Fixed<N, K>(a) /= b;
}

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator+(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    using ResultType = Fixed<std::max(N1, N2), std::max(K1, K2)>;
    ResultType result;
    if (K1 > K2) {
        result.v = b.v << (K1 - K2);
        result.v += a.v;
    } else {
        result.v = a.v << (K2 - K1);
        result.v += b.v;
    }
    return result;
}

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator-(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    return a + (-b);
}

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator*(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    using ResultType = Fixed<std::max(N1, N2), K1 + K2>;
    ResultType result;
    result.v = (static_cast<ResultType::type>(a.v) * static_cast<ResultType::type>(b.v)) >> K1;
    return result;
}

template <unsigned N1, unsigned K1, unsigned N2, unsigned K2>
auto operator/(Fixed<N1, K1> a, Fixed<N2, K2> b) {
    if (b.v == 0) {
        throw std::runtime_error("Division by zero in Fixed-point arithmetic");
    }
    using ResultType = Fixed<std::max(N1, N2), K1 - K2>;
    ResultType result;
    result.v = (static_cast<ResultType::type>(a.v) << K1) / b.v;
    return result;
}


template<unsigned N, unsigned K>
Fixed<N, K> abs(Fixed<N, K> x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
}

template <unsigned N, unsigned K>
std::ostream& operator<<(std::ostream& out, Fixed<N, K> x) {
    return out << x.v / (double)(1 << K);
}

template<unsigned N, unsigned K>
struct Fast_Fixed {

};

} // namespace Fluid