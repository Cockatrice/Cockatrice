# Line-coverage instrumentation opt-in, plus per-source opt-out
# Input: ENABLE_COVERAGE
# Defines: cockatrice_disable_coverage_for
#
# Included once from the root CMakeLists.txt, immediately after the per-compiler flag
# block, so add_compile_options() reaches every library and executable defined below.
#
# Note the name deliberately shadows CMake's own built-in Modules/Coverage.cmake. It resolves
# to this file only because the root CMakeLists.txt puts cmake/ at the front of
# CMAKE_MODULE_PATH; if that ever moves, include(Coverage) would silently pick up the built-in
# module, which only defines a `COVERAGE` custom target and touches no global flags, so
# instrumentation would quietly stop happening rather than erroring.

include(CheckCXXSourceCompiles)

# Opt a set of vendored sources out of instrumentation.
#
# cockatrice_disable_coverage_for(<source> ...)
#
# Third-party code drags the total down with lines no test can ever reach, and a total that
# cannot move is not a useful signal. This is the same carve-out the sanitizer build uses.
#
# The sources must be named relative to the directory that compiles them. CMake resolves a
# source-file property against the directory scope the target was declared in, so calling
# this from the root with absolute paths silently does nothing. That is why the call sites
# live next to the add_library/add_executable that pull the vendored code in.
#
# SKIP_PRECOMPILE_HEADERS mirrors the sanitizer helper. The precompiled header carries the
# instrumentation flags, and mixing an instrumented PCH with an uninstrumented translation
# unit makes GCC reject it.
function(cockatrice_disable_coverage_for)
  if(NOT ENABLE_COVERAGE)
    return()
  endif()
  # There is no -fno-coverage: --coverage is shorthand for -fprofile-arcs -ftest-coverage, so both
  # halves have to be negated. CC rejects "-fno-coverage" outright, which is how the vendored C
  # files below hit it first.
  set_source_files_properties(
    ${ARGN} PROPERTIES COMPILE_OPTIONS "-fno-profile-arcs;-fno-test-coverage" SKIP_PRECOMPILE_HEADERS ON
  )
endfunction()

if(ENABLE_COVERAGE)
  # Line coverage needs the compiler's own toolchain, so only GCC is supported. Clang's
  # equivalent is llvm-cov, which reads a different intermediate format and would need a
  # separate report path; rejecting it is better than emitting .gcno files that gcovr cannot
  # read.
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "ENABLE_COVERAGE requires GCC, but the compiler is ${CMAKE_CXX_COMPILER_ID}. "
                        "Clang coverage needs llvm-cov and is not wired up."
    )
  endif()

  # Probe the way the flag will actually be used: compile AND link. A .gcno file comes from the
  # compile step, but the runtime that writes .gcda files comes from the link step, so probing
  # only the compile step would pass on a toolchain that cannot actually produce coverage.
  set(_cockatrice_saved_required_flags "${CMAKE_REQUIRED_FLAGS}")
  set(_cockatrice_saved_required_link_options "${CMAKE_REQUIRED_LINK_OPTIONS}")
  set(CMAKE_REQUIRED_FLAGS "-O0 -g")
  set(CMAKE_REQUIRED_LINK_OPTIONS -fprofile-arcs -ftest-coverage)
  check_cxx_source_compiles("int main() { return 0; }" COCKATRICE_HAVE_COVERAGE_FLAGS)
  set(CMAKE_REQUIRED_FLAGS "${_cockatrice_saved_required_flags}")
  set(CMAKE_REQUIRED_LINK_OPTIONS "${_cockatrice_saved_required_link_options}")

  if(NOT COCKATRICE_HAVE_COVERAGE_FLAGS)
    message(FATAL_ERROR "ENABLE_COVERAGE is on, but this toolchain rejected the coverage flags.")
  endif()

  # -O0 keeps the line mapping honest: at higher optimisation levels the compiler merges and
  # relocates lines, and the report describes code that no longer exists in that form.
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)

  message(STATUS "Coverage instrumentation enabled (--coverage). Report with gcovr over the build dir.")
endif()
