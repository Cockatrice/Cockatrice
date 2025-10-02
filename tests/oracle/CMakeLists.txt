add_executable(parse_cipt_test ../../oracle/src/parsehelpers.cpp parse_cipt_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(parse_cipt_test gtest)
endif()

set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Widgets)

target_link_libraries(parse_cipt_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_test(NAME parse_cipt_test COMMAND parse_cipt_test)
