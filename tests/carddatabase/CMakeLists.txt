cmake_minimum_required(VERSION 3.16)
project(CardDatabaseTests VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# ------------------------
# Definitions
# ------------------------
add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

# ------------------------
# Card Database Test
# ------------------------
cockatrice_add_test(
  NAME
  carddatabase_test
  SOURCES
  ${VERSION_STRING_CPP}
  carddatabase_test.cpp
  mocks.cpp
  LIBS
  libcockatrice_card
)

# ------------------------
# Filter String Test
# (guard must match the condition for libcockatrice_filters in the root CMakeLists.txt)
# ------------------------
if(WITH_ORACLE OR WITH_CLIENT)
  cockatrice_add_test(
    NAME
    filter_string_test
    SOURCES
    ${VERSION_STRING_CPP}
    filter_string_test.cpp
    mocks.cpp
    LIBS
    libcockatrice_filters
  )
endif()

# ------------------------
# Load Benchmark (manual, not run in CI)
# ------------------------
cockatrice_add_manual_test(
  NAME
  load_benchmark
  SOURCES
  ${VERSION_STRING_CPP}
  load_benchmark.cpp
  mocks.cpp
  LIBS
  libcockatrice_card
  libcockatrice_models
  MANUAL
)
