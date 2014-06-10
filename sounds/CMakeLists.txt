FILE(GLOB sounds "${CMAKE_CURRENT_SOURCE_DIR}/sounds/*.raw")
INSTALL(FILES ${sounds} DESTINATION share/cockatrice/sounds)
