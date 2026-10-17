# Crash reporting via sentry-native (Sentry protocol, MIT).
# Included once from the root CMakeLists.txt, right after the sanitizer/coverage helpers.
#
# Inputs:
#   WITH_CRASH_REPORTING -- build the client with the sentry-native SDK (default OFF, our release
#                           CI passes ON; packagers and from-source builds stay SDK-free until this
#                           ships something real, so offline package builds never fetch the SDK)
#   COCKATRICE_SENTRY_DSN -- Sentry DSN baked into the build (default empty = SDK init skipped at runtime)
#   COCKATRICE_SENTRY_TAG -- sentry-native release tag to vendor via FetchContent
#   ENABLE_SANITIZERS -- forces the sentry backend to "none" (crashpad vs ASan conflict)
# Defines: cockatrice_target_enable_crash_reporting(<target>)

option(WITH_CRASH_REPORTING "Build with crash reporting via sentry-native" OFF)
set(COCKATRICE_SENTRY_DSN
    ""
    CACHE STRING "Sentry DSN baked into the client; empty disables crash-report upload at runtime"
)
# Pinned stable release; tag verified at https://github.com/getsentry/sentry-native/tags.
set(COCKATRICE_SENTRY_TAG
    "0.17.3"
    CACHE STRING "sentry-native release tag to vendor via FetchContent"
)

# Wire the SDK (or the no-op fallback) into a client target. Always callable: with the
# option OFF the compile definitions below make crash_reporter.cpp compile to no-ops.
function(cockatrice_target_enable_crash_reporting _target)
  if(WITH_CRASH_REPORTING AND TARGET sentry::sentry)
    target_link_libraries(${_target} PRIVATE sentry::sentry)
    target_compile_definitions(${_target} PRIVATE COCKATRICE_WITH_CRASH_REPORTING=1)
  else()
    target_compile_definitions(${_target} PRIVATE COCKATRICE_WITH_CRASH_REPORTING=0)
  endif()
  # Always baked in (possibly empty): main() prefers the COCKATRICE_SENTRY_DSN env var at runtime.
  target_compile_definitions(${_target} PRIVATE "COCKATRICE_SENTRY_DSN=\"${COCKATRICE_SENTRY_DSN}\"")
endfunction()

if(WITH_CRASH_REPORTING AND WITH_CLIENT)
  include(FetchContent)

  if(ENABLE_SANITIZERS)
    # The crash handler cannot run under ASan/UBSan, so there is no backend to capture with.
    set(_sentry_backend "none")
  else()
    # Breakpad on every OS (increment 3 probe, 2026-10-14): the crashpad sources
    # do vendor friction-free (FetchContent inits all submodules recursively by
    # default, verified populated in a local build tree), but the crashpad backend
    # would need its out-of-process crashpad_handler binary installed next to the
    # exe, into the macOS bundle, and through NSIS, plus handler-path wiring and
    # notarization care. Breakpad is in-process, needs no extra binary and no
    # packaging on any OS sentry-native builds, so simplicity wins.
    set(_sentry_backend "breakpad")
  endif()
  # FORCE so toggling ENABLE_SANITIZERS on a reused build dir cannot leave a stale backend behind.
  set(SENTRY_BACKEND
      "${_sentry_backend}"
      CACHE STRING "Crash handler backend for sentry-native" FORCE
  )
  unset(_sentry_backend)

  if(NOT MSVC)
    # Symbol upload needs debug info in Release binaries: the top-level flags build GCC
    # Release as -s -O2 (stripped, no DWARF) and Clang/Apple Release as plain -O2 (no DWARF),
    # both of which upload nothing useful. Keep -g and drop the strip flag while the SDK is
    # on; the OFF default leaves local, distro and Debug builds untouched. MSVC already
    # links /DEBUG with PDBs in release (see top-level CMakeLists.txt), so it needs nothing.
    string(REGEX REPLACE "(^| )-s( |$)" "\\1\\2" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    if(NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "(^| )-g( |$)")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    endif()
  endif()

  # No Qt integration: it runs find_package(Qt6) inside the vendored build, which
  # cannot see our thin/aqt Qt layouts (macOS CI) under foreign toolchains, and its
  # breadcrumbs could carry unredacted log lines into native crash events that bypass
  # before_send. Our attached redacted log preview already carries the log context.
  set(SENTRY_INTEGRATION_QT
      OFF
      CACHE BOOL "Qt log-message breadcrumbs for sentry-native (off: discovery + secret surface)" FORCE
  )
  # The transport stays at the SDK default (curl on Apple/Linux, winhttp on Windows).
  set(SENTRY_BUILD_TESTS
      OFF
      CACHE BOOL "Build sentry-native tests" FORCE
  )
  set(SENTRY_BUILD_EXAMPLES
      OFF
      CACHE BOOL "Build sentry-native examples" FORCE
  )

  fetchcontent_declare(
    sentry
    GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
    GIT_TAG ${COCKATRICE_SENTRY_TAG}
    GIT_SHALLOW TRUE
  )
  fetchcontent_makeavailable(sentry)

  # Vendored sentry-native (and its breakpad/crashpad trees) warn under our -Wall -Wextra
  # (e.g. breakpad's -Wmissing-field-initializers), and WARNING_AS_ERROR would fail the build
  # on code we cannot fix. Those findings are not actionable for us, so opt the whole vendored
  # tree out of warnings-as-errors while keeping the warnings themselves visible; our own
  # targets keep -Werror. Interface libraries compile nothing, so they are skipped outright:
  # an INTERFACE -Wno-error would propagate to our consumers and weaken their build.
  function(_cockatrice_sentry_opt_out_of_werror _dir)
    get_property(
      _sentry_targets
      DIRECTORY "${_dir}"
      PROPERTY BUILDSYSTEM_TARGETS
    )
    foreach(_sentry_target IN LISTS _sentry_targets)
      get_target_property(_sentry_type ${_sentry_target} TYPE)
      if(_sentry_type STREQUAL "INTERFACE_LIBRARY")
        continue()
      elseif(_sentry_type MATCHES "LIBRARY|EXECUTABLE")
        get_target_property(_sentry_imported ${_sentry_target} IMPORTED)
        if(NOT _sentry_imported)
          target_compile_options(${_sentry_target} PRIVATE -Wno-error)
        endif()
      endif()
    endforeach()
    get_property(
      _sentry_subdirs
      DIRECTORY "${_dir}"
      PROPERTY SUBDIRECTORIES
    )
    foreach(_sentry_subdir IN LISTS _sentry_subdirs)
      _cockatrice_sentry_opt_out_of_werror("${_sentry_subdir}")
    endforeach()
  endfunction()
  if(NOT MSVC)
    _cockatrice_sentry_opt_out_of_werror("${sentry_SOURCE_DIR}")
  endif()

  message(STATUS "Crash reporting: sentry-native ${COCKATRICE_SENTRY_TAG} (backend: ${SENTRY_BACKEND})")
else()
  message(STATUS "Crash reporting: SDK not vendored "
                 "(WITH_CRASH_REPORTING=${WITH_CRASH_REPORTING}, WITH_CLIENT=${WITH_CLIENT})"
  )
endif()
