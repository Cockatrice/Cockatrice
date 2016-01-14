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

# Find Qt libraries, used by classes invoked by tests

set(TESTS_QT_LIBS)

# Qt4 stuff
if(Qt4_FOUND)
    SET(QT_USE_QTNETWORK TRUE)

    # Include directories
    INCLUDE(${QT_USE_FILE})
    INCLUDE_DIRECTORIES(${QT_INCLUDES})
    LIST(APPEND TESTS_QT_LIBS ${QT_LIBRARIES})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND TESTS_QT_LIBS Widgets)

    # QtNetwork
    find_package(Qt5Network)
    if(Qt5Network_FOUND)
        include_directories(${Qt5Network_INCLUDE_DIRS})
        list(APPEND TESTS_QT_LIBS Network)
    endif()
endif()


# enable c++11
set_property(TARGET dummy_test PROPERTY CXX_STANDARD 11)
set_property(TARGET dummy_test PROPERTY CXX_STANDARD_REQUIRED ON)

# dummy_test is using includes from common
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/common)

include_directories(${GTEST_INCLUDE_DIRS})

if(Qt4_FOUND)
    target_link_libraries(dummy_test ${GTEST_BOTH_LIBRARIES} ${TESTS_QT_LIBS})
endif()
if(Qt5Widgets_FOUND)
    target_link_libraries(dummy_test ${GTEST_BOTH_LIBRARIES})
    qt5_use_modules(dummy_test ${TESTS_QT_LIBS})
endif()
