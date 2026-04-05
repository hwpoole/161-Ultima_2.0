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
set(CURSES_NEED_NCURSES TRUE)           # Set NCURSES as needed.

enable_testing()                        # Enable testing - required for ctest.

find_package(Curses REQUIRED)           # Find NCURSES package.
######################################################################################################
add_executable(
  Ultima
  src/Ultima.cpp
  src/Sched.cpp
  src/Sema.cpp
)

target_include_directories(
  Ultima
  PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${CURSES_INCLUDE_DIRS}
)

target_link_libraries (
  Ultima
  PRIVATE
  ${CURSES_LIBRARIES}
)
