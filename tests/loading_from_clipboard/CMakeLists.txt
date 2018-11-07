ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(loading_from_clipboard_test
        loading_from_clipboard_test.cpp
        clipboard_testing.cpp
        ../../common/decklist.cpp
        )

if(NOT GTEST_FOUND)
    add_dependencies(loading_from_clipboard_test gtest)
endif()

find_package(Qt5 COMPONENTS Concurrent Network Widgets REQUIRED)
set(TEST_QT_MODULES Qt5::Concurrent Qt5::Network Qt5::Widgets)

target_link_libraries(loading_from_clipboard_test cockatrice_common ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
add_test(NAME loading_from_clipboard_test COMMAND loading_from_clipboard_test)
