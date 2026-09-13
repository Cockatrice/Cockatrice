# CMakeLists for ./cockatrice/themes directory
#
# Add themes subfolders

set(defthemes Default Fabric Fusion Leather Plasma VelvetMarble)

if(UNIX)
  if(APPLE) # macOS
    install(DIRECTORY ${defthemes} DESTINATION Cockatrice.app/Contents/Resources/themes/)
  else() # Assume Linux
    install(DIRECTORY ${defthemes} DESTINATION share/cockatrice/themes/)
  endif()
elseif(WIN32)
  install(DIRECTORY ${defthemes} DESTINATION themes/)
endif()
