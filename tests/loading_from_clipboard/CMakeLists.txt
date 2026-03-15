add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(loading_from_clipboard_test ${VERSION_STRING_CPP} clipboard_testing.cpp loading_from_clipboard_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(loading_from_clipboard_test gtest)
endif()

target_link_libraries(
  loading_from_clipboard_test libcockatrice_deck_list libcockatrice_card Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
add_test(NAME loading_from_clipboard_test COMMAND loading_from_clipboard_test)
