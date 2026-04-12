######################################################################################################
# Reference materials:
#
# CMake's Website:
# https://cmake.org/cmake/help/book/mastering-cmake/index.html
######################################################################################################
cmake_minimum_required(VERSION 3.24)    # Set minimum required CMake version.
project(161-Ultima-2.0)                 # Set project name as 161-Ultima 2.0

set(CMAKE_CXX_STANDARD 20)              # Set C++20 as the minimum required C++ version.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Enforce the C++20 minimum requirement.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # Export a list of compile commands - useful for clang / VIM.
set(CURSES_NEED_NCURSES TRUE)           # Set NCURSES as needed.

#enable_testing()                        # Enable testing - required for ctest.

#include(FetchContent)                   # Include the FetchContent CMake feature.
#include(GoogleTest)                     # Include GoogleTest.

find_package(Curses REQUIRED)           # Find NCURSES package.
######################################################################################################
#FetchContent_Declare(                   # Define the content that is needed, and where to get it from.
#  GoogleTest
#  GIT_REPOSITORY https://github.com/google/googletest.git
#  GIT_TAG v1.17.0
#  FIND_PACKAGE_ARGS NAMES GTest, gtest  # Try to find GoogleTest locally before fetching it from GH.
#)

#FetchContent_MakeAvailable(GoogleTest)  # Fetch and make GoogleTest available.
######################################################################################################
add_executable(                         # 
  Ultima
  src/Ultima.cpp
  src/Sched.cpp
  src/Sema.cpp
  src/Kernel.cpp
  src/Uthread.cpp
  src/Pipe.cpp
)

target_include_directories(
  Ultima
  PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(
  Ultima
  PRIVATE
  ${CURSES_LIBRARIES}
)
######################################################################################################
#add_executable(
#  KernelTest
#  test/KernelTest.cpp
#)

#target_include_directories(
#  KernelTest
#  PRIVATE
#  "${CMAKE_SOURCE_DIR}/src"
#)

#target_link_libraries(
#  KernelTest
#  PRIVATE
#  gtest
#  GTest::gtest_main
#  GTest::gmock
#  GTest::gmock_main
#)

#gtest_discover_tests(KernelTest)
