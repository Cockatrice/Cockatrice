# CMakeLists for themes directory
#
# add themes subfolders

set(defthemes Fabric Fusion Leather Plasma VelvetMarble System)

if(UNIX)
  if(APPLE)
    install(DIRECTORY ${defthemes} DESTINATION Cockatrice.app/Contents/Resources/themes/)
  else()
    # Assume linux
    install(DIRECTORY ${defthemes} DESTINATION share/cockatrice/themes/)
  endif()
elseif(WIN32)
  install(DIRECTORY ${defthemes} DESTINATION themes/)
endif()
