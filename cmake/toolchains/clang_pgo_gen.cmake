
# SPDX-License-Identifier: Apache-2.0
# Profile generation toolchain (Clang/GCC)
set(CMAKE_C_COMPILER clang CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "" FORCE)
add_compile_options(-fprofile-generate)
add_link_options(-fprofile-generate)
message(STATUS "PGO: profile generation enabled")
