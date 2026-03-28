######################################################################################################
# Reference materials:
#
# CMake's Website:
# https://cmake.org/cmake/help/book/mastering-cmake/index.html
######################################################################################################
cmake_minimum_required(VERSION 3.24)    # Set minimum required CMake version.
project(Phase1)                         # Set project name as Phase1.

set(CMAKE_CXX_STANDARD 20)              # Set C++20 as the minimum required C++ version.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Enforce the C++20 minimum requirement.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # Export a list of compile commands - useful for clang / VIM.

enable_testing()                        # Enable testing - required for ctest.
######################################################################################################
include(FetchContent)                   # Include the FetchContent CMake module.
include(GoogleTest)                     # Include GoogleTest.
######################################################################################################
FetchContent_Declare(                   # Declare the content to be fetched, and from where.
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)  # Fech GoogleTest and make it available.
######################################################################################################
add_executable(                         # Declare the executable to be added, and from where.
  SmokeTest
  test/SmokeTest.cpp
)

target_link_libraries(                  # Link <file> with <content / library>.
  SmokeTest
  PRIVATE
  gtest
  GTest::gtest_main
)

gtest_discover_tests(SmokeTest)         # Call GoogleTest to discover tests in SmokeTest.
