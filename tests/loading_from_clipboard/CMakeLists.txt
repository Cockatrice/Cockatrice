ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(loading_from_clipboard_test
        loading_from_clipboard_test.cpp
        clipboard_testing.cpp
        ../../common/decklist.cpp
        )

if(NOT GTEST_FOUND)
    add_dependencies(loading_from_clipboard_test gtest)
endif()

SET(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network ${COCKATRICE_QT_VERSION_NAME}::Widgets)

target_link_libraries(loading_from_clipboard_test cockatrice_common Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
add_test(NAME loading_from_clipboard_test COMMAND loading_from_clipboard_test)
