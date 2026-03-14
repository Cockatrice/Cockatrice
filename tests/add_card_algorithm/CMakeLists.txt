add_executable(add_card_algorithm_test add_card_algorithm_test.cpp)

target_include_directories(add_card_algorithm_test
    PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src/game/zones/logic
)

target_link_libraries(add_card_algorithm_test
    PRIVATE Threads::Threads
    PRIVATE ${GTEST_BOTH_LIBRARIES}
)

add_test(NAME add_card_algorithm_test COMMAND add_card_algorithm_test)

if(NOT GTEST_FOUND)
    add_dependencies(add_card_algorithm_test gtest)
endif()
