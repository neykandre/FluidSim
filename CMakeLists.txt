cmake_minimum_required(VERSION 3.28)

project(Fluid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.0
)
FetchContent_MakeAvailable(cxxopts)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2 # Укажите актуальную версию
)
FetchContent_MakeAvailable(nlohmann_json)

add_compile_definitions("TYPES=FIXED(64,8), FAST_FIXED(32,16), FLOAT")
add_compile_definitions("SIZES=S(36,84)")

add_executable(Fluid main.cpp)
target_link_libraries(Fluid PRIVATE cxxopts nlohmann_json::nlohmann_json)