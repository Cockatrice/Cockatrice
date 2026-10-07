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
add_test(NAME server_game_join_test COMMAND server_game_join_test)
add_test(NAME warning_categories_test COMMAND warning_categories_test)
# The translation loader test compiles a .qm fixture, which needs lrelease from Qt Linguist.
if(TARGET Qt6::lrelease)
  add_test(NAME translation_loader_test COMMAND translation_loader_test)
endif()
add_test(NAME report_categories_test COMMAND report_categories_test)
add_test(NAME lag_monitor_test COMMAND lag_monitor_test)
add_test(NAME latency_tracker_test COMMAND latency_tracker_test)
add_test(NAME metrics_registry_test COMMAND metrics_registry_test)
add_test(NAME loader_local_matching_test COMMAND loader_local_matching_test)

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
add_executable(server_game_join_test server_game_join_test.cpp)
add_executable(warning_categories_test warning_categories_test.cpp)
if(TARGET Qt6::lrelease)
  add_executable(translation_loader_test translation_loader_test.cpp)

  # QTranslator only accepts real .qm files, so the translation loader test gets one compiled from its
  # own .ts fixture rather than a stub.
  set(TRANSLATION_LOADER_TEST_QM "${CMAKE_CURRENT_BINARY_DIR}/translation_loader_test_de.qm")
  add_custom_command(
    OUTPUT "${TRANSLATION_LOADER_TEST_QM}"
    COMMAND Qt6::lrelease "${CMAKE_CURRENT_SOURCE_DIR}/translation_loader_test.ts" -qm "${TRANSLATION_LOADER_TEST_QM}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/translation_loader_test.ts"
    COMMENT "Compiling translation_loader_test.ts"
  )
  add_custom_target(translation_loader_test_qm DEPENDS "${TRANSLATION_LOADER_TEST_QM}")
  add_dependencies(translation_loader_test translation_loader_test_qm)
  target_compile_definitions(translation_loader_test PRIVATE TRANSLATION_LOADER_TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")

  # A multi-config generator (MSBuild, Xcode) puts the executable in a per-config subdirectory while
  # the .qm stays in the current binary directory, so copy the fixture next to the binary: that is the
  # layout ABuildTreeFindsTheTranslationsNextToTheExecutable is about, and the only one it finds on
  # those generators.
  add_custom_command(
    TARGET translation_loader_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TRANSLATION_LOADER_TEST_QM}"
            "$<TARGET_FILE_DIR:translation_loader_test>/"
    VERBATIM
  )
endif()
add_executable(report_categories_test report_categories_test.cpp)
add_executable(lag_monitor_test ${CMAKE_SOURCE_DIR}/cockatrice/src/client/lag_monitor.cpp lag_monitor_test.cpp)
target_include_directories(lag_monitor_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)
add_executable(latency_tracker_test latency_tracker_test.cpp)
add_executable(metrics_registry_test ../servatrice/src/metrics_registry.cpp metrics_registry_test.cpp)
add_executable(
  loader_local_matching_test
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/card_picture_loader/card_picture_loader_local.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/cache_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/card_counter_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/shortcuts_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network/update/client/release_channel.cpp
  ${VERSION_STRING_CPP}
  loader_local_matching_test.cpp
)
target_include_directories(loader_local_matching_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)

# Guard must match the condition for src/single_instance_manager.cpp in cockatrice/CMakeLists.txt.
if(WITH_CLIENT)
  add_executable(
    single_instance_manager_test ${CMAKE_SOURCE_DIR}/cockatrice/src/single_instance_manager.cpp
                                 single_instance_manager_test.cpp
  )
  target_include_directories(single_instance_manager_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)

  add_test(NAME single_instance_manager_test COMMAND single_instance_manager_test)
endif()

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
  add_dependencies(server_game_join_test gtest)
  add_dependencies(warning_categories_test gtest)
  if(TARGET translation_loader_test)
    add_dependencies(translation_loader_test gtest)
  endif()
  add_dependencies(report_categories_test gtest)
  add_dependencies(lag_monitor_test gtest)
  add_dependencies(latency_tracker_test gtest)
  add_dependencies(metrics_registry_test gtest)
  add_dependencies(loader_local_matching_test gtest)
  add_dependencies(single_instance_manager_test gtest)
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
  server_game_join_test libcockatrice_network_server_remote libcockatrice_rng Threads::Threads ${GTEST_BOTH_LIBRARIES}
  ${TEST_QT_MODULES}
)
target_link_libraries(
  warning_categories_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
if(TARGET translation_loader_test)
  target_link_libraries(
    translation_loader_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
  )
endif()
target_link_libraries(
  report_categories_test libcockatrice_utility Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_link_libraries(lag_monitor_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(
  latency_tracker_test libcockatrice_network Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_include_directories(metrics_registry_test PRIVATE ${CMAKE_SOURCE_DIR}/servatrice/src)
target_link_libraries(metrics_registry_test ${TEST_QT_MODULES} Threads::Threads ${GTEST_BOTH_LIBRARIES})
target_link_libraries(
  loader_local_matching_test libcockatrice_settings Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)

if(WITH_CLIENT)
  target_link_libraries(single_instance_manager_test Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
endif()

add_subdirectory(card_zone_algorithms)
add_subdirectory(carddatabase)
add_subdirectory(deck_list_model)
add_subdirectory(deck_list_zones)
add_subdirectory(loading_from_clipboard)
add_subdirectory(movecard_tests)
add_subdirectory(oracle)
add_subdirectory(settings)

# libcockatrice_game only exists when the client is built.
if(WITH_CLIENT)
  add_subdirectory(game)
endif()

# theme_migration_test compiles client sources, whose headers are only reachable from a client build.
if(WITH_CLIENT)
  add_subdirectory(theme_migration)
endif()

# picture_loader_benchmark links libcockatrice_settings, which only exists when
# a client/UI-capable target is being built.
if(WITH_ORACLE OR WITH_CLIENT)
  add_subdirectory(picture_loader_benchmark)
endif()
