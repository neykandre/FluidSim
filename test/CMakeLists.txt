cmake_minimum_required(VERSION 3.28)
project(FluidTest)

# Enable testing
enable_testing()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TESTSOURCES *.cpp)

add_executable(
        ${PROJECT_NAME}
        ${TESTSOURCES}
)

target_link_libraries(
        ${PROJECT_NAME}
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})