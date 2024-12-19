#pragma once

#include <type_traits>
#include <vector>
#include <array>

namespace Fluid {
template <typename T>
constexpr bool is_static = (T::value > 0);

template <typename T, typename Size>
struct Array2d {
    using Arr_t = std::conditional_t<is_static<Size>, std::array<T, Size::value>,
                                     std::vector<T>>;

  public:
    Array2d(size_t, size_t)
        requires is_static<Size>
    {};

    Array2d(size_t rows, size_t cols)
        requires(!is_static<Size>)
        : rows(rows),
          cols(cols) {
        data.resize(rows * cols, T{});
    }

    T& operator()(size_t i, size_t j)
        requires is_static<Size>
    {
        return data[i * Size::cols + j];
    }

    T& operator()(size_t i, size_t j)
        requires(!is_static<Size>)
    {
        return data[i * cols + j];
    }

    void clear() {
        std::fill(data.begin(), data.end(), T{});
    }

    Arr_t data{};

  private:
    size_t rows;
    size_t cols;
};
}