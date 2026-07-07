# CMakeLists for sounds directory
#
# Add sounds subfolders

set(defsounds Default Legacy)

if(UNIX)
  if(APPLE)
    install(DIRECTORY ${defsounds} DESTINATION Cockatrice.app/Contents/Resources/sounds/)
  else()
    # Assume Linux
    install(DIRECTORY ${defsounds} DESTINATION share/cockatrice/sounds/)
  endif()
elseif(WIN32) # Windows (including 64bit)
  install(DIRECTORY ${defsounds} DESTINATION sounds/)
endif()
