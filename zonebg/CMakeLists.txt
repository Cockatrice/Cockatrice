# CMakeLists for zonebg/ directory
#
# Installs default "zone background" files

FILE(GLOB zonebg "${CMAKE_CURRENT_SOURCE_DIR}/*.*")

if(UNIX)
    if(APPLE)
      	INSTALL(FILES ${zonebg} DESTINATION ${CMAKE_INSTALL_PREFIX}/zonebg)
    else()
        # Assume linux
        INSTALL(FILES ${zonebg} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cockatrice/zonebg)
    endif()
elseif(WIN32)
    INSTALL(FILES ${zonebg} DESTINATION ${CMAKE_INSTALL_PREFIX}/zonebg)
endif()