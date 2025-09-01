
# SPDX-License-Identifier: Apache-2.0
# Profile use toolchain (Clang/GCC)
set(CMAKE_C_COMPILER clang CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "" FORCE)
add_compile_options(-fprofile-use -fprofile-correction)
add_link_options(-fprofile-use)
message(STATUS "PGO: profile use enabled")
