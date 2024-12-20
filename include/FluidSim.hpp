#pragma once

#include "Array2d.hpp"
#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <random>
#include <type_traits>

namespace Fluid {

template <size_t N, size_t K>
struct StaticSize {
    static constexpr size_t rows  = N;
    static constexpr size_t cols  = K;
    static constexpr size_t value = N * K;
};

template <typename P_t, typename V_t, typename V_flow_t,
          typename Size = StaticSize<0, 0>>
class FluidSim {

    template <typename T>
    using Arr_t = Array2d<T, Size>;

  public:
    FluidSim(size_t rows, size_t cols)
        : rows{ rows },
          cols{ cols },
          p{ rows, cols },
          old_p{ rows, cols },
          field{ rows, cols },
          velocity{ rows, cols },
          velocity_flow{ rows, cols },
          last_use{ rows, cols },
          dirs{ rows, cols },
          g{ 0.01 } {
        rho[' '] = 0.01;
        rho['.'] = 1000;
    }

    int get_tick() const {
        return tick;
    }

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

        for_each_cell([&](size_t x, size_t y) {
            if (field(x, y) == '#') {
                return;
            }
            for (auto [dx, dy] : deltas) {
                dirs(x, y) += (field(x + dx, y + dy) != '#');
            }
        });
    }

    void run() {
        auto start = std::chrono::system_clock::now();
        for (; tick < TICKS; ++tick) {

            apply_external_forces();
            apply_p_forces();
            make_flow_from_vel();
            recalc_p();
            if (make_step()) {
                std::cout << "Tick " << tick << ":\n";
                for (size_t x = 0; x < rows; ++x) {
                    for (size_t y = 0; y < cols; ++y) {
                        std::cout << field(x, y);
                    }
                    std::cout << '\n';
                }
            }

            // if (tick == 5) {
            //     break;
            // }
            // std::cout << std::endl;
        }
        auto end = std::chrono::system_clock::now();

        std::cout << "Time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                           start)
                         .count()
                  << " ms\n";
        // auto total_start = std::chrono::high_resolution_clock::now();

        // // Variables to store cumulative durations
        // size_t total_external_forces_time = 0;
        // size_t total_p_forces_time        = 0;
        // size_t total_make_flow_time       = 0;
        // size_t total_recalc_p_time        = 0;
        // size_t total_make_step_time       = 0;

        // for (; tick < TICKS; ++tick) {
        //     auto start = std::chrono::high_resolution_clock::now();

        //     // Measure apply_external_forces
        //     auto ext_start = std::chrono::high_resolution_clock::now();
        //     apply_external_forces();
        //     auto ext_end = std::chrono::high_resolution_clock::now();
        //     total_external_forces_time +=
        //         std::chrono::duration_cast<std::chrono::microseconds>(ext_end -
        //                                                               ext_start)
        //             .count();

        //     // Measure apply_p_forces
        //     auto p_start = std::chrono::high_resolution_clock::now();
        //     apply_p_forces();
        //     auto p_end = std::chrono::high_resolution_clock::now();
        //     total_p_forces_time +=
        //         std::chrono::duration_cast<std::chrono::microseconds>(p_end -
        //                                                               p_start)
        //             .count();

        //     // Measure make_flow_from_vel
        //     auto flow_start = std::chrono::high_resolution_clock::now();
        //     make_flow_from_vel();
        //     auto flow_end = std::chrono::high_resolution_clock::now();
        //     total_make_flow_time +=
        //         std::chrono::duration_cast<std::chrono::microseconds>(flow_end -
        //                                                               flow_start)
        //             .count();

        //     // Measure recalc_p
        //     auto recalc_start = std::chrono::high_resolution_clock::now();
        //     recalc_p();
        //     auto recalc_end = std::chrono::high_resolution_clock::now();
        //     total_recalc_p_time +=
        //         std::chrono::duration_cast<std::chrono::microseconds>(recalc_end -
        //                                                               recalc_start)
        //             .count();

        //     // Measure make_step
        //     auto step_start = std::chrono::high_resolution_clock::now();
        //     bool step_done  = make_step();
        //     auto step_end   = std::chrono::high_resolution_clock::now();
        //     total_make_step_time +=
        //         std::chrono::duration_cast<std::chrono::microseconds>(step_end -
        //                                                               step_start)
        //             .count();

        //     if (tick == 1000) {
        //         break;
        //     }
        // }

        // auto total_end = std::chrono::high_resolution_clock::now();

        // // Output cumulative timings
        // std::cout << "Total simulation time: "
        //           << std::chrono::duration_cast<std::chrono::milliseconds>(
        //                  total_end - total_start)
        //                  .count()
        //           << " ms\n";
        // std::cout << "apply_external_forces total time: "
        //           << total_external_forces_time << " µs\n";
        // std::cout << "apply_p_forces total time: " << total_p_forces_time
        //           << " µs\n ";
        // std::cout << "make_flow_from_vel total time: " << total_make_flow_time
        //           << " µs\n";
        // std::cout << "recalc_p total time: " << total_recalc_p_time << " µs\n";
        // std::cout << "make_step total time: " << total_make_step_time << " µs\n";
    }

    void serialize(std::ofstream& file) const {
        nlohmann::json json;

        json["tick"]          = tick;
        json["p"]             = p.data;
        json["old_p"]         = old_p.data;
        json["field"]         = field.data;
        json["velocity"]      = velocity.v.data;
        json["velocity_flow"] = velocity_flow.v.data;
        json["last_use"]      = last_use.data;
        json["dirs"]          = dirs.data;
        json["rho"]           = rho;
        json["g"]             = g;

        file << json.dump();
    }

    void deserialize(std::ifstream& file) {
        nlohmann::json json;
        file >> json;

        tick            = json["tick"].get<size_t>();
        p.data          = json["p"].get<decltype(p.data)>();
        old_p.data      = json["old_p"].get<decltype(old_p.data)>();
        field.data      = json["field"].get<decltype(field.data)>();
        velocity.v.data = json["velocity"].get<decltype(velocity.v.data)>();
        velocity_flow.v.data =
            json["velocity_flow"].get<decltype(velocity_flow.v.data)>();
        last_use.data = json["last_use"].get<decltype(last_use.data)>();
        dirs.data     = json["dirs"].get<decltype(dirs.data)>();
        rho           = json["rho"].get<decltype(rho)>();
        g             = json["g"].get<V_t>();
    }

  private:
    void apply_external_forces() {
        for_each_cell([&](size_t x, size_t y) {
            if (field(x, y) == '#') {
                return;
            }
            if (field(x + 1, y) != '#') {
                velocity.add(x, y, 1, 0, g);
            }
        });
    }

    void apply_p_forces() {
        std::copy(p.data.begin(), p.data.end(), old_p.data.begin());
        for_each_cell([&](size_t x, size_t y) {
            if (field(x, y) == '#') {
                return;
            }
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                auto&& field_cell = field(nx, ny);
                if (field_cell != '#' && old_p(nx, ny) < old_p(x, y)) {
                    auto force  = old_p(x, y) - old_p(nx, ny);
                    auto& contr = velocity.get(nx, ny, -dx, -dy);
                    if (contr * rho[(int)field_cell] >= force) {
                        contr -= force / rho[(int)field_cell];
                        continue;
                    }
                    force -= contr * rho[(int)field_cell];
                    contr = 0;
                    velocity.add(x, y, dx, dy, force / rho[(int)field(x, y)]);
                    p(x, y) -= force / dirs(x, y);
                }
            }
        });
    }

    void make_flow_from_vel() {
        velocity_flow.v.clear();
        bool prop = false;
        do {
            UT += 2;
            prop = 0;
            for_each_cell([&](size_t x, size_t y) {
                if (field(x, y) != '#' && last_use(x, y) != UT) {
                    auto [t, local_prop, _] = propagate_flow(x, y, 1);
                    if (t > 0) {
                        prop = 1;
                    }
                }
            });
        } while (prop);
    }

    void recalc_p() {
        for_each_cell([&](size_t x, size_t y) {
            if (field(x, y) == '#') {
                return;
            }
            for (auto [dx, dy] : deltas) {
                auto old_v = velocity.get(x, y, dx, dy);
                auto new_v = velocity_flow.get(x, y, dx, dy);
                if (old_v > 0) {
                    assert(!(new_v > old_v));
                    velocity.get(x, y, dx, dy) = static_cast<V_t>(new_v);
                    auto force = (old_v - new_v) * rho[(int)field(x, y)];
                    if (field(x, y) == '.') {
                        force *= 0.8;
                    }
                    if (field(x + dx, y + dy) == '#') {
                        p(x, y) += force / dirs(x, y);
                    } else {
                        p(x + dx, y + dy) += force / dirs(x + dx, y + dy);
                    }
                }
            }
        });
    }

    bool make_step() {
        UT += 2;
        bool prop = false;
        for_each_cell([&](size_t x, size_t y) {
            if (field(x, y) != '#' && last_use(x, y) != UT) {
                if (random01() < move_prob(x, y)) {
                    prop = true;
                    propagate_move(x, y, true);
                } else {
                    propagate_stop(x, y, true);
                }
            }
        });
        return prop;
    }

    std::tuple<V_t, bool, std::pair<int, int>> propagate_flow(int x, int y,
                                                              V_t lim) {

        last_use(x, y) = UT - 1;

        V_t ret = 0;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field(nx, ny) != '#' && last_use(nx, ny) < UT) {
                auto cap  = velocity.get(x, y, dx, dy);
                auto flow = velocity_flow.get(x, y, dx, dy);
                if (flow == cap) {
                    continue;
                }
                auto vp = std::min(lim, static_cast<V_t>(cap - flow));
                if (last_use(nx, ny) == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);

                    last_use(x, y) = UT;
                    return { vp, 1, { nx, ny } };
                }

                auto [t, prop, end] = propagate_flow(nx, ny, vp);

                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, t);

                    last_use(x, y) = UT;

                    return { t, prop && end != std::make_pair(x, y), end };
                }
            }
        }
        last_use(x, y) = UT;

        return { ret, 0, { 0, 0 } };
    }

    inline auto random01() {
        if constexpr (std::is_floating_point_v<V_t>) {
            return V_t{ dist(rnd) };
        } else {
            return V_t::random01(rnd());
        }
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

    void for_each_cell(auto&& func) {
        for (size_t x = 1; x < rows - 1; ++x) {
            for (size_t y = 1; y < cols - 1; ++y) {
                func(x, y);
            }
        }
    }

  private:
    static constexpr std::array<std::pair<int, int>, 4> deltas{
        { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } }
    };

    template <typename T>
    struct VectorField {
        VectorField(size_t rows, size_t cols)
            : v{ rows, cols } {
        }

        Arr_t<std::array<T, deltas.size()>> v;

        T& add(int x, int y, int dx, int dy, auto dv) {
            return get(x, y, dx, dy) += dv;
        }

        T& get(int x, int y, int dx, int dy) {
            // for (int i = 0; i < deltas.size(); ++i) {
            //     if (deltas[i].first == dx && deltas[i].second == dy) return
            //     v(x, y)[i];
            // }
            // assert(false && "Invalid direction");
            size_t i =
                std::ranges::find(deltas, std::make_pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v(x, y)[i];
        }
    };

    size_t rows;
    size_t cols;

    size_t tick{};
    Arr_t<char> field;
    Arr_t<P_t> p;
    Arr_t<P_t> old_p;
    std::array<P_t, 256> rho{};
    VectorField<V_t> velocity;
    VectorField<V_flow_t> velocity_flow;
    Arr_t<int> last_use;
    int UT{};
    std::mt19937 rnd{ 1337 };
    std::uniform_real_distribution<float> dist{ 0, 1 };
    Arr_t<int> dirs;
    static constexpr size_t TICKS = 1'000'000;
    V_t g;

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