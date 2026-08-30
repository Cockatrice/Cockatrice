add_executable(parse_cipt_test ../../oracle/src/parsehelpers.cpp parse_cipt_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(parse_cipt_test gtest)
endif()

target_link_libraries(parse_cipt_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_test(NAME parse_cipt_test COMMAND parse_cipt_test)

# Oracle importer unit tests
add_executable(
  oracle_importer_test ${VERSION_STRING_CPP} ../../oracle/src/oracleimporter.cpp ../../oracle/src/parsehelpers.cpp
                       oracle_importer_test.cpp
)

if(NOT GTEST_FOUND)
  add_dependencies(oracle_importer_test gtest)
endif()

target_link_libraries(
  oracle_importer_test libcockatrice_card libcockatrice_interfaces Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)

add_test(NAME oracle_importer_test COMMAND oracle_importer_test)

# Oracle importer benchmark tests (manual, not run in CI)
add_executable(
  oracle_importer_benchmark_test ${VERSION_STRING_CPP} ../../oracle/src/oracleimporter.cpp
                                 ../../oracle/src/parsehelpers.cpp oracle_importer_benchmark_test.cpp
)

if(NOT GTEST_FOUND)
  add_dependencies(oracle_importer_benchmark_test gtest)
endif()

target_link_libraries(
  oracle_importer_benchmark_test libcockatrice_card libcockatrice_interfaces Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
