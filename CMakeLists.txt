cmake_minimum_required(VERSION 3.10)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(harmony_fsm LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(HARMONY_FSM_MASTER_PROJECT ON)
endif()

option(
  ROS_TIME
  "Use ROS Time"
  FALSE
)

if(ROS_TIME)
  message("building with ROS timers")
  add_definitions("-DUSE_ROS_TIME")

  find_package(catkin REQUIRED COMPONENTS roscpp )
  
  catkin_package(
     INCLUDE_DIRS include
     LIBRARIES ${PROJECT_NAME}
  )

  include_directories(${roscpp_INCLUDE_DIRS})
endif()

option(
  BUILD_TESTS
  "Build tests"
  ${HARMONY_FSM_MASTER_PROJECT}
)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/finite_state_machine.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/event_table_entry.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/fsm_clocks.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/fsm_rate.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/fsm_runner.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/config_parser.hpp
)

add_library(${PROJECT_NAME} SHARED ${HEADERS})

target_include_directories( ${PROJECT_NAME} INTERFACE include)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(ROS_TIME)
  install(DIRECTORY include/${PROJECT_NAME}/
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
    PATTERN ".svn" EXCLUDE
  )
  
  install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
  )
endif()

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
    DEPENDENCIES harmony_fsm
  )
endif()
