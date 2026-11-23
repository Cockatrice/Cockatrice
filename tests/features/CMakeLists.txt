# The catalog and the planner are deliberately free of client dependencies, so both
# targets compile the two implementation files directly instead of linking the app.
cockatrice_add_test(
  NAME
  feature_intro_planner_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/features/feature_intro_catalog.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/features/feature_intro_planner.cpp
  feature_intro_planner_test.cpp
  LIBS
  libcockatrice_settings
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/features
)

cockatrice_add_test(
  NAME
  feature_intro_catalog_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/features/feature_intro_catalog.cpp
  feature_intro_catalog_test.cpp
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/features
)
