add_executable(deck_list_zones_test deck_list_zones_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(deck_list_zones_test gtest)
endif()

target_link_libraries(
  deck_list_zones_test libcockatrice_deck_list Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
add_test(NAME deck_list_zones_test COMMAND deck_list_zones_test)
