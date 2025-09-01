# SPDX-License-Identifier: Apache-2.0
add_library(minitest INTERFACE)
target_include_directories(minitest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(unit_tests
  test_xor.cpp
)
target_link_libraries(unit_tests PRIVATE nwx minitest)
add_test(NAME unit_tests COMMAND unit_tests)

add_executable(test_metrics_ex test_metrics_ex.cpp)
target_link_libraries(test_metrics_ex PRIVATE nwx)
add_test(NAME metrics_ex COMMAND test_metrics_ex)

add_executable(test_determinism test_determinism.cpp)
target_link_libraries(test_determinism PRIVATE nwx)
add_test(NAME determinism COMMAND test_determinism)
