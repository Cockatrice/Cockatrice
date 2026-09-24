# CMakeLists for themes directory
#
# Add themes subfolders

set(defthemes Fabric Fusion Leather Plasma VelvetMarble System)

if(UNIX)
  if(APPLE)
    install(DIRECTORY ${defthemes} DESTINATION Cockatrice.app/Contents/Resources/themes/)
  else()
    # Assume Linux
    install(DIRECTORY ${defthemes} DESTINATION share/cockatrice/themes/)
  endif()
elseif(WIN32) # Windows (including 64bit)
  install(DIRECTORY ${defthemes} DESTINATION themes/)
endif()
