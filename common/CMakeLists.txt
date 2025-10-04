# CMakeLists for common directory
#
# provides the common library

set(common_SOURCES
)

set(ORACLE_LIBS)

include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(cockatrice_common ${common_SOURCES} ${common_MOC_SRCS})
