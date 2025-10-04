cmake_minimum_required(VERSION 3.16)
project(Utility VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UTILITY_SOURCES
    src/expression.cpp
    src/featureset.cpp
    src/key_signals.cpp
    src/levenshtein.cpp
    src/logger.cpp
    src/passwordhasher.cpp
)

set(UTILITY_HEADERS
    include/utility/color.h
    include/utility/expression.h
    include/utility/featureset.h
    include/utility/key_signals.h
    include/utility/levenshtein.h
    include/utility/logger.h
    include/utility/macros.h
    include/utility/passwordhasher.h
    include/utility/trice_limits.h
)

qt_add_library(utility STATIC
    ${UTILITY_SOURCES}
    ${UTILITY_HEADERS}
)

target_include_directories(utility
    PUBLIC include
)

target_link_libraries(utility
    PUBLIC ${COCKATRICE_QT_MODULES}
)

set(ORACLE_LIBS)

include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
