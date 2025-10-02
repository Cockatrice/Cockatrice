# copied from vcpkg/triplets/x64-osx.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES x86_64)
# end of copied code

# Set the minimum macOS version for all dependencies
set(VCPKG_CMAKE_SYSTEM_VERSION 13.0)
set(VCPKG_OSX_DEPLOYMENT_TARGET 13.0)
