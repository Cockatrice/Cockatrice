set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS local_server.h local_server_interface.h)

set(SOURCES local_server.cpp local_server_interface.cpp)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(libcockatrice_network_server_local STATIC ${MOC_SOURCES} ${SOURCES})

add_dependencies(libcockatrice_network_server_local libcockatrice_protocol)

target_include_directories(libcockatrice_network_server_local PUBLIC .)

target_link_libraries(libcockatrice_network_server_local PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_protocol)
