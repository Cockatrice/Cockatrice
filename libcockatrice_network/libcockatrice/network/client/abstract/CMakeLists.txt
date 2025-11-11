set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS abstract_client.h)

set(SOURCES abstract_client.cpp)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(libcockatrice_network_client_abstract STATIC ${MOC_SOURCES} ${SOURCES})

add_dependencies(libcockatrice_network_client_abstract libcockatrice_protocol libcockatrice_network_server_remote)

target_include_directories(libcockatrice_network_client_abstract PUBLIC .)

target_link_libraries(
  libcockatrice_network_client_abstract
  PUBLIC ${COCKATRICE_QT_VERSION_NAME}::Network ${COCKATRICE_QT_VERSION_NAME}::WebSockets libcockatrice_protocol
         libcockatrice_network_server_remote
)
