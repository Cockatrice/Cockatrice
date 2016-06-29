# Find the OpenSSL runtime libraries (.dll) for Windows that
# will be needed by Qt in order to access https urls.

if (WIN32)
  # Get standard installation paths for OpenSSL under Windows 

  # http://www.slproweb.com/products/Win32OpenSSL.html

  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # target win64
    set(_OPENSSL_ROOT_HINTS
      ${OPENSSL_ROOT_DIR}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenSSL (64-bit)_is1;Inno Setup: App Path]"
      ENV OPENSSL_ROOT_DIR
      )
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
    set(_OPENSSL_ROOT_PATHS
      "${_programfiles}/OpenSSL-Win64"
      "C:/OpenSSL-Win64/"
      )
    unset(_programfiles)
  else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    # target win32
    set(_OPENSSL_ROOT_HINTS
      ${OPENSSL_ROOT_DIR}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenSSL (32-bit)_is1;Inno Setup: App Path]"
      ENV OPENSSL_ROOT_DIR
      )
    file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _programfiles)
    set(_OPENSSL_ROOT_PATHS
      "${_programfiles}/OpenSSL"
      "${_programfiles}/OpenSSL-Win32"
      "C:/OpenSSL/"
      "C:/OpenSSL-Win32/"
      )
    unset(_programfiles)
  endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

else ()
  set(_OPENSSL_ROOT_HINTS
    ${OPENSSL_ROOT_DIR}
    ENV OPENSSL_ROOT_DIR
    )
endif ()

set(_OPENSSL_ROOT_HINTS_AND_PATHS
    HINTS ${_OPENSSL_ROOT_HINTS}
    PATHS ${_OPENSSL_ROOT_PATHS}
    )

# Even if the dll is 64bit, it's still suffixed as *32.dll
FIND_FILE(WIN32SSLRUNTIME_LIBEAY NAMES libeay32.dll ${_OPENSSL_ROOT_HINTS_AND_PATHS})
FIND_FILE(WIN32SSLRUNTIME_SSLEAY NAMES ssleay32.dll ${_OPENSSL_ROOT_HINTS_AND_PATHS})


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
