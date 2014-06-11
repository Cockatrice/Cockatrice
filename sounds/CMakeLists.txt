# CMakeLists for sounds/ directory
#
# Installs default sound files

FILE(GLOB sounds "${CMAKE_CURRENT_SOURCE_DIR}/*.raw")

if(UNIX)
    if(APPLE)
      	INSTALL(FILES ${sounds} DESTINATION ${CMAKE_INSTALL_PREFIX}/sounds)
    else()
        # Assume linux
        INSTALL(FILES ${sounds} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cockatrice/sounds)
    endif()
elseif(WIN32)
    INSTALL(FILES ${sounds} DESTINATION ${CMAKE_INSTALL_PREFIX}/sounds)
endif()