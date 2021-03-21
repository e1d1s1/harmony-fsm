add_executable( simpleTest simple.cpp )
add_test(simpleTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/simpleTest )
target_link_libraries( simpleTest harmony-fsm )

add_executable( switchRunnerTest switch_runner.cpp )
add_test(switchRunnerTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/simpleTest )
target_link_libraries( switchRunnerTest harmony-fsm pthread )

add_executable( functionalRunnerTest functional_runner.cpp )
add_test(functionalRunnerTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/simpleTest )
target_link_libraries( functionalRunnerTest harmony-fsm )
