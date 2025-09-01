# SPDX-License-Identifier: Apache-2.0
# Hardening and determinism flags (opt-in from main CMakeLists via include())
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIC)
  add_link_options(-Wl,-z,relro -Wl,-z,now)
  if (ipo_supported)
    message(STATUS "IPO/LTO available; enable with -DNWX_ENABLE_LTO=ON")
  endif()
  # Determinism
  add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
  add_compile_options(-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.)
endif()
