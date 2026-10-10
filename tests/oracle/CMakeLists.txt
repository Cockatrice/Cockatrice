cockatrice_add_test(NAME parse_cipt_test SOURCES ../../oracle/src/parsehelpers.cpp parse_cipt_test.cpp)

# Oracle importer unit tests
cockatrice_add_test(
  NAME
  oracle_importer_test
  SOURCES
  ${VERSION_STRING_CPP}
  ../../oracle/src/oracleimporter.cpp
  ../../oracle/src/parsehelpers.cpp
  ../../oracle/src/raw_json_scanner.cpp
  oracle_importer_test.cpp
  LIBS
  libcockatrice_card
  libcockatrice_interfaces
)

# Oracle importer benchmark tests (manual, not run in CI, incl. RAM benchmark)
# Optional compression libs, mirrored from oracle/CMakeLists.txt, so the benchmark
# can download and decompress whatever AllPrintings format the default URL selects.
# add_definitions() is directory-scoped and order-independent, so HAS_ZLIB/HAS_LZMA also
# reach the two unit tests above. That is harmless: those macros gate code only in
# oracle/src/pages.cpp and oracle/src/pagetemplates.cpp, which no test target compiles.
find_package(ZLIB)
if(ZLIB_FOUND)
  add_definitions("-DHAS_ZLIB")
  set(_ORACLE_BENCH_EXTRA_SOURCES ../../oracle/src/zip/unzip.cpp ../../oracle/src/zip/zipglobal.cpp)
  cockatrice_disable_sanitizers_for(${_ORACLE_BENCH_EXTRA_SOURCES})
  set(_ORACLE_BENCH_EXTRA_LIBRARIES ${ZLIB_LIBRARIES})
  include_directories(${ZLIB_INCLUDE_DIRS})
else()
  message(STATUS "Oracle tests: zlib not found; zip download benchmark disabled")
endif()

find_package(LibLZMA)
if(LIBLZMA_FOUND)
  add_definitions("-DHAS_LZMA")
  list(APPEND _ORACLE_BENCH_EXTRA_SOURCES ../../oracle/src/lzma/decompress.cpp)
  cockatrice_disable_sanitizers_for(../../oracle/src/lzma/decompress.cpp)
  list(APPEND _ORACLE_BENCH_EXTRA_LIBRARIES ${LIBLZMA_LIBRARIES})
  include_directories(${LIBLZMA_INCLUDE_DIRS})
else()
  message(STATUS "Oracle tests: LibLZMA not found; xz download benchmark disabled")
endif()

cockatrice_add_manual_test(
  NAME
  oracle_importer_benchmark_test
  SOURCES
  ${VERSION_STRING_CPP}
  ../../oracle/src/oracleimporter.cpp
  ../../oracle/src/parsehelpers.cpp
  ../../oracle/src/raw_json_scanner.cpp
  oracle_importer_benchmark_test.cpp
  ${_ORACLE_BENCH_EXTRA_SOURCES}
  LIBS
  libcockatrice_card
  libcockatrice_interfaces
  EXTRA_LIBS
  ${_ORACLE_BENCH_EXTRA_LIBRARIES}
  MANUAL
)
