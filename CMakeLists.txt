cmake_minimum_required(VERSION 3.28)

project(Fluid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

include(FetchContent)

# set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.0
)
FetchContent_MakeAvailable(cxxopts)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# add_compile_definitions("TYPES=FAST_FIXED(32,16)")
# add_compile_definitions("SIZES=S(36,84)")

set(TYPES
    ""
    CACHE STRING "Types to precompile")
add_compile_definitions(-DTYPES=${TYPES})

set(SIZES
    ""
    CACHE STRING "Sizes to precompile")
add_compile_definitions(-DSIZES=${SIZES})

add_executable(Fluid main.cpp)
target_link_libraries(Fluid PRIVATE cxxopts nlohmann_json::nlohmann_json)