# CMakeLists for themes directory
#
# add themes subfolders

SET(defthemes
    Default
    Fabric
    Leather
    Plasma
    VelvetMarble
)

if(UNIX)
    if(APPLE)
        INSTALL(DIRECTORY ${defthemes} DESTINATION Cockatrice.app/Contents/Resources/themes/)
    else()
        # Assume linux
        INSTALL(DIRECTORY ${defthemes} DESTINATION share/cockatrice/themes/)
    endif()
elseif(WIN32)
    INSTALL(DIRECTORY ${defthemes} DESTINATION themes/)
endif()
