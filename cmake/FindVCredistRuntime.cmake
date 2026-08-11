# Find the MS Visual Studio VC redistributable package

if(WIN32) # Windows (including 64bit)
  set(VCREDISTRUNTIME_FOUND "NO")

  if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
    set(REDIST_ARCH x64)
  endif()

  set(REDIST_FILE_NAMES vc_redist.${REDIST_ARCH}.exe)

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)

  # Check if the list contains minimum one element, to get the path from
  list(LENGTH CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS libsCount)
  if(libsCount GREATER 0)
    list(GET CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS 0 _path)

    get_filename_component(_path ${_path} DIRECTORY)
    get_filename_component(_path ${_path}/../../ ABSOLUTE)

    foreach(redist_file ${REDIST_FILE_NAMES})
      if(EXISTS "${_path}/${redist_file}")
        set(VCREDISTRUNTIME_FOUND "YES")
        set(VCREDISTRUNTIME_FILE ${_path}/${redist_file})
        break()
      endif()
    endforeach()
  endif()

  if(VCREDISTRUNTIME_FOUND)
    message(STATUS "Found VCredist ${VCREDISTRUNTIME_FILE}")
  else()
    message(
      WARNING "Could not find VCredist package. It's not required for compiling, but needs to be available at runtime."
    )
  endif()
endif()
