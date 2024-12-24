#include "include/FluidSim.hpp"
#include "include/Mapping.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <optional>

struct Parsed {
    enum class Type {
        READ_FIELD,
        LOAD_SAVE
    };
    Type type;
    std::string p_type;
    std::string v_type;
    std::string v_flow_type;
    std::string field_path;
    std::string load_path;
    std::optional<size_t> num_threads;
};

Parsed parse_arguments(int argc, char* argv[]) {
    try {
        Parsed parsed;
        cxxopts::Options options(
            "FluidSim", "Required parameters for the simulation (excluding help)");

        options.add_options()("h,help", "Display help")(
            "p-type", "Type of parameter p-type", cxxopts::value<std::string>())(
            "v-type", "Type of parameter v-type", cxxopts::value<std::string>())(
            "v-flow-type", "Type of parameter v-flow-type",
            cxxopts::value<std::string>())("field-path", "Path to the field",
                                           cxxopts::value<std::string>())(
            "load-path", "Path to the saved simulation",
            cxxopts::value<std::string>())("num-threads", "Number of threads",
                                           cxxopts::value<size_t>());

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
            parsed.type      = Parsed::Type::LOAD_SAVE;
            parsed.load_path = result["load-path"].as<std::string>();
        } else {
            parsed.type        = Parsed::Type::READ_FIELD;
            parsed.p_type      = result["p-type"].as<std::string>();
            parsed.v_type      = result["v-type"].as<std::string>();
            parsed.v_flow_type = result["v-flow-type"].as<std::string>();
            parsed.field_path  = result["field-path"].as<std::string>();
        }
        if (result.count("num-threads")) {
            parsed.num_threads = result["num-threads"].as<size_t>();
        }

        return parsed;

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
    auto parsed = parse_arguments(argc, argv);
    // auto parsed = Parsed{.p_type="FAST_FIXED(32,16)", .v_type="FAST_FIXED(32,16)", .v_flow_type="FAST_FIXED(32,16)", .field_path="../base_field", .num_threads=3};
    Mapper mapped;
    if (parsed.type == Parsed::Type::LOAD_SAVE) {
        mapped = Mapper{ parsed.load_path };
    } else {
        mapped = Mapper{ parsed.p_type, parsed.v_type, parsed.v_flow_type,
                         parsed.field_path };
    }

    mapped.map_instance([&]<typename SimType> {
        SimType sim(mapped.get_rows(), mapped.get_cols(),
                    parsed.num_threads.has_value() ? parsed.num_threads.value() : 1);
        if (parsed.type == Parsed::Type::LOAD_SAVE) {
            std::ifstream file(parsed.load_path);
            assert(file.is_open());
            file.ignore(std::numeric_limits<std::streamsize>::max(),
                        file.widen('\n'));
            sim.deserialize(file);
        } else {
            sim.read_field(parsed.field_path);
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

    return 0;
}
