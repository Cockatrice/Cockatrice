add_executable(reverse_card_move_test reverse_card_move_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(reverse_card_move_test gtest)
endif()

target_link_libraries(
  reverse_card_move_test
  PRIVATE libcockatrice_network_server_remote
  PRIVATE libcockatrice_rng
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME reverse_card_move_test COMMAND reverse_card_move_test)
