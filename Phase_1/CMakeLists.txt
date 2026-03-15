######################################################################################################
# Reference materials:
#
# CMake's Website:
# https://cmake.org/cmake/help/book/mastering-cmake/index.html
######################################################################################################
cmake_minimum_required(VERSION 3.24)    # Set minimum required CMake version.
project(Phase1)                         # Set project name as Phase1.

set(CMAKE_CXX_STANDARD 17)              # Set C++17 as the minimum required C++ version.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Enforce the C++17 minimum requirement.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # Export a list of compile commands - useful for clang / VIM.
######################################################################################################
