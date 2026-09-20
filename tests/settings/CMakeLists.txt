add_executable(settings_manager_test settings_manager_test.cpp)
add_executable(settings_defaults_test settings_defaults_test.cpp)
add_executable(settings_migration_test settings_migration_test.cpp)

target_link_libraries(
  settings_manager_test libcockatrice_settings Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)

target_link_libraries(
  settings_defaults_test libcockatrice_settings Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)
target_compile_definitions(settings_defaults_test PRIVATE SETTINGS_UNIT_TEST)

target_link_libraries(
  settings_migration_test libcockatrice_settings Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES}
)

add_test(NAME settings_manager_test COMMAND settings_manager_test)
add_test(NAME settings_defaults_test COMMAND settings_defaults_test)
add_test(NAME settings_migration_test COMMAND settings_migration_test)

if(NOT GTEST_FOUND)
  add_dependencies(settings_manager_test gtest)
  add_dependencies(settings_defaults_test gtest)
  add_dependencies(settings_migration_test gtest)
endif()
