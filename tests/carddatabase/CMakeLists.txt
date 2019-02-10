ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(carddatabase_test
    carddatabase_test.cpp
    mocks.cpp
    ../../cockatrice/src/carddatabase.cpp
    ../../cockatrice/src/carddbparser/carddatabaseparser.cpp
    ../../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../../cockatrice/src/carddbparser/cockatricexml4.cpp
)
add_executable(filter_string_test
    filter_string_test.cpp
    mocks.cpp
    ../../cockatrice/src/filter_string.cpp
    ../../cockatrice/src/cardfilter.cpp
    ../../cockatrice/src/filtertree.cpp
    ../../cockatrice/src/carddatabase.cpp
    ../../cockatrice/src/carddbparser/carddatabaseparser.cpp
    ../../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../../cockatrice/src/carddbparser/cockatricexml4.cpp
)
if(NOT GTEST_FOUND)
    add_dependencies(carddatabase_test gtest)
    add_dependencies(filter_string_test gtest)
endif()

find_package(Qt5 COMPONENTS Concurrent Network Widgets Svg REQUIRED)
set(TEST_QT_MODULES Qt5::Concurrent Qt5::Network Qt5::Widgets Qt5::Svg)

target_link_libraries(carddatabase_test ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(filter_string_test ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})


add_test(NAME carddatabase_test COMMAND carddatabase_test)
add_test(NAME filter_string_test COMMAND filter_string_test)