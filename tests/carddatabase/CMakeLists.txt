add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network
                    ${COCKATRICE_QT_VERSION_NAME}::Widgets ${COCKATRICE_QT_VERSION_NAME}::Svg
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOCKS_SOURCES ../../cockatrice/src/settingscache.h ../../cockatrice/src/settings/carddatabasesettings.h)
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOCKS_SOURCES ../../cockatrice/src/settingscache.h ../../cockatrice/src/settings/carddatabasesettings.h)
endif()

add_executable(
  carddatabase_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/carddatabase.cpp
  ../../cockatrice/src/carddbparser/carddatabaseparser.cpp
  ../../cockatrice/src/carddbparser/cockatricexml3.cpp
  ../../cockatrice/src/carddbparser/cockatricexml4.cpp
  ../../cockatrice/src/settings/settingsmanager.cpp
  carddatabase_test.cpp
  mocks.cpp
)
add_executable(
  filter_string_test
  ${MOCKS_SOURCES}
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/carddatabase.cpp
  ../../cockatrice/src/carddbparser/carddatabaseparser.cpp
  ../../cockatrice/src/carddbparser/cockatricexml3.cpp
  ../../cockatrice/src/carddbparser/cockatricexml4.cpp
  ../../cockatrice/src/cardfilter.cpp
  ../../cockatrice/src/filter_string.cpp
  ../../cockatrice/src/filtertree.cpp
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
