cmake_minimum_required(VERSION 3.16)
project(Utility VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UTILITY_SOURCES libcockatrice/utility/expression.cpp libcockatrice/utility/levenshtein.cpp
                    libcockatrice/utility/passwordhasher.cpp
)

set(UTILITY_HEADERS
    libcockatrice/utility/color.h libcockatrice/utility/expression.h libcockatrice/utility/levenshtein.h
    libcockatrice/utility/macros.h libcockatrice/utility/passwordhasher.h libcockatrice/utility/trice_limits.h
)

add_library(libcockatrice_utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})

target_include_directories(libcockatrice_utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_utility PUBLIC libcockatrice_rng ${QT_CORE_MODULE})

set(ORACLE_LIBS)

include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
