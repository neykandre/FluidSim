#pragma once

#include "FluidSim.hpp"
#include "Types.hpp"
#include <array>
#include <csignal>
#include <string>
#include <string_view>

using namespace std::literals::string_view_literals;

#ifndef TYPES
#define TYPES FIXED(64, 16)
#endif

#ifndef SIZES
#define SIZES S(0, 0)
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define TYPES_STRING STRINGIFY((TYPES))
#define SIZES_STRING STRINGIFY((SIZES))

constexpr std::string_view erase_paren(std::string_view input) {
    std::string_view type = input;
    type.remove_prefix(1);
    type.remove_suffix(1);
    return type;
}

constexpr size_t get_size(std::string_view input) {
    size_t size     = !input.empty();
    bool left_paren = false;
    for (auto c : input) {
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
}

constexpr size_t types_count = get_size(erase_paren(TYPES_STRING));
constexpr size_t sizes_count = get_size(erase_paren(SIZES_STRING));

template <size_t size>
constexpr auto parse_definition(auto input) {
    std::string_view type = erase_paren(input);

    std::array<std::string_view, size> result;
    for (int i = 0; i < size; i++) {
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
}

constexpr auto types_names = parse_definition<types_count>(TYPES_STRING);
constexpr auto sizes_names = parse_definition<sizes_count>(SIZES_STRING);

#define DOUBLE double
#define FLOAT float
#define FIXED(x, y) Fluid::Fixed<x, y>
#define FAST_FIXED(x, y) Fluid::Fixed<x, y, true>
#define S(x, y) Fluid::StaticSize<x, y>

class Mapper {

    template <typename F, typename... Ts, size_t... Is>
    constexpr static bool choose_func(std::string_view name, F f, auto names,
                                      std::index_sequence<Is...>) {
        bool was_found = false;
        (
            [&]<size_t i> {
                if (names[i] == name) {
                    was_found = true;
                    f.template operator()<std::remove_cvref_t<
                        std::tuple_element_t<i, std::tuple<Ts...>>>>();
                }
            }.template operator()<Is>(),
            ...);
        return was_found;
    }

    template <typename... Ts>
    bool map_string(std::string_view arg, auto names, auto f) {
        return choose_func<decltype(f), Ts...>(arg, f, names,
                                               std::index_sequence_for<Ts...>{});
    }

    void map_type(std::string_view arg, auto f) {
        map_string<TYPES>(arg, types_names, f);
    }

    void map_size(std::string_view arg, auto f) {
        if (!map_string<SIZES>(arg, sizes_names, f)) {
            f.template operator()<Fluid::StaticSize<0, 0>>();
        }
    }


static std::function<void(int)> shutdown_handler;
static void signal_handler(int signal) {
    shutdown_handler(signal);
}

  public:
    Mapper() = default;

    explicit Mapper(const std::string& p_type, const std::string& v_type,
                    const std::string& v_flow_type, const ::std::string& path)
        : m_p_type(p_type),
          m_v_type(v_type),
          m_v_flow_type(v_flow_type),
          m_rows{}, m_cols{} {
        std::ifstream file(path);
        assert(file.is_open());
        while (!file.eof()) {
            std::string line;
            std::getline(file, line);
            if (m_rows > 0) {
                assert(m_cols == line.size());
            }
            m_rows++;
            m_cols = line.size();
        }
    }

    explicit Mapper(const std::string& load_path) {
        std::ifstream file(load_path);
        assert(file.is_open());
        file >> m_p_type >> m_v_type >> m_v_flow_type >> m_rows >> m_cols;
    }

    void map_instance(auto f) {
        bool p_type_was      = false;
        bool v_type_was      = false;
        bool v_flow_type_was = false;

        std::string size =
            "S(" + std::to_string(m_rows) + "," + std::to_string(m_cols) + ")";

        map_type(m_p_type, [&]<typename T1> {
            p_type_was = true;
            map_type(m_v_type, [&]<typename T2> {
                v_type_was = true;
                map_type(m_v_flow_type, [&]<typename T3> {
                    v_flow_type_was = true;
                    map_size(size, [&]<typename S> {
                        
                        f.template operator()<Fluid::FluidSim<T1, T2, T3, S>>();
                    });
                });
            });
        });

        if (!p_type_was) {
            std::cerr << "Error: Unknown type: " << m_p_type << std::endl;
        }
        if (!v_type_was) {
            std::cerr << "Error: Unknown type: " << m_v_type << std::endl;
        }
        if (!v_flow_type_was) {
            std::cerr << "Error: Unknown type: " << m_v_flow_type << std::endl;
        }
    }

    size_t get_rows() const {
        return m_rows;
    }
    size_t get_cols() const {
        return m_cols;
    }

    std::string get_p_type() const {
        return m_p_type;
    }

    std::string get_v_type() const {
        return m_v_type;
    }

    std::string get_v_flow_type() const {
        return m_v_flow_type;
    }

  private:
    std::string m_p_type;
    std::string m_v_type;
    std::string m_v_flow_type;
    size_t m_rows;
    size_t m_cols;
};