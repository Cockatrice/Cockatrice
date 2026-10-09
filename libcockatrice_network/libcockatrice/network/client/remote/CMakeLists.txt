set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS remote_client.h)

set(SOURCES remote_client.cpp)

qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

add_library(libcockatrice_network_client_remote STATIC ${MOC_SOURCES} ${SOURCES})

add_dependencies(libcockatrice_network_client_remote libcockatrice_network_client_abstract libcockatrice_protocol)

target_include_directories(libcockatrice_network_client_remote PUBLIC .)

target_link_libraries(
  libcockatrice_network_client_remote
  PUBLIC ${QT_MAIN_VERSION_STRING}::Network ${QT_MAIN_VERSION_STRING}::WebSockets libcockatrice_network_client_abstract
         libcockatrice_interfaces libcockatrice_utility libcockatrice_protocol
)
