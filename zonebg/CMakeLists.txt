# CMakeLists for zonebg/ directory
#
# Installs default "zone background" files

FILE(GLOB zonebg "${CMAKE_CURRENT_SOURCE_DIR}/zonebg/*.*")
INSTALL(FILES ${zonebg} DESTINATION share/cockatrice/zonebg)
