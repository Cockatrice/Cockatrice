set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS libcockatrice/rng/rng_abstract.h libcockatrice/rng/rng_sfmt.h libcockatrice/rng/sfmt/SFMT.h)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_rng STATIC ${MOC_SOURCES} libcockatrice/rng/rng_abstract.cpp libcockatrice/rng/rng_sfmt.cpp
                           libcockatrice/rng/sfmt/SFMT.c
)

target_include_directories(libcockatrice_rng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_rng PUBLIC ${COCKATRICE_QT_MODULES})
