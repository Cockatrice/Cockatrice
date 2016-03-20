enable_testing()
add_test(NAME dummy_test COMMAND dummy_test)

# Find GTest

add_executable(dummy_test dummy_test.cpp)

find_package(GTest)

if(NOT GTEST_FOUND)
    IF(NOT EXISTS "${CMAKE_BINARY_DIR}/gtest-build")
        message(STATUS "Downloading googletest")
        configure_file("${CMAKE_SOURCE_DIR}/cmake/gtest-CMakeLists.txt.in" "${CMAKE_BINARY_DIR}/gtest-download/CMakeLists.txt")
        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download )
        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download )
    ELSE()
        message(STATUS "GoogleTest directory exists")
    ENDIF()

    # Add gtest directly to our build
    add_subdirectory(${CMAKE_BINARY_DIR}/gtest-src
            ${CMAKE_BINARY_DIR}/gtest-build
            EXCLUDE_FROM_ALL )

    # Add the gtest include directory, since gtest
    # doesn't add that dependency to its gtest target
    target_include_directories(gtest INTERFACE
            "${CMAKE_BINARY_DIR}/gtest-src/include" )

    SET(GTEST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/gtest-src/include")
    SET(GTEST_BOTH_LIBRARIES gtest)
    add_dependencies(dummy_test gtest)
endif()

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(dummy_test ${GTEST_BOTH_LIBRARIES})

add_subdirectory(carddatabase)