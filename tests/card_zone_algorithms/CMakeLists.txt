add_executable(card_zone_algorithms_test card_zone_algorithms_test.cpp)

target_include_directories(card_zone_algorithms_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src/game/zones/logic)

target_link_libraries(
  card_zone_algorithms_test
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
)

add_test(NAME card_zone_algorithms_test COMMAND card_zone_algorithms_test)

if(NOT GTEST_FOUND)
  add_dependencies(card_zone_algorithms_test gtest)
endif()
