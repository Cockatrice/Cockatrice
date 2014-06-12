# CMakeLists for sounds/ directory
#
# Installs default sound files

FILE(GLOB sounds "${CMAKE_CURRENT_SOURCE_DIR}/*.raw")

if(UNIX)
    if(APPLE)
      	INSTALL(FILES ${sounds} DESTINATION sounds/)
    else()
        # Assume linux
        INSTALL(FILES ${sounds} DESTINATION share/cockatrice/sounds/)
    endif()
elseif(WIN32)
    INSTALL(FILES ${sounds} DESTINATION sounds/)
endif()