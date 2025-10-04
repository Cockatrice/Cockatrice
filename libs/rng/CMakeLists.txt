set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS include/rng/rng_abstract.h include/rng/rng_sfmt.h include/rng/sfmt/SFMT.h)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(cockatrice_rng STATIC ${MOC_SOURCES} src/rng_abstract.cpp src/rng_sfmt.cpp src/sfmt/SFMT.c)

target_include_directories(
  cockatrice_rng
  PUBLIC include
  PUBLIC include/sfmt
)

target_link_libraries(cockatrice_rng PUBLIC ${COCKATRICE_QT_MODULES})
