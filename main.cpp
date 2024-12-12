#include <array>
#include <iostream>
#include <string_view>

using namespace std::literals::string_view_literals;

#ifndef TYPES
#define TYPES DOUBLE
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define TYPES_STRING STRINGIFY((TYPES))

constexpr auto input_types_string = [] {
    std::string_view type = TYPES_STRING;
    type.remove_prefix(1);
    type.remove_suffix(1);
    return type;
}();

constexpr auto types_count = [] {
    std::string_view type = input_types_string;
    size_t size           = !type.empty();
    bool left_paren       = false;
    for (auto c : type) {
        if (c == ',' && !left_paren) {
            size++;
        }
        if (c == '(') {
            left_paren = true;
        }
        if (c == ')') {
            left_paren = false;
        }
    }
    return size;
}();

constexpr std::array types_names = { [] {
    std::string_view type        = input_types_string;
    std::array<std::string_view, types_count> result;
    for (int i = 0; i < types_count; i++) {
        size_t par_pos = type.find('(');
        size_t pos     = type.find(',');
        if (par_pos != std::string_view::npos && pos != std::string_view::npos &&
            pos > par_pos) {
            pos = type.find(',', type.find(')'));
        }
        result[i] = type.substr(0, pos);
        type.remove_prefix(pos + 1);
        while (type[0] == ' ') {
            type.remove_prefix(1);
        }
    }
    return result;
}() };

#define DOUBLE double
#define FLOAT float
#define FIXED(x, y) Fixed<x, y>
#define FAST_FIXED(x, y) Fast_Fixed<x, y>

template <int N, int K>
struct Fixed {};

template <int N, int K>
class Fast_Fixed {};

template <typename F, typename... Ts, size_t... Is>
constexpr void choose_func(std::string_view name, F f, std::index_sequence<Is...>) {
    (
        [&]<size_t i> {
            if (types_names[i] == name) {
                f.template operator()<std::remove_cvref_t<
                    std::tuple_element_t<i, std::tuple<Ts...>>>>();
            }
        }.template operator()<Is>(),
        ...);
}

void map(std::string_view arg, auto f) {
    choose_func<decltype(f), TYPES>(arg, f, std::index_sequence_for<TYPES>{});
}

template <typename P, typename Velocity_t, typename Velocity_flow>
class FluidSim {};

int main(int argc, char** argv) {
    map(argv[1], [&]<typename T1> {
        map(argv[2], [&]<typename T2> {
            map(argv[3], [&]<typename T3> {
                std::cout << typeid(T1).name() << " " << typeid(T2).name() << " "
                          << typeid(T3).name(

                             );
                FluidSim<T1, T2, T3> sim;
                // std::cout << sizeof(FluidSim<T1, T2, T3>) << std::endl;
            });
        });
    });
    return 0;
}
