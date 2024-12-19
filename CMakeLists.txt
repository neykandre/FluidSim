cmake_minimum_required(VERSION 3.28)

project(Fluid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем FetchContent
include(FetchContent)

# Скачиваем cxxopts
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.0
)
FetchContent_MakeAvailable(cxxopts)

# add_compile_definitions("TYPES=DOUBLE,FLOAT,FIXED(16,10),FAST_FIXED(18,202), FAST_FIXED(18,20), FIXED(32, 32), FIXED(64, 32), FIXED(32, 16)")
add_compile_definitions("TYPES=DOUBLE")
add_compile_definitions("SIZES=S(36, 84)")
# add_compile_definitions("SIZES=S(1920,1080),S(10,10),S(42,1337)")

add_executable(Fluid main.cpp)
target_link_libraries(Fluid PRIVATE cxxopts)
# add_subdirectory(test)