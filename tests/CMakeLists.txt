cmake_minimum_required(VERSION 3.10)

add_executable( simpleTest simple.cpp )
add_test(simpleTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/simpleTest )
target_link_libraries( simpleTest harmony_fsm )

add_executable( runnerTest runner.cpp )
add_test(runnerTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runnerTest )

if(ROS_TIME)
  target_link_libraries( runnerTest harmony_fsm pthread ${roscpp_LIBRARIES} )
else()
  target_link_libraries( runnerTest harmony_fsm pthread )
endif()

file(COPY rules.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
