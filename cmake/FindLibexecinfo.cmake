# Find the LibExecinfo library - FreeBSD only

find_path(LIBEXECINFO_INCLUDE_DIR execinfo.h)
find_library(LIBEXECINFO_LIBRARY NAMES execinfo)

if(LIBEXECINFO_INCLUDE_DIR AND LIBEXECINFO_LIBRARY)
  set(LIBEXECINFO_FOUND TRUE)
endif()

if(LIBEXECINFO_FOUND)
  if(NOT LIBEXECINFO_FIND_QUIETLY)
    message(STATUS "Found LibExecinfo: ${EXECINFO_LIBRARY}")
  endif()
else()
  if(LIBEXECINFO_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find LibExecinfo")
  endif()
endif()
