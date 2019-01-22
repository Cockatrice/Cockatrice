ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(carddatabase_test
    carddatabase_test.cpp
    ../../cockatrice/src/carddatabase.cpp
    ../../cockatrice/src/carddbparser/carddatabaseparser.cpp
    ../../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../../cockatrice/src/carddbparser/cockatricexml4.cpp
)
if(NOT GTEST_FOUND)
    add_dependencies(carddatabase_test gtest)
endif()

find_package(Qt5 COMPONENTS Concurrent Network Widgets REQUIRED)
set(TEST_QT_MODULES Qt5::Concurrent Qt5::Network Qt5::Widgets)

target_link_libraries(carddatabase_test ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
add_test(NAME carddatabase_test COMMAND carddatabase_test)