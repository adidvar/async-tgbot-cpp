file(GLOB_RECURSE TEST_SRC_LIST "./*.cpp")

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories("${PROJECT_SOURCE_DIR}/test")
add_executable(${PROJECT_NAME}_test ${TEST_SRC_LIST})
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} Boost::unit_test_framework)
add_test(${PROJECT_NAME}_test ${PROJECT_NAME}_test)