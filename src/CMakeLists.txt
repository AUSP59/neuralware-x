# SPDX-License-Identifier: Apache-2.0
add_library(nwx STATIC
  core/json.cpp
  core/logging.cpp
  core/tensor.cpp
  core/model.cpp
  core/trainer.cpp
  core/metrics.cpp
  core/optim.cpp
  core/scaler.cpp
  core/split.cpp
  core/activation.cpp
  io/bundle.cpp
  core/version.cpp
  io/csv.cpp
  io/serialize.cpp
)

target_include_directories(nwx PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(nwx PROPERTIES OUTPUT_NAME "nwx")

add_executable(nwx_app app/main.cpp)
target_link_libraries(nwx_app PRIVATE nwx)

install(TARGETS nwx nwx_app
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Link OpenMP if available
if(OpenMP_CXX_FOUND)
  target_link_libraries(nwx PUBLIC OpenMP::OpenMP_CXX)
endif()

# Link BLAS if available
if(BLAS_FOUND)
  target_link_libraries(nwx PUBLIC BLAS::BLAS)
endif()

  core/stratified.cpp
  core/metrics_ex.cpp

add_executable(nwx_serve app/serve.cpp)
target_link_libraries(nwx_serve PRIVATE nwx)
install(TARGETS nwx_serve RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_eval app/eval.cpp)
target_link_libraries(nwx_eval PRIVATE nwx)
install(TARGETS nwx_eval RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Hardened server target (TLS optional)
add_executable(nwx_serve app/serve.cpp)
target_link_libraries(nwx_serve PRIVATE nwx)
if(OPENSSL_FOUND)
  target_link_libraries(nwx_serve PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  target_compile_definitions(nwx_serve PRIVATE NWX_HAVE_OPENSSL=1)
endif()
install(TARGETS nwx_serve RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_check app/check.cpp)
target_link_libraries(nwx_check PRIVATE nwx)
install(TARGETS nwx_check RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_bench app/bench.cpp)
target_link_libraries(nwx_bench PRIVATE nwx)
install(TARGETS nwx_bench RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_cv app/cv.cpp)
target_link_libraries(nwx_cv PRIVATE nwx)
install(TARGETS nwx_cv RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_importance app/importance.cpp)
target_link_libraries(nwx_importance PRIVATE nwx)
install(TARGETS nwx_importance RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_report app/report.cpp)
target_link_libraries(nwx_report PRIVATE nwx)
install(TARGETS nwx_report RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_fairness app/fairness.cpp)
target_link_libraries(nwx_fairness PRIVATE nwx)
install(TARGETS nwx_fairness RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_drift app/drift.cpp)
target_link_libraries(nwx_drift PRIVATE nwx)
install(TARGETS nwx_drift RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_impute app/impute.cpp)
target_link_libraries(nwx_impute PRIVATE nwx)
install(TARGETS nwx_impute RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
add_executable(nwx_confusion app/confusion.cpp)
target_link_libraries(nwx_confusion PRIVATE nwx)
install(TARGETS nwx_confusion RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_prof app/prof.cpp)
target_link_libraries(nwx_prof PRIVATE nwx)
install(TARGETS nwx_prof RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(nwx_explain app/explain.cpp)
target_link_libraries(nwx_explain PRIVATE nwx)
install(TARGETS nwx_explain RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_library(nwx_capi SHARED capi/capi.cpp)
target_link_libraries(nwx_capi PRIVATE nwx)
target_include_directories(nwx_capi PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_target_properties(nwx_capi PROPERTIES OUTPUT_NAME "nwx_capi")
install(TARGETS nwx_capi LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_executable(nwx_encrypt app/encrypt.cpp)
target_link_libraries(nwx_encrypt PRIVATE nwx)
if (OPENSSL_FOUND)
  target_compile_definitions(nwx_encrypt PRIVATE NWX_HAVE_OPENSSL)
  target_link_libraries(nwx_encrypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
install(TARGETS nwx_encrypt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

option(NWX_ENABLE_GRPC "Build gRPC server (requires gRPC & Protobuf)" OFF)
if (NWX_ENABLE_GRPC)
  find_package(Protobuf REQUIRED)
  find_package(gRPC REQUIRED)
  set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
  file(MAKE_DIRECTORY ${GENERATED_DIR})
  set(PROTO ${PROJECT_SOURCE_DIR}/proto/nwx.proto)
  protobuf_generate_cpp(PROTO_CPP PROTO_H ${PROTO} OUT_VAR PROTO_GEN_FILES PROTOC_OUT_DIR ${GENERATED_DIR})
  # Try to use gRPC helpers if available; otherwise fall back to custom command
  if (COMMAND grpc_generate_cpp)
    grpc_generate_cpp(GRPC_CPP GRPC_H ${PROTO} GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PROTOC_OUT_DIR ${GENERATED_DIR})
  else()
    message(STATUS "grpc_generate_cpp() not available; please generate with protoc and set NWX_ENABLE_GRPC=OFF")
  endif()
  add_executable(nwx_grpc_serve grpc/serve_grpc.cpp ${PROTO_CPP} ${GRPC_CPP})
  target_include_directories(nwx_grpc_serve PRIVATE ${GENERATED_DIR})
  target_link_libraries(nwx_grpc_serve PRIVATE nwx gRPC::grpc++ protobuf::libprotobuf)
  target_compile_definitions(nwx_grpc_serve PRIVATE NWX_ENABLE_GRPC)
  install(TARGETS nwx_grpc_serve RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
