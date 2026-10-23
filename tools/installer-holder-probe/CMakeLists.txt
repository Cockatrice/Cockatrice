# Windows-only. The installer runs this on the abort path of a failed purge to name the
# applications and services holding the install directory open (Restart Manager); see
# the LogProcessesUsingInstallDir function in cmake/NSIS.template.in. It ships inside
# the installer and is extracted to $PLUGINSDIR there, never installed, so it links
# nothing from this project: no Qt, and a static C runtime because the installed runtime
# is exactly what the purge may have failed to replace.
add_executable(installer-holder-probe holder_line.cpp main.cpp restart_manager_holders.cpp)

target_link_libraries(installer-holder-probe PRIVATE rstrtmgr)

# The NSIS template embeds the built file by the same relative path that resolves
# NSIS.definitions.nsh, and that path points at the build root.
set_target_properties(
  installer-holder-probe
  PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
)
