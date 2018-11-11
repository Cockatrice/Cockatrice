# Find the MS Visual Studio VC redistributable package

if (WIN32)
    set(VCREDISTRUNTIME_FOUND "NO")

    if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
        set(REDIST_ARCH x64)
    else()
        set(REDIST_ARCH x86)
    endif()

    set(REDIST_FILE vcredist_${REDIST_ARCH}.exe)

    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)

    # Check if the list contains minimum one element, to get the path from
    list(LENGTH CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS libsCount)
    if (libsCount GREATER 0)
        list(GET CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS 0 _path)

        get_filename_component(_path ${_path} DIRECTORY)
        get_filename_component(_path ${_path}/../../ ABSOLUTE)

        if (EXISTS "${_path}/${REDIST_FILE}") # VS 2017
            set(VCREDISTRUNTIME_FOUND "YES")
            set(VCREDISTRUNTIME_FILE ${_path}/${REDIST_FILE})
        endif()
    endif()

    if(VCREDISTRUNTIME_FOUND)
      message(STATUS "Found VCredist ${VCREDISTRUNTIME_FILE}")
    else()
      message(WARNING "Could not find VCredist package. It's not required for compiling, but needs to be available at runtime.")
    endif()
endif()
