#pragma once

#include "Types.hpp"
#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <type_traits>
#include <vector>

namespace Fluid {

template <size_t N, size_t K>
struct StaticSize {
    static constexpr size_t rows  = N;
    static constexpr size_t cols  = K;
    static constexpr size_t value = N * K;
};

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
        data.resize(rows * cols);
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

    Arr_t data{};

  private:
    size_t rows;
    size_t cols;
};

template <typename P_t, typename V_t, typename V_flow_t,
          typename Size = StaticSize<0, 0>>
class FluidSim {

    template <typename T>
    using Arr_t = Array2d<T, Size>;

  public:
    FluidSim(size_t rows, size_t cols, const std::string& path)
        : rows{ rows },
          cols{ cols },
          p{ rows, cols },
          old_p{ rows, cols },
          field{ rows, cols },
          velocity{ rows, cols },
          velocity_flow{ rows, cols },
          last_use{ rows, cols },
          dirs{ rows, cols } {
        read_field(path);
    }

    void run() { // TODO dummy run function
        rho[' '] = 0.01;
        rho['.'] = 1000;
        P_t g    = 0.1;

        for (size_t x = 0; x < rows; ++x) {
            for (size_t y = 0; y < cols; ++y) {
                if (field(x, y) == '#') {
                    continue;
                }
                for (auto [dx, dy] : deltas) {
                    dirs(x, y) += (field(x + dx, y + dy) != '#');
                }
            }
        }

        for (size_t i = 0; i < T; ++i) {

            P_t total_delta_p = 0;
            // Apply external forces
            for (size_t x = 0; x < rows; ++x) {
                for (size_t y = 0; y < cols; ++y) {
                    if (field(x, y) == '#') {
                        continue;
                    }
                    if (field(x + 1, y) != '#') {
                        velocity.add(x, y, 1, 0, g);
                    }
                }
            }

            // Apply forces from p
            // std::memcpy(old_p, p, sizeof(p));
            std::copy(p.data.begin(), p.data.end(), old_p.data.begin());
            for (size_t x = 0; x < rows; ++x) {
                for (size_t y = 0; y < cols; ++y) {
                    if (field(x, y) == '#') {
                        continue;
                    }
                    for (auto [dx, dy] : deltas) {
                        int nx = x + dx, ny = y + dy;
                        if (field(nx, ny) != '#' && old_p(nx, ny) < old_p(x, y)) {
                            auto delta_p = old_p(x, y) - old_p(nx, ny);
                            auto force   = delta_p;
                            auto& contr  = velocity.get(nx, ny, -dx, -dy);
                            if (contr * rho[(int)field(nx, ny)] >= force) {
                                contr -= force / rho[(int)field(nx, ny)];
                                continue;
                            }
                            force -= contr * rho[(int)field(nx, ny)];
                            contr = 0;
                            velocity.add(x, y, dx, dy,
                                         force / rho[(int)field(x, y)]);
                            p(x, y) -= force / dirs(x, y);
                            total_delta_p -= force / dirs(x, y);
                        }
                    }
                }
            }

            // Make flow from velocities
            bool prop = false;
            do {
                UT += 2;
                prop = 0;
                for (size_t x = 0; x < rows; ++x) {
                    for (size_t y = 0; y < cols; ++y) {
                        if (field(x, y) != '#' && last_use(x, y) != UT) {
                            auto [t, local_prop, _] = propagate_flow(x, y, 1);
                            if (t > 0) {
                                prop = 1;
                            }
                        }
                    }
                }
            } while (prop);

            // Recalculate p with kinetic energy
            for (size_t x = 0; x < rows; ++x) {
                for (size_t y = 0; y < cols; ++y) {
                    if (field(x, y) == '#') {
                        continue;
                    }
                    for (auto [dx, dy] : deltas) {
                        auto old_v = velocity.get(x, y, dx, dy);
                        auto new_v = velocity_flow.get(x, y, dx, dy);
                        if (old_v > 0) {
                            assert(new_v <= old_v);
                            velocity.get(x, y, dx, dy) = new_v;
                            auto force = (old_v - new_v) * rho[(int)field(x, y)];
                            if (field(x, y) == '.') {
                                force *= 0.8;
                            }
                            if (field(x + dx, y + dy) == '#') {
                                p(x, y) += force / dirs(x, y);
                                total_delta_p += force / dirs(x, y);
                            } else {
                                p(x + dx, y + dy) += force / dirs(x + dx, y + dy);
                                total_delta_p += force / dirs(x + dx, y + dy);
                            }
                        }
                    }
                }
            }

            UT += 2;
            prop = false;
            for (size_t x = 0; x < rows; ++x) {
                for (size_t y = 0; y < cols; ++y) {
                    if (field(x, y) != '#' && last_use(x, y) != UT) {
                        if (random01() < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        } else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }

            if (prop) {
                std::cout << "Tick " << i << ":\n";
                for (size_t x = 0; x < rows; ++x) {
                    for (size_t y = 0; y < cols; ++y) {
                        std::cout << field(x, y);
                    }
                    std::cout << '\n';
                }
            }
        }
        // std::cout << "p_type: " << typeid(P_t).name() << std::endl;
        // std::cout << "v_type: " << typeid(V_t).name() << std::endl;
        // std::cout << "v_flow_type: " << typeid(V_flow_t).name() << std::endl;
        // std::cout << "size: " << rows << "x" << cols << std::endl;
        // std::cout << (is_static<Size> ? "static": "dynamic") << std::endl;
        // std::cout << "Array_data_type: " << typeid(Arr_t<P_t>(0, 0).data).name()
        // << std::endl; std::cout << std::endl;

        // for (size_t i = 0; i < rows; ++i) {
        //     for (size_t j = 0; j < cols; ++j) {
        //         std::cout << field(i, j);
        //     }
        //     std::cout << std::endl;
        // }
    }

  private:
    void read_field(const std::string& path) {
        std::ifstream file{ path };
        assert(file.is_open());
        size_t i = 0;
        size_t j = 0;
        std::for_each(std::istreambuf_iterator<char>{ file },
                      std::istreambuf_iterator<char>(), [&](const char& c) {
                          if (c == '\n') {
                              i++;
                              j = 0;
                          } else {
                              field(i, j) = c;
                              j++;
                          }
                      });
    }

    std::tuple<V_t, bool, std::pair<int, int>> propagate_flow(int x, int y,
                                                              V_t lim) {
        last_use(x, y) = UT - 1;
        V_t ret        = 0;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field(nx, ny) != '#' && last_use(nx, ny) < UT) {
                auto cap  = velocity.get(x, y, dx, dy);
                auto flow = velocity_flow.get(x, y, dx, dy);
                if (flow == cap) { // TODO потенциально может ломаться
                    continue;
                }
                // assert(v >= velocity_flow.get(x, y, dx, dy));
                auto vp = std::min(lim, cap - flow);
                if (last_use(nx, ny) == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);
                    last_use(x, y) = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " <<
                    // vp << " / " << lim << "\n";
                    return { vp, 1, { nx, ny } };
                }
                auto [t, prop, end] = propagate_flow(nx, ny, vp);
                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, t);
                    last_use(x, y) = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t
                    // << " / " << lim << "\n";
                    return { t, prop && end != std::pair(x, y), end };
                }
            }
        }
        last_use(x, y) = UT;
        return { ret, 0, { 0, 0 } };
    }

    auto random01() {
        std::uniform_real_distribution<double> dist(0, 1);
        double double_rnd = dist(rnd);
        return V_t { double_rnd };
    }

    void propagate_stop(int x, int y, bool force = false) {
        if (!force) {
            bool stop = true;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field(nx, ny) != '#' && last_use(nx, ny) < UT - 1 &&
                    velocity.get(x, y, dx, dy) > 0) {
                    stop = false;
                    break;
                }
            }
            if (!stop) {
                return;
            }
        }
        last_use(x, y) = UT;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field(nx, ny) == '#' || last_use(nx, ny) == UT ||
                velocity.get(x, y, dx, dy) > 0) {
                continue;
            }
            propagate_stop(nx, ny);
        }
    }

    auto move_prob(int x, int y) {
        V_t sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field(nx, ny) == '#' || last_use(nx, ny) == UT) {
                continue;
            }
            auto v = velocity.get(x, y, dx, dy);
            if (v < 0) {
                continue;
            }
            sum += v;
        }
        return sum;
    }

    bool propagate_move(int x, int y, bool is_first) {
        last_use(x, y) = UT - is_first;
        bool ret       = false;
        int nx = -1, ny = -1;
        do {
            std::array<V_t, deltas.size()> tres;
            V_t sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field(nx, ny) == '#' || last_use(nx, ny) == UT) {
                    tres[i] = sum;
                    continue;
                }
                auto v = velocity.get(x, y, dx, dy);
                if (v < 0) {
                    tres[i] = sum;
                    continue;
                }
                sum += v;
                tres[i] = sum;
            }

            if (sum == 0) {
                break;
            }

            V_t p    = random01() * sum;
            size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

            auto [dx, dy] = deltas[d];
            nx            = x + dx;
            ny            = y + dy;
            assert(velocity.get(x, y, dx, dy) > 0 && field(nx, ny) != '#' &&
                   last_use(nx, ny) < UT);

            ret = (last_use(nx, ny) == UT - 1 || propagate_move(nx, ny, false));
        } while (!ret);
        last_use(x, y) = UT;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field(nx, ny) != '#' && last_use(nx, ny) < UT - 1 &&
                velocity.get(x, y, dx, dy) < 0) {
                propagate_stop(nx, ny);
            }
        }
        if (ret) {
            if (!is_first) {
                swap_with(x, y);
                swap_with(nx, ny);
                swap_with(x, y);
            }
        }
        return ret;
    }

  private:
    static constexpr std::array<std::pair<int, int>, 4> deltas{
        { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } }
    };

    struct VectorField {
        VectorField(size_t rows, size_t cols)
            : v{ rows, cols } {
        }

        Arr_t<std::array<V_t, deltas.size()>> v;

        V_t& add(int x, int y, int dx, int dy, V_t dv) {
            return get(x, y, dx, dy) += dv;
        }

        V_t& get(int x, int y, int dx, int dy) {
            size_t i = std::ranges::find(deltas, std::pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v(x, y)[i];
        }
    };

    size_t rows;
    size_t cols;

    Arr_t<char> field;
    Arr_t<P_t> p;
    Arr_t<P_t> old_p;
    std::array<P_t, 256> rho;
    VectorField velocity;
    VectorField velocity_flow;
    Arr_t<int> last_use;
    int UT{};
    std::mt19937 rnd{ 1337 };
    Arr_t<int> dirs;
    static constexpr size_t T = 1'000'000;

    char type{};
    P_t cur_p{};
    std::array<V_t, deltas.size()> v{};
    void swap_with(int x, int y) {
        std::swap(field(x, y), type);
        std::swap(p(x, y), cur_p);
        std::swap(velocity.v(x, y), v);
    }
};
} // namespace Fluid