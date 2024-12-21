#include "include/FluidSim.hpp"
#include "include/Mapping.hpp"
#include <array>
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <string_view>

std::variant<std::array<std::string, 4>, std::string> parse_arguments(int argc,
                                                                      char* argv[]) {
    try {
        cxxopts::Options options(
            "FluidSim", "Required parameters for the simulation (excluding help)");
        options.add_options()("h,help", "Display help")(
            "p-type", "Type of parameter p-type", cxxopts::value<std::string>())(
            "v-type", "Type of parameter v-type", cxxopts::value<std::string>())(
            "v-flow-type", "Type of parameter v-flow-type",
            cxxopts::value<std::string>())("field-path", "Path to the field",
                                           cxxopts::value<std::string>())(
            "load-path", "Path to the saved simulation",
            cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << '\n';
            std::exit(0);
        }

        if (!(result.count("load-path") ^
              (result.count("p-type") && result.count("v-type") &&
               result.count("v-flow-type") && result.count("field-path")))) {
            throw std::runtime_error(
                "Error: Either load-path or all parameters (--p-type, "
                "--v-type, --v-flow-type, --field-path) must be provided.");
        }

        if (result.count("load-path")) {
            return result["load-path"].as<std::string>();
        } else {
            return std::array<std::string, 4>{
                result["p-type"].as<std::string>(),
                result["v-type"].as<std::string>(),
                result["v-flow-type"].as<std::string>(),
                result["field-path"].as<std::string>()
            };
        }

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Parsing error: " << e.what() << '\n';
        std::exit(1);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::exit(1);
    }
}

static std::function<void(int)> shutdown_handler;
static void signal_handler(int signal) {
    shutdown_handler(signal);
}

int main(int argc, char** argv) {
    // Fluid::Fixed<32, 16> a(0.1);
    // Fluid::Fixed<64, 8> b(0.05);
    // std::cout << (a < 0.01) << std::endl;
    auto parsed = parse_arguments(argc, argv);
    Mapper mapped;
    if (std::holds_alternative<std::string>(parsed)) {
        mapped = Mapper{std::get<1>(parsed)};
    } else {
        auto [p_type, v_type, v_flow_type, path] = std::get<0>(parsed);
        mapped = Mapper{ p_type, v_type, v_flow_type, path };
    }

    mapped.map_instance([&]<typename SimType> {
        SimType sim(mapped.get_rows(), mapped.get_cols());
        if (std::holds_alternative<std::string>(parsed)) {
            std::ifstream file(std::get<1>(parsed));
            assert(file.is_open());
            file.ignore(std::numeric_limits<std::streamsize>::max(),
                        file.widen('\n'));
            sim.deserialize(file);
        } else {
            sim.read_field(std::get<0>(parsed)[3]);
        }

        std::signal(SIGINT, signal_handler);

        shutdown_handler = [&](int signal) {
            if (signal == SIGINT) {
                std::ofstream file("save_" + std::to_string(sim.get_tick()));
                assert(file.is_open());

                file << mapped.get_p_type() << " " << mapped.get_v_type() << " "
                     << mapped.get_v_flow_type() << " " << mapped.get_rows() << " "
                     << mapped.get_cols() << std::endl;
                sim.serialize(file);
                std::cout << "Simulation saved to "
                          << "save_" << sim.get_tick() << std::endl;
                sleep(1);
            }
        };

        sim.run();
    });
    // auto p_type      = "FIXED(64,8)";
    // auto v_type      = "FIXED(64,8)";
    // auto v_flow_type = "FIXED(64,8)";
    // auto path        = "../base_field";

    return 0;
}
