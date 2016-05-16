ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(carddatabase_test
    carddatabase_test.cpp
    ../../cockatrice/src/carddatabase.cpp

)
if(NOT GTEST_FOUND)
    add_dependencies(carddatabase_test gtest)
endif()
target_link_libraries(carddatabase_test ${GTEST_BOTH_LIBRARIES})
add_test(NAME carddatabase_test COMMAND carddatabase_test)

# qt5 stuff
include_directories(${Qt5Widgets_INCLUDE_DIRS})
list(APPEND COCKATRICE_LIBS Widgets)

# QtConcurrent
find_package(Qt5Concurrent)
if(Qt5Concurrent_FOUND)
    include_directories(${Qt5Concurrent_INCLUDE_DIRS})
    list(APPEND ORACLE_LIBS Concurrent)
endif()

# QtNetwork
find_package(Qt5Network)
if(Qt5Network_FOUND)
    include_directories(${Qt5Network_INCLUDE_DIRS})
    list(APPEND COCKATRICE_LIBS Network)
endif()

qt5_use_modules(carddatabase_test ${COCKATRICE_LIBS})
