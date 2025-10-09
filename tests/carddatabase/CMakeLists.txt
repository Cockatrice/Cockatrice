cmake_minimum_required(VERSION 3.16)
project(CardDatabaseTests VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# ------------------------
# Definitions
# ------------------------
add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

# ------------------------
# Qt modules
# ------------------------
set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network
                    ${COCKATRICE_QT_VERSION_NAME}::Widgets ${COCKATRICE_QT_VERSION_NAME}::Svg
)

# ------------------------
# Card Database Test
# ------------------------
add_executable(carddatabase_test ${MOCKS_SOURCES} ${VERSION_STRING_CPP} carddatabase_test.cpp mocks.cpp)

target_link_libraries(
  carddatabase_test
  PRIVATE libcockatrice_card
  PRIVATE libcockatrice_settings
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME carddatabase_test COMMAND carddatabase_test)

# ------------------------
# Filter String Test
# ------------------------
add_executable(
  filter_string_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/filters/filter_card.cpp
  ../../cockatrice/src/filters/filter_string.cpp
  ../../cockatrice/src/filters/filter_tree.cpp
  filter_string_test.cpp
  mocks.cpp
)

target_link_libraries(
  filter_string_test
  PRIVATE libcockatrice_card
  PRIVATE libcockatrice_settings
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME filter_string_test COMMAND filter_string_test)

# ------------------------
# Dependencies on gtest
# ------------------------
if(NOT GTEST_FOUND)
  add_dependencies(carddatabase_test gtest)
  add_dependencies(filter_string_test gtest)
endif()
