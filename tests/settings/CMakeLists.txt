cockatrice_add_test(NAME settings_manager_test SOURCES settings_manager_test.cpp LIBS libcockatrice_settings)

cockatrice_add_test(
  NAME
  settings_defaults_test
  SOURCES
  settings_defaults_test.cpp
  LIBS
  libcockatrice_settings
  DEFINITIONS
  SETTINGS_UNIT_TEST
)

cockatrice_add_test(NAME settings_migration_test SOURCES settings_migration_test.cpp LIBS libcockatrice_settings)
