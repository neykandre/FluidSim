cmake_minimum_required(VERSION 3.28)

project(Fluid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions("TYPES=DOUBLE,FLOAT,FIXED(10,10),FAST_FIXED(18,202), FAST_FIXED(18,20), FIXED(32, 32), FIXED(16, 16), FIXED(32, 16)")

add_executable(Fluid main.cpp)
add_subdirectory(test)