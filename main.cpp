#include "include/FluidSim.hpp"
#include "include/Mapping.hpp"
#include <array>
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <string_view>

std::tuple<std::string, std::string, std::string, std::string> parse_arguments(int argc,
                                                                  char* argv[]) {
    try {
        cxxopts::Options options(
            "FluidSim", "Required parameters for the simulation (excluding help)");
        options.add_options()("h,help", "Display help")(
            "p-type", "Type of parameter p-type", cxxopts::value<std::string>())(
            "v-type", "Type of parameter v-type", cxxopts::value<std::string>())(
            "v-flow-type", "Type of parameter v-flow-type",
            cxxopts::value<std::string>())("field-path", "Path to the field",
                                           cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << '\n';
            std::exit(0);
        }

        if (!result.count("p-type") || !result.count("v-type") ||
            !result.count("v-flow-type") || !result.count("field-path")) {
            throw std::runtime_error("Error: All parameters (--p-type, --v-type, "
                                     "--v-flow-type, --field-path) must be provided.");
        }

        return { result["p-type"].as<std::string>(),
                 result["v-type"].as<std::string>(),
                 result["v-flow-type"].as<std::string>(),
                 result["field-path"].as<std::string>() };

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Parsing error: " << e.what() << '\n';
        std::exit(1);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::exit(1);
    }
}

int main(int argc, char** argv) {
    // Fluid::Fixed<32, 16> a(0.1);
    // Fluid::Fixed<64, 8> b(0.05);
    // std::cout << (a < 0.01) << std::endl;
    auto [p_type, v_type, v_flow_type, path] = parse_arguments(argc, argv);

    Mapping::run_sim(p_type, v_type, v_flow_type, path);

    return 0;
}
