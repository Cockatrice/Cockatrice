add_executable(new_counter_id_test new_counter_id_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(new_counter_id_test gtest)
endif()

target_link_libraries(
  new_counter_id_test
  PRIVATE libcockatrice_network_server_remote
  PRIVATE libcockatrice_rng
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME new_counter_id_test COMMAND new_counter_id_test)

add_executable(counter_command_auth_test counter_command_auth_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(counter_command_auth_test gtest)
endif()

target_link_libraries(
  counter_command_auth_test
  PRIVATE libcockatrice_network_server_remote
  PRIVATE libcockatrice_rng
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME counter_command_auth_test COMMAND counter_command_auth_test)

add_executable(setup_zones_command_zone_test setup_zones_command_zone_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(setup_zones_command_zone_test gtest)
endif()

target_link_libraries(
  setup_zones_command_zone_test
  PRIVATE libcockatrice_network_server_remote
  PRIVATE libcockatrice_rng
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME setup_zones_command_zone_test COMMAND setup_zones_command_zone_test)
