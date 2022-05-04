# Find the OpenSSL runtime libraries (.dll) for Windows that
# will be needed by Qt in order to access https urls.
if (WIN32)
  if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    message(STATUS "Looking for OpenSSL for ${CMAKE_GENERATOR_PLATFORM}")
    set(_OPENSSL_ROOT_HINTS ${OPENSSL_ROOT_DIR} ENV OPENSSL_ROOT_DIR)
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
    set(_OPENSSL_ROOT_PATHS
            "$ENV{VCPKG_PACKAGES_DIR}/x64-windows/bin"
            "C:/OpenSSL-Win64/bin"
            "C:/OpenSSL-Win64"
            "C:/Tools/vcpkg/installed/x64-windows/bin"
            "${_programfiles}/OpenSSL-Win64"
            )
    unset(_programfiles)
  elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
    message(STATUS "Looking for OpenSSL for ${CMAKE_GENERATOR_PLATFORM}")
    set(_OPENSSL_ROOT_HINTS ${OPENSSL_ROOT_DIR} ENV OPENSSL_ROOT_DIR)
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
  else()
    message(WARNING "Unknown OS_PATH: $ENV{OS_PATH}, Can't find OpenSSL correctly")
  endif()
else ()
  set(_OPENSSL_ROOT_HINTS ${OPENSSL_ROOT_DIR} ENV OPENSSL_ROOT_DIR)
endif()

message(STATUS "Looking for OpenSSL @ ${CMAKE_GENERATOR_PLATFORM} in ${_OPENSSL_ROOT_PATHS}")
if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
  FIND_FILE(WIN32SSLRUNTIME_LIBEAY NAMES libcrypto-1_1-x64.dll libcrypto-3-x64.dll libcrypto.dll PATHS ${_OPENSSL_ROOT_PATHS} NO_DEFAULT_PATH)
  FIND_FILE(WIN32SSLRUNTIME_SSLEAY NAMES libssl-1_1-x64.dll libssl-3-x64.dll libssl.dll PATHS ${_OPENSSL_ROOT_PATHS} NO_DEFAULT_PATH)
elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
  FIND_FILE(WIN32SSLRUNTIME_LIBEAY NAMES libcrypto-1_1.dll libcrypto-3.dll libcrypto.dll PATHS ${_OPENSSL_ROOT_PATHS} NO_DEFAULT_PATH)
  FIND_FILE(WIN32SSLRUNTIME_SSLEAY NAMES libssl-1_1.dll libssl-3.dll libssl.dll PATHS ${_OPENSSL_ROOT_PATHS} NO_DEFAULT_PATH)
endif()

IF(WIN32SSLRUNTIME_LIBEAY AND WIN32SSLRUNTIME_SSLEAY)
  SET(WIN32SSLRUNTIME_LIBRARIES "${WIN32SSLRUNTIME_LIBEAY}" "${WIN32SSLRUNTIME_SSLEAY}")
  SET(WIN32SSLRUNTIME_FOUND "YES")
  message(STATUS "Found OpenSSL ${WIN32SSLRUNTIME_LIBRARIES}")
ELSE()
  SET(WIN32SSLRUNTIME_FOUND "NO")
  message(WARNING "Could not find OpenSSL runtime libraries. They are not required for compiling, but needs to be available at runtime.")
ENDIF()

MARK_AS_ADVANCED(
  WIN32SSLRUNTIME_LIBEAY
  WIN32SSLRUNTIME_SSLEAY
  )
