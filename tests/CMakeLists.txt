add_executable( simpleTest simple.cpp )
add_test(simpleTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/simpleTest )
target_link_libraries( simpleTest harmony-fsm )

add_executable( runnerTest runner.cpp )
add_test(runnerTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runnerTest )
target_link_libraries( runnerTest harmony-fsm pthread )
