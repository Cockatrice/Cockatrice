add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network
                    ${COCKATRICE_QT_VERSION_NAME}::Widgets ${COCKATRICE_QT_VERSION_NAME}::Svg
)

if(Qt6_FOUND)
  qt6_wrap_cpp(
    MOCKS_SOURCES ../../cockatrice/src/settings/cache_settings.h ../../cockatrice/src/settings/card_database_settings.h
  )
elseif(Qt5_FOUND)
  qt5_wrap_cpp(
    MOCKS_SOURCES ../../cockatrice/src/settings/cache_settings.h ../../cockatrice/src/settings/card_database_settings.h
  )
endif()

add_executable(
  carddatabase_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/card/card_info.cpp
  ../../cockatrice/src/card/card_relation.cpp
  ../../cockatrice/src/card/card_set.cpp
  ../../cockatrice/src/card/card_set_list.cpp
  ../../cockatrice/src/card/exact_card.cpp
  ../../cockatrice/src/card/printing_info.cpp
  ../../cockatrice/src/database/card_database.cpp
  ../../cockatrice/src/database/card_database_loader.cpp
  ../../cockatrice/src/database/card_database_querier.cpp
  ../../cockatrice/src/database/parser/card_database_parser.cpp
  ../../cockatrice/src/database/parser/cockatrice_xml_3.cpp
  ../../cockatrice/src/database/parser/cockatrice_xml_4.cpp
  ../../cockatrice/src/settings/settings_manager.cpp
  carddatabase_test.cpp
  mocks.cpp
)
add_executable(
  filter_string_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
    ../../cockatrice/src/card/card_info.cpp
    ../../cockatrice/src/card/card_relation.cpp
    ../../cockatrice/src/card/card_set.cpp
    ../../cockatrice/src/card/card_set_list.cpp
    ../../cockatrice/src/card/exact_card.cpp
    ../../cockatrice/src/card/printing_info.cpp
  ../../cockatrice/src/database/card_database.cpp
  ../../cockatrice/src/database/card_database_loader.cpp
  ../../cockatrice/src/database/card_database_querier.cpp
  ../../cockatrice/src/database/card_database_manager.cpp
  ../../cockatrice/src/database/parser/card_database_parser.cpp
  ../../cockatrice/src/database/parser/cockatrice_xml_3.cpp
  ../../cockatrice/src/database/parser/cockatrice_xml_4.cpp
  ../../cockatrice/src/filters/filter_card.cpp
  ../../cockatrice/src/filters/filter_string.cpp
  ../../cockatrice/src/filters/filter_tree.cpp
  ../../cockatrice/src/settings/settings_manager.cpp
  filter_string_test.cpp
  mocks.cpp
)
if(NOT GTEST_FOUND)
  add_dependencies(carddatabase_test gtest)
  add_dependencies(filter_string_test gtest)
endif()

target_link_libraries(carddatabase_test cockatrice_common Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(filter_string_test cockatrice_common Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_test(NAME carddatabase_test COMMAND carddatabase_test)
add_test(NAME filter_string_test COMMAND filter_string_test)
