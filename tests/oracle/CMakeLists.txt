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

# Oracle importer benchmark tests (manual, not run in CI, incl. RAM benchmark)
# Optional compression libs, mirrored from oracle/CMakeLists.txt, so the benchmark
# can download and decompress whatever AllPrintings format the default URL selects.
find_package(ZLIB)
if(ZLIB_FOUND)
  add_definitions("-DHAS_ZLIB")
  set(_ORACLE_BENCH_EXTRA_SOURCES ../../oracle/src/zip/unzip.cpp ../../oracle/src/zip/zipglobal.cpp)
  set(_ORACLE_BENCH_EXTRA_LIBRARIES ${ZLIB_LIBRARIES})
  include_directories(${ZLIB_INCLUDE_DIRS})
else()
  message(STATUS "Oracle tests: zlib not found; zip download benchmark disabled")
endif()

find_package(LibLZMA)
if(LIBLZMA_FOUND)
  add_definitions("-DHAS_LZMA")
  list(APPEND _ORACLE_BENCH_EXTRA_SOURCES ../../oracle/src/lzma/decompress.cpp)
  list(APPEND _ORACLE_BENCH_EXTRA_LIBRARIES ${LIBLZMA_LIBRARIES})
  include_directories(${LIBLZMA_INCLUDE_DIRS})
else()
  message(STATUS "Oracle tests: LibLZMA not found; xz download benchmark disabled")
endif()

add_executable(
  oracle_importer_benchmark_test
  ${VERSION_STRING_CPP} ../../oracle/src/oracleimporter.cpp ../../oracle/src/parsehelpers.cpp
  oracle_importer_benchmark_test.cpp ${_ORACLE_BENCH_EXTRA_SOURCES}
)

if(NOT GTEST_FOUND)
  add_dependencies(oracle_importer_benchmark_test gtest)
endif()

target_link_libraries(
  oracle_importer_benchmark_test
  libcockatrice_card
  libcockatrice_interfaces
  Threads::Threads
  ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
  ${_ORACLE_BENCH_EXTRA_LIBRARIES}
)
