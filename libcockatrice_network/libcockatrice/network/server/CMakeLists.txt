set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(local)
add_subdirectory(remote)

add_library(libcockatrice_network_server INTERFACE)

target_include_directories(libcockatrice_network_server INTERFACE .)

target_link_libraries(
  libcockatrice_network_server INTERFACE libcockatrice_network_server_local libcockatrice_network_server_remote
)
include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
