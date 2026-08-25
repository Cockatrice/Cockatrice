# NOTE: Qt modules for tests are defined centrally in cmake/FindQtRuntime.cmake (the _TEST_NEEDED variable).
# If a new test needs additional Qt modules, add them there — not in individual test CMakeLists.txt files.
enable_testing()

add_test(NAME dummy_test COMMAND dummy_test)
add_test(NAME expression_test COMMAND expression_test)
add_test(NAME clamped_arithmetic_test COMMAND clamped_arithmetic_test)
add_test(NAME test_age_formatting COMMAND test_age_formatting)
add_test(NAME password_hash_test COMMAND password_hash_test)
add_test(NAME playmat_resolver_test COMMAND playmat_resolver_test)
add_test(NAME server_card_counter_test COMMAND server_card_counter_test)
add_test(NAME server_counter_test COMMAND server_counter_test)
add_test(NAME server_rate_limiter_test COMMAND server_rate_limiter_test)
add_test(NAME server_developer_role_test COMMAND server_developer_role_test)
add_test(NAME warning_categories_test COMMAND warning_categories_test)
add_test(NAME lag_monitor_test COMMAND lag_monitor_test)
add_test(NAME latency_tracker_test COMMAND latency_tracker_test)
add_test(NAME metrics_registry_test COMMAND metrics_registry_test)

add_test(NAME deck_hash_performance_test COMMAND deck_hash_performance_test)
set_tests_properties(deck_hash_performance_test PROPERTIES TIMEOUT 15)

# Find GTest

add_executable(dummy_test dummy_test.cpp)
add_executable(expression_test expression_test.cpp)
add_executable(clamped_arithmetic_test clamped_arithmetic_test.cpp)
add_executable(test_age_formatting test_age_formatting.cpp)
add_executable(password_hash_test password_hash_test.cpp)
add_executable(playmat_resolver_test playmat_resolver_test.cpp)
add_executable(deck_hash_performance_test deck_hash_performance_test.cpp)
add_executable(server_card_counter_test server_card_counter_test.cpp)
add_executable(server_counter_test server_counter_test.cpp)
add_executable(server_rate_limiter_test server_rate_limiter_test.cpp)
add_executable(server_developer_role_test server_developer_role_test.cpp)
add_executable(warning_categories_test warning_categories_test.cpp)
add_executable(lag_monitor_test ${CMAKE_SOURCE_DIR}/cockatrice/src/client/lag_monitor.cpp lag_monitor_test.cpp)
target_include_directories(lag_monitor_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)
add_executable(latency_tracker_test latency_tracker_test.cpp)
add_executable(metrics_registry_test ../servatrice/src/metrics_registry.cpp metrics_registry_test.cpp)

find_package(GTest)

if(NOT GTEST_FOUND)
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/gtest-build")
    message(STATUS "Downloading googletest")
    configure_file(
      "${CMAKE_SOURCE_DIR}/cmake/gtest-CMakeLists.txt.in" "${CMAKE_BINARY_DIR}/gtest-download/CMakeLists.txt"
    )
    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download
    )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download)
  else()
    message(STATUS "GoogleTest directory exists")
  endif()

  # Add gtest directly to our build
  add_subdirectory(${CMAKE_BINARY_DIR}/gtest-src ${CMAKE_BINARY_DIR}/gtest-build EXCLUDE_FROM_ALL)

  # Add the gtest include directory, since gtest
  # doesn't add that dependency to its gtest target
  target_include_directories(gtest INTERFACE "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gtest-src/include>")

  set(GTEST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/gtest-src/include")
  set(GTEST_BOTH_LIBRARIES gtest)
  add_dependencies(dummy_test gtest)
  add_dependencies(expression_test gtest)
  add_dependencies(clamped_arithmetic_test gtest)
  add_dependencies(test_age_formatting gtest)
  add_dependencies(password_hash_test gtest)
  add_dependencies(playmat_resolver_test gtest)
  add_dependencies(deck_hash_performance_test gtest)
  add_dependencies(server_card_counter_test gtest)
  add_dependencies(server_counter_test gtest)
  add_dependencies(server_rate_limiter_test gtest)
  add_dependencies(server_developer_role_test gtest)
  add_dependencies(warning_categories_test gtest)
  add_dependencies(lag_monitor_test gtest)
  add_dependencies(latency_tracker_test gtest)
  add_dependencies(metrics_registry_test gtest)
endif()

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(dummy_test Threads::Threads ${GTEST_BOTH_LIBRARIES})
target_link_libraries(expression_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(
  clamped_arithmetic_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  test_age_formatting libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  password_hash_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  playmat_resolver_test libcockatrice_deck_list libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
target_link_libraries(
  deck_hash_performance_test libcockatrice_deck_list libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
target_link_libraries(
  server_card_counter_test libcockatrice_network Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  server_counter_test libcockatrice_network Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  server_rate_limiter_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(
  server_developer_role_test libcockatrice_network libcockatrice_rng Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
target_link_libraries(
  warning_categories_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(lag_monitor_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(
  latency_tracker_test libcockatrice_network Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_include_directories(metrics_registry_test PRIVATE ${CMAKE_SOURCE_DIR}/servatrice/src)
target_link_libraries(metrics_registry_test ${TEST_QT_MODULES} Threads::Threads ${GTEST_BOTH_LIBRARIES})

add_subdirectory(card_zone_algorithms)
add_subdirectory(carddatabase)
add_subdirectory(deck_list_zones)
add_subdirectory(loading_from_clipboard)
add_subdirectory(movecard_tests)
add_subdirectory(oracle)
add_subdirectory(settings)
