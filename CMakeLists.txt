# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20)
project(NEURALWAREX VERSION 14.0.0 LANGUAGES CXX)

option(NWX_ENABLE_TESTS "Build tests" ON)
option(NWX_ENABLE_SANITIZERS "Enable ASan/UBSan (non‑MSVC)" OFF)
option(NWX_ENABLE_HARDENING "Enable extra hardening flags (non‑MSVC)" ON)
option(NWX_USE_OPENMP "Enable OpenMP parallelism if available" OFF)
option(NWX_ENABLE_LTO "Enable link-time optimization when supported" ON)
option(NWX_ENABLE_COVERAGE "Enable coverage flags (GCC/Clang)" OFF)
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(NWX_ENABLE_FUZZ "Build fuzzers with libFuzzer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(MSVC)
  add_compile_options(/W4 /permissive- /D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(NWX_ENABLE_SANITIZERS AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

if(NWX_ENABLE_HARDENING AND NOT MSVC)
  add_compile_options(-D_FORTIFY_SOURCE=2)
  add_link_options(-Wl,-z,relro,-z,now)
endif()

if(NWX_ENABLE_COVERAGE AND NOT MSVC)
  add_compile_options(--coverage)
  add_link_options(--coverage)
endif()

if(NWX_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT lto_supported OUTPUT lto_err)
  if(lto_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
endif()

if(NWX_USE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found: enabling parallelism")
    add_definitions(-DNWX_USE_OPENMP)
  else()
    message(WARNING "NWX_USE_OPENMP=ON but OpenMP not found")
  endif()
endif()

find_package(BLAS)
if(BLAS_FOUND)
  add_definitions(-DNWX_USE_CBLAS)
endif()

add_subdirectory(src)

if(NWX_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(NWX_ENABLE_FUZZ AND NOT MSVC)
  add_executable(json_fuzz fuzz/json_fuzz.cpp)
  target_link_libraries(json_fuzz PRIVATE nwx)
  target_compile_options(json_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(json_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)

  add_executable(csv_fuzz fuzz/csv_fuzz.cpp)
  target_link_libraries(csv_fuzz PRIVATE nwx)
  target_compile_options(csv_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(csv_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

# Export targets for find_package consumers
install(EXPORT NEURALWAREXTargets NAMESPACE nwx:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NEURALWAREX)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/NEURALWAREXConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NEURALWAREXConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NEURALWAREX
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/NEURALWAREXConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/NEURALWAREXConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/NEURALWAREXConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NEURALWAREX
)

include(CPack)
set(CPACK_PACKAGE_NAME "neuralwarex")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")

option(NWX_ENABLE_OPENSSL "Enable OpenSSL TLS for server (opt-in)" OFF)
if(NWX_ENABLE_OPENSSL)
  find_package(OpenSSL)
  if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found: enabling TLS in server")
    set(NWX_HAVE_OPENSSL 1)
  else()
    message(WARNING "NWX_ENABLE_OPENSSL=ON but OpenSSL not found")
  endif()
endif()

# pkg-config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/pkgconfig/neuralwarex.pc.in ${CMAKE_CURRENT_BINARY_DIR}/neuralwarex.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/neuralwarex.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

option(NWX_ENABLE_CUDA "Enable CUDA/cuBLAS backend (opt-in)" OFF)
if(NWX_ENABLE_CUDA)
  find_package(CUDAToolkit)
  if(CUDAToolkit_FOUND)
    enable_language(CUDA)
    add_library(nwx_cuda STATIC src/gpu/cuda_backend.cu)
    target_include_directories(nwx_cuda PUBLIC ${CUDAToolkit_INCLUDE_DIRS})
    target_link_libraries(nwx_cuda PUBLIC CUDA::cudart CUDA::cublas)
    target_compile_features(nwx_cuda PUBLIC cxx_std_20)
    target_compile_definitions(nwx PUBLIC NWX_HAVE_CUBLAS)
    target_link_libraries(nwx PUBLIC nwx_cuda)
  else()
    message(WARNING "NWX_ENABLE_CUDA=ON but CUDAToolkit not found; CUDA disabled")
  endif()
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/python/CMakeLists.txt)
  add_subdirectory(bindings/python)
endif()

# Security hardening for Release builds
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options("$<$<CONFIG:Release>:-D_FORTIFY_SOURCE=2>" "$<$<CONFIG:Release>:-fstack-protector-strong>" -fPIC)
  add_link_options("$<$<CONFIG:Release>:-Wl,-z,relro,-z,now>" "$<$<CONFIG:Release>:-pie>")
endif()

option(NWX_ENABLE_OPENMP "Enable OpenMP parallelism (fallback path)" ON)
if (NWX_ENABLE_OPENMP)
  find_package(OpenMP)
  if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found: enabling parallel fallback")
    target_link_libraries(nwx PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(nwx PUBLIC NWX_HAVE_OPENMP=1)
  endif()
endif()

option(NWX_ENABLE_AVX2 "Enable AVX2/FMA acceleration (float64 matmul)" ON)
if(NWX_ENABLE_AVX2)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-mavx2" HAS_AVX2_FLAG)
  check_cxx_compiler_flag("-mfma" HAS_FMA_FLAG)
  if (HAS_AVX2_FLAG)
    target_compile_options(nwx PUBLIC -mavx2)
  endif()
  if (HAS_FMA_FLAG)
    target_compile_options(nwx PUBLIC -mfma)
  endif()
  target_compile_definitions(nwx PUBLIC NWX_ENABLE_AVX2=1)
endif()

option(NWX_BUILD_FUZZERS "Build libFuzzer targets (requires Clang)" OFF)
option(NWX_SANITIZE "Enable -fsanitize=address,undefined" OFF)
if (NWX_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
if (NWX_BUILD_FUZZERS)
  add_executable(csv_fuzz src/fuzz/csv_fuzz.cpp)
  target_link_libraries(csv_fuzz PRIVATE nwx)
  target_compile_options(csv_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(csv_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

option(NWX_GZIP "Enable gzip responses if zlib is available" ON)
if (NWX_GZIP)
  find_package(ZLIB)
  if (ZLIB_FOUND)
    message(STATUS "zlib found; gzip responses enabled")
    target_link_libraries(nwx PRIVATE ZLIB::ZLIB)
    target_compile_definitions(nwx PRIVATE NWX_HAVE_ZLIB=1)
  endif()
endif()


# ---- Hardening & LTO options ----
option(NWX_HARDEN "Enable hardening compile/link flags" ON)
option(NWX_LTO "Enable interprocedural optimization/LTO if supported" OFF)

include(CheckIPOSupported)
if (NWX_LTO)
  check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_err)
  if (_ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
endif()

if (NWX_HARDEN AND NOT MSVC)
  add_compile_options(-D_FORTIFY_SOURCE=3 -fstack-protector-strong -fstack-clash-protection -D_GLIBCXX_ASSERTIONS)
  add_link_options(-Wl,-z,relro -Wl,-z,now -Wl,--as-needed)
endif()


# ---- pkg-config file ----
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nwx.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/nwx.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nwx.pc DESTINATION lib/pkgconfig)


# ---- Strip symbols on install (opt-in) ----
option(NWX_STRIP_SYMBOLS "Strip symbols on install" OFF)
if (NWX_STRIP_SYMBOLS)
  if (CMAKE_INSTALL_DO_STRIP)
  else()
    set(CMAKE_INSTALL_DO_STRIP ON)
  endif()
endif()


# ---- Export compile_commands.json ----
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Reproducible build flags (best-effort) ----
option(NWX_REPRODUCIBLE "Enable reproducible build flags (-ffile-prefix-map/-fdebug-prefix-map)" ON)
if (NWX_REPRODUCIBLE AND NOT MSVC)
  add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=. -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.)
endif()


# ---- Export and install CMake targets (if library target exists) ----
if (TARGET nwx)
  install(TARGETS nwx EXPORT nwxTargets
          ARCHIVE DESTINATION lib
          LIBRARY DESTINATION lib
          RUNTIME DESTINATION bin
          INCLUDES DESTINATION include)
  install(EXPORT nwxTargets
          NAMESPACE nwx::
          FILE nwxTargets.cmake
          DESTINATION lib/cmake/nwx)
endif()
