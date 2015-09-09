# CMakeLists for sounds/ directory
#
# Installs default sound files

FILE(GLOB sounds "${CMAKE_CURRENT_SOURCE_DIR}/*.wav")

if(UNIX)
    if(APPLE)
      	INSTALL(FILES ${sounds} DESTINATION cockatrice.app/Contents/Resources/sounds/)
    else()
        # Assume linux
        INSTALL(FILES ${sounds} DESTINATION share/cockatrice/sounds/)
    endif()
elseif(WIN32)
    INSTALL(FILES ${sounds} DESTINATION sounds/)
endif()