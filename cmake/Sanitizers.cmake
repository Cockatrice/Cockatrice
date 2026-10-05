# AddressSanitizer + UndefinedBehaviorSanitizer opt-in, plus per-source opt-out
# Input: ENABLE_SANITIZERS
# Defines: cockatrice_disable_sanitizers_for
#
# Included once from the root CMakeLists.txt, immediately after the per-compiler flag
# block, so add_compile_options() reaches every library and executable defined below.

include(CheckCXXSourceCompiles)

# Opt a set of vendored sources out of instrumentation.
#
# cockatrice_disable_sanitizers_for(<source> ...)
#
# Sanitizer findings inside vendored code are not actionable for us, and letting them
# through buries our own bugs in noise until everyone learns to skip the job.
#
# The sources must be named relative to the directory that compiles them. CMake resolves a
# source-file property against the directory scope the target was declared in, so calling
# this from the root with absolute paths silently does nothing. That is why the call sites
# live next to the add_library/add_executable that pull the vendored code in.
#
# SKIP_PRECOMPILE_HEADERS is not optional. The precompiled header is built with the
# sanitizer flags, so it defines __SANITIZE_ADDRESS__; a translation unit compiled with
# -fno-sanitize=all does not, and GCC then refuses the PCH with
# "not used because __SANITIZE_ADDRESS__ not defined", which is a fatal -Werror=invalid-pch
# in the servatrice and oracle builds.
function(cockatrice_disable_sanitizers_for)
  if(NOT ENABLE_SANITIZERS)
    return()
  endif()
  set_source_files_properties(${ARGN} PROPERTIES COMPILE_OPTIONS "-fno-sanitize=all" SKIP_PRECOMPILE_HEADERS ON)
endfunction()

if(ENABLE_SANITIZERS)
  # Probe a flag the way it will actually be used: on the compile step AND the link step.
  # check_cxx_compiler_flag() only probes the compile step, which makes every sanitizer
  # flag look unsupported because linking an ASan object without -fsanitize=address leaves
  # __asan_init undefined. That is a silent misconfiguration rather than an error, so the
  # probe has to link.
  function(cockatrice_probe_flag FLAG VARNAME)
    set(CMAKE_REQUIRED_FLAGS "${FLAG}")
    set(CMAKE_REQUIRED_LINK_OPTIONS "${FLAG}")
    check_cxx_source_compiles("int main() { return 0; }" ${VARNAME})
    unset(CMAKE_REQUIRED_FLAGS)
    unset(CMAKE_REQUIRED_LINK_OPTIONS)
  endfunction()

  set(SANITIZER_FLAGS
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
      # Qt builds QObject subclasses into raw storage and writes the vptr afterwards, which
      # the vptr check reports on essentially every QObject instantiation. The check has no
      # value for us because nothing outside moc-generated code inherits QObject, so it is
      # off outright rather than suppressed piecemeal.
      -fno-sanitize=vptr
  )

  # MSVC has no ASan/UBSan equivalent, and clang-cl would need /fsanitize=address plus a
  # matching runtime link, which is a separate piece of work.
  if(MSVC)
    message(FATAL_ERROR "ENABLE_SANITIZERS is not supported with MSVC")
  endif()

  # Probe rather than assume, so an older toolchain gets the sanitizers it can support
  # instead of failing the build on a flag it does not recognize.
  set(SANITIZER_SUPPORTED_FLAGS)
  foreach(FLAG ${SANITIZER_FLAGS})
    string(MAKE_C_IDENTIFIER "COCKATRICE_HAS${FLAG}" _FLAG_VAR)
    cockatrice_probe_flag("${FLAG}" ${_FLAG_VAR})
    if(${_FLAG_VAR})
      list(APPEND SANITIZER_SUPPORTED_FLAGS ${FLAG})
    else()
      message(STATUS "Sanitizers: ${FLAG} unsupported by ${CMAKE_CXX_COMPILER_ID}, skipping")
    endif()
  endforeach()

  # The whole point of the option is the sanitizers, so an unsupported pair is fatal even
  # though the cosmetic flags above are allowed to degrade.
  if(NOT "-fsanitize=address,undefined" IN_LIST SANITIZER_SUPPORTED_FLAGS)
    message(FATAL_ERROR "ENABLE_SANITIZERS requires -fsanitize=address,undefined, which "
                        "${CMAKE_CXX_COMPILER_ID} does not accept"
    )
  endif()

  add_compile_options(${SANITIZER_SUPPORTED_FLAGS})
  add_link_options(-fsanitize=address,undefined)

  # Recorded so the test tree can label its entries and a plain `make` at the same time
  # cannot quietly disagree with what CI ran.
  set(COCKATRICE_SANITIZERS
      ON
      CACHE INTERNAL "AddressSanitizer and UBSan are enabled"
  )
endif()
