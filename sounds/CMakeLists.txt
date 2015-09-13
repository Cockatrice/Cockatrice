# CMakeLists for sounds directory
#
# add sounds subfolders

SET(defsounds
    Default
    Legacy
)

if(UNIX)
    if(APPLE)
        INSTALL(DIRECTORY ${defsounds} DESTINATION cockatrice.app/Contents/Resources/sounds/)
    else()
        # Assume linux
        INSTALL(DIRECTORY ${defsounds} DESTINATION share/cockatrice/sounds/)
    endif()
elseif(WIN32)
    INSTALL(DIRECTORY ${defsounds} DESTINATION sounds/)
endif()
