add_executable(keyboard_navigator_test keyboard_card_navigator_test.cpp keyboard_navigator_test_stubs.cpp)

target_compile_options(keyboard_navigator_test PRIVATE --coverage)
target_link_options(keyboard_navigator_test PRIVATE --coverage)

target_include_directories(
  keyboard_navigator_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src ${CMAKE_SOURCE_DIR}/cockatrice/src/game
                                  ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/libcockatrice_interfaces
)

target_link_libraries(
  keyboard_navigator_test
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
  PRIVATE libcockatrice_settings
  PRIVATE libcockatrice_interfaces
  PRIVATE libcockatrice_protocol
  PRIVATE libcockatrice_card
  PRIVATE libcockatrice_deck_list
  PRIVATE libcockatrice_models
  PRIVATE libcockatrice_rng
  PRIVATE libcockatrice_network
  PRIVATE libcockatrice_utility
  PRIVATE Qt6::Widgets
)

add_test(NAME keyboard_navigator_test COMMAND keyboard_navigator_test)

if(NOT GTEST_FOUND)
  add_dependencies(keyboard_navigator_test gtest)
endif()
