# expands ~ to user home directory
#
# usage:
# expanduser("~/code" x)

function(expanduser in outvar)

  if(CMAKE_VERSION VERSION_LESS 3.21)
    get_filename_component(out ${in} ABSOLUTE)
  else()
    file(REAL_PATH ${in} out EXPAND_TILDE)
  endif()

  set(${outvar}
      ${out}
      PARENT_SCOPE
  )

endfunction(expanduser)
