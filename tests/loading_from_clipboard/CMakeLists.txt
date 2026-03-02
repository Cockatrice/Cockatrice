add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(loading_from_clipboard_test clipboard_testing.cpp loading_from_clipboard_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(loading_from_clipboard_test gtest)
endif()

set(TEST_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Concurrent ${COCKATRICE_QT_VERSION_NAME}::Network
                    ${COCKATRICE_QT_VERSION_NAME}::Widgets
)

target_link_libraries(
  loading_from_clipboard_test libcockatrice_deck_list Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
add_test(NAME loading_from_clipboard_test COMMAND loading_from_clipboard_test)
