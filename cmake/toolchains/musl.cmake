# SPDX-License-Identifier: Apache-2.0
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER musl-gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXE_LINKER_FLAGS "-static")
