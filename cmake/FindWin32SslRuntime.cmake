# Find the OpenSSL runtime libraries (.dll) for Windows that
# will be needed by Qt in order to access https urls.
if(NOT DEFINED WIN32 OR NOT ${WIN32})
  message(STATUS "Non-Windows device trying to execute FindWin32SslRuntime, skipping")
  return()
endif()

if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
  message(STATUS "Looking for OpenSSL for ${CMAKE_GENERATOR_PLATFORM}")
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
  set(_OPENSSL_ROOT_PATHS "$ENV{VCPKG_PACKAGES_DIR}/x64-windows/bin" "C:/OpenSSL-Win64/bin" "C:/OpenSSL-Win64"
                          "C:/Tools/vcpkg/installed/x64-windows/bin" "${_programfiles}/OpenSSL-Win64"
  )
  unset(_programfiles)
elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
  message(STATUS "Looking for OpenSSL for ${CMAKE_GENERATOR_PLATFORM}")
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
  set(_OPENSSL_ROOT_PATHS
      "$ENV{VCPKG_PACKAGES_DIR}/x86-windows/bin"
      "C:/OpenSSL-Win32/bin"
      "C:/OpenSSL-Win32"
      "C:/OpenSSL"
      "C:/Tools/vcpkg/installed/x86-windows/bin"
      "${_programfiles}/OpenSSL"
      "${_programfiles}/OpenSSL-Win32"
  )
  unset(_programfiles)
endif()

message(STATUS "Looking for OpenSSL @ $ENV{CMAKE_GENERATOR_PLATFORM} in ${_OPENSSL_ROOT_PATHS}")
if("$ENV{CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
  find_file(
    WIN32SSLRUNTIME_LIBEAY
    NAMES libcrypto-1_1-x64.dll libcrypto.dll
    PATHS ${_OPENSSL_ROOT_PATHS}
    NO_DEFAULT_PATH
  )
  find_file(
    WIN32SSLRUNTIME_SSLEAY
    NAMES libssl-1_1-x64.dll libssl.dll
    PATHS ${_OPENSSL_ROOT_PATHS}
    NO_DEFAULT_PATH
  )
elseif("$ENV{CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
  find_file(
    WIN32SSLRUNTIME_LIBEAY
    NAMES libcrypto-1_1.dll libcrypto.dll
    PATHS ${_OPENSSL_ROOT_PATHS}
    NO_DEFAULT_PATH
  )
  find_file(
    WIN32SSLRUNTIME_SSLEAY
    NAMES libssl-1_1.dll libssl.dll
    PATHS ${_OPENSSL_ROOT_PATHS}
    NO_DEFAULT_PATH
  )
endif()

if(WIN32SSLRUNTIME_LIBEAY AND WIN32SSLRUNTIME_SSLEAY)
  set(WIN32SSLRUNTIME_LIBRARIES "${WIN32SSLRUNTIME_LIBEAY}" "${WIN32SSLRUNTIME_SSLEAY}")
  set(WIN32SSLRUNTIME_FOUND "YES")
  message(STATUS "Found OpenSSL ${WIN32SSLRUNTIME_LIBRARIES}")
else()
  set(WIN32SSLRUNTIME_FOUND "NO")
  message(
    WARNING
      "Could not find OpenSSL runtime libraries. They are not required for compiling, but needs to be available at runtime."
  )
endif()

mark_as_advanced(WIN32SSLRUNTIME_LIBEAY WIN32SSLRUNTIME_SSLEAY)
