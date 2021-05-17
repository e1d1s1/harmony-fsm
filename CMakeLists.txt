cmake_minimum_required(VERSION 3.6)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(harmony-fsm LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(HARMONY_FSM_MASTER_PROJECT ON)
endif()

add_library(harmony-fsm INTERFACE)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/finite_state_machine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/event_table_entry.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/fsm_clocks.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/fsm_rate.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/fsm_runner.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/harmony-fsm/config_parser.hpp
)

target_include_directories( harmony-fsm INTERFACE include)

option(
  ROS_TIME
  "Use ROS Time"
  FALSE
)

if(ROS_TIME)
  add_definitions("-DUSE_ROS_TIME")

  find_package( roscpp REQUIRED )

  include_directories(${roscpp_INCLUDE_DIRS})
endif()

option(
  BUILD_TESTS
  "Build tests"
  ${HARMONY_FSM_MASTER_PROJECT}
)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

option(CODE_COVERAGE
  "Build targets with code coverage instrumentation (requires lcov)"
  OFF
)
if (CODE_COVERAGE)
  include (CodeCoverage)
  append_coverage_compiler_flags()
  setup_target_for_coverage_lcov(
    NAME coverage
    EXECUTABLE ctest -j ${PROCESSOR_COUNT}
    DEPENDENCIES harmony-fsm
  )
endif()
