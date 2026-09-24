add_executable(deck_list_model_custom_zones_test ${VERSION_STRING_CPP} deck_list_model_custom_zones_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(deck_list_model_custom_zones_test gtest)
endif()

target_link_libraries(
  deck_list_model_custom_zones_test
  libcockatrice_models
  libcockatrice_card
  libcockatrice_deck_list
  Threads::Threads
  ${GTEST_BOTH_LIBRARIES}
  ${QT_MODULES_TEST}
)
add_test(NAME deck_list_model_custom_zones_test COMMAND deck_list_model_custom_zones_test)

add_executable(deck_list_model_zone_integration_test ${VERSION_STRING_CPP} deck_list_model_zone_integration_test.cpp)

if(NOT GTEST_FOUND)
  add_dependencies(deck_list_model_zone_integration_test gtest)
endif()

target_link_libraries(
  deck_list_model_zone_integration_test
  libcockatrice_models
  libcockatrice_card
  libcockatrice_deck_list
  Threads::Threads
  ${GTEST_BOTH_LIBRARIES}
  ${QT_MODULES_TEST}
)
add_test(NAME deck_list_model_zone_integration_test COMMAND deck_list_model_zone_integration_test)
