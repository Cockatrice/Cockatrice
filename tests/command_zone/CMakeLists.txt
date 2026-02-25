# ------------------------
# Qt modules for command zone tests
# Qt::Gui required for QColor in z_values.h
# Qt::Widgets required for QGraphicsItem in abstract_card_item.h
# ------------------------
set(TEST_QT_MODULES
    ${COCKATRICE_QT_VERSION_NAME}::Core
    ${COCKATRICE_QT_VERSION_NAME}::Test
    ${COCKATRICE_QT_VERSION_NAME}::Gui
    ${COCKATRICE_QT_VERSION_NAME}::Widgets
)

# ------------------------
# Command Zone Logic Test
# Tests addCardImpl insertion behavior
# ------------------------
add_executable(command_zone_logic_test
    mocks/mock_card_item.cpp
    command_zone_logic_test.cpp
)

target_include_directories(command_zone_logic_test
    PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src/game/zones/logic
)

target_link_libraries(command_zone_logic_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
    PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME command_zone_logic_test COMMAND command_zone_logic_test)

# ------------------------
# Commander Tax Counter Test
# Tests setValue clamping behavior
# ------------------------
add_executable(commander_tax_counter_test
    commander_tax_counter_test.cpp
)

target_link_libraries(commander_tax_counter_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
    PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME commander_tax_counter_test COMMAND commander_tax_counter_test)

# ------------------------
# Command Zone State Test
# Tests the extracted CommandZoneState pure state machine
# ------------------------
add_executable(command_zone_state_test
    command_zone_state_test.cpp
    ${CMAKE_SOURCE_DIR}/cockatrice/src/game/zones/command_zone_state.cpp
)

target_include_directories(command_zone_state_test
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src/game/zones
)

target_link_libraries(command_zone_state_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
    PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME command_zone_state_test COMMAND command_zone_state_test)

# ------------------------
# Command Zone Integration Test
# Tests counter parenting and zone coordination
# ------------------------
add_executable(command_zone_integration_test
    command_zone_integration_test.cpp
)

target_include_directories(command_zone_integration_test
    PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(command_zone_integration_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
    PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME command_zone_integration_test COMMAND command_zone_integration_test)

# ------------------------
# Counter Visibility Test
# Tests counter visibility during zone state transitions
# ------------------------
add_executable(counter_visibility_test
    counter_visibility_test.cpp
)

target_link_libraries(counter_visibility_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
    PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME counter_visibility_test COMMAND counter_visibility_test)

# ------------------------
# Dependencies on gtest
# ------------------------
if(NOT GTEST_FOUND)
    add_dependencies(command_zone_logic_test gtest)
    add_dependencies(commander_tax_counter_test gtest)
    add_dependencies(command_zone_state_test gtest)
    add_dependencies(command_zone_integration_test gtest)
    add_dependencies(counter_visibility_test gtest)
endif()
