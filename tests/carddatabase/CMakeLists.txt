add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network
                    ${COCKATRICE_QT_VERSION_NAME}::Widgets ${COCKATRICE_QT_VERSION_NAME}::Svg
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOCKS_SOURCES ../../cockatrice/src/settings/cache.h ../../cockatrice/src/settings/carddatabasesettings.h)
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOCKS_SOURCES ../../cockatrice/src/settings/cache.h ../../cockatrice/src/settings/carddatabasesettings.h)
endif()

add_executable(
  carddatabase_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
        ../../cockatrice/src/game/cards/database.cpp
        ../../cockatrice/src/carddbparser/card_database_parser.cpp
        ../../cockatrice/src/carddbparser/cockatrice_xml3.cpp
        ../../cockatrice/src/carddbparser/cockatrice_xml4.cpp
  ../../cockatrice/src/settings/settingsmanager.cpp
  carddatabase_test.cpp
  mocks.cpp
)
add_executable(
  filter_string_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
        ../../cockatrice/src/game/cards/database.cpp
        ../../cockatrice/src/carddbparser/card_database_parser.cpp
        ../../cockatrice/src/carddbparser/cockatrice_xml3.cpp
        ../../cockatrice/src/carddbparser/cockatrice_xml4.cpp
        ../../cockatrice/src/game/filter/card.cpp
        ../../cockatrice/src/game/filter/string.cpp
        ../../cockatrice/src/game/filter/tree.cpp
  ../../cockatrice/src/settings/settingsmanager.cpp
  filter_string_test.cpp
  mocks.cpp
)
if(NOT GTEST_FOUND)
  add_dependencies(carddatabase_test gtest)
  add_dependencies(filter_string_test gtest)
endif()

target_link_libraries(carddatabase_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(filter_string_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_test(NAME carddatabase_test COMMAND carddatabase_test)
add_test(NAME filter_string_test COMMAND filter_string_test)
