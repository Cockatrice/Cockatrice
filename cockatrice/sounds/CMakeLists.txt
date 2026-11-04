# CMakeLists for sounds directory
#
# add sounds subfolders

set(defsounds Default Legacy)

if(UNIX)
  if(APPLE)
    install(DIRECTORY ${defsounds} DESTINATION Cockatrice.app/Contents/Resources/sounds/)
  else()
    # Assume linux
    install(DIRECTORY ${defsounds} DESTINATION share/cockatrice/sounds/)
  endif()
elseif(WIN32)
  install(DIRECTORY ${defsounds} DESTINATION sounds/)
endif()
