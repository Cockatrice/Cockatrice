# Crash reporting via sentry-native (Sentry protocol, MIT).
# Included once from the root CMakeLists.txt, right after the sanitizer/coverage helpers.
#
# Inputs:
#   WITH_CRASH_REPORTING -- build the client with the sentry-native SDK (default OFF, our release
#                           CI passes ON; packagers and from-source builds stay SDK-free until this
#                           ships something real, so offline package builds never fetch the SDK)
#   COCKATRICE_SENTRY_DSN -- Sentry DSN baked into the build (default empty = SDK init skipped at runtime)
# Defines: cockatrice_target_enable_crash_reporting(<target>)

option(WITH_CRASH_REPORTING "Build with crash reporting via sentry-native" OFF)
set(COCKATRICE_SENTRY_DSN
    ""
    CACHE STRING "Sentry DSN baked into the client; empty disables crash-report upload at runtime"
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
  # Sentry release for events from this target: PROJECT_VERSION (getversion.cmake ran before this
  # file was included). Deliberately not PROJECT_VERSION_FRIENDLY / VERSION_STRING — that carries
  # a build date, so every build of the same version would land in a different Sentry release.
  # PROJECT_VERSION matches what the installer reports in its own log and what CPack derives the
  # package version from, so client and installer events group under one release.
  target_compile_definitions(${_target} PRIVATE "COCKATRICE_SENTRY_RELEASE=\"${PROJECT_VERSION}\"")
endfunction()

if(WITH_CRASH_REPORTING AND WITH_CLIENT)
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

  find_package(sentry CONFIG REQUIRED)
  message(STATUS "Crash reporting: sentry-native ${sentry_VERSION}")
else()
  message(STATUS "Crash reporting: SDK not vendored "
                 "(WITH_CRASH_REPORTING=${WITH_CRASH_REPORTING}, WITH_CLIENT=${WITH_CLIENT})"
  )
endif()

# Packaged release binaries must not ship the DWARF that the symbol upload needs: it is
# 80-90% of an unstripped binary and nobody downloads a 500 MB installer to get it. The
# build tree keeps -g (CI uploads symbols from there); the *packaged copy* is split at
# install time by a CPack install script, which extracts the debug info next to the staging
# dir and strips the binary. Only ELF (Linux/BSD) needs this: Windows links /DEBUG with
# separate PDBs, and macOS keeps its dSYM extracted from the bundle.
option(COCKATRICE_SPLIT_DEBUG_SYMBOLS "Strip DWARF out of packaged release binaries" ON)
set(COCKATRICE_DEBUG_SYMBOL_DIR
    "${CMAKE_BINARY_DIR}/debug-symbols"
    CACHE PATH "Where the packaged binaries' split debug information is written (kept out of the package)"
)
if(WITH_CRASH_REPORTING
   AND WITH_CLIENT
   AND COCKATRICE_SPLIT_DEBUG_SYMBOLS
   AND CMAKE_BUILD_TYPE STREQUAL "Release"
   AND UNIX
   AND NOT APPLE
)
  find_program(COCKATRICE_OBJCOPY NAMES objcopy)
  find_program(COCKATRICE_STRIP_TOOL NAMES strip)
  if(COCKATRICE_OBJCOPY AND COCKATRICE_STRIP_TOOL)
    configure_file(
      "${CMAKE_CURRENT_LIST_DIR}/SplitDebugSymbols.cmake.in" "${CMAKE_BINARY_DIR}/cmake/SplitDebugSymbols.cmake" @ONLY
    )
    # PRE_BUILD runs after CPack stages the install tree and before the package is produced.
    # CPACK_INSTALL_SCRIPTS would run too early (staging is still empty); the script walks
    # CPACK_TEMPORARY_DIRECTORY, never CMAKE_INSTALL_PREFIX, and refuses to run otherwise.
    list(APPEND CPACK_PRE_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/cmake/SplitDebugSymbols.cmake")
    message(STATUS "Crash reporting: packaged binaries stripped, debug info split to ${COCKATRICE_DEBUG_SYMBOL_DIR}")
  else()
    message(WARNING "COCKATRICE_SPLIT_DEBUG_SYMBOLS is ON but objcopy/strip were not found; "
                    "packaged release binaries will keep their debug information"
    )
  endif()
endif()

# Installer failure reporting (increment 4): parse COCKATRICE_SENTRY_DSN into the parts the Windows installer
# needs. These are CPACK_* so CPack snapshots them at include(CPack) time and substitutes them into
# cmake/NSIS.template.in at package time; on non-Windows they are inert (only the NSIS template consumes them).
# Set from COCKATRICE_SENTRY_DSN on every configure, so a changed or removed DSN can never leave a stale value
# behind. Plain variables, not CACHE: they only need to be in scope when include(CPack) later snapshots every
# CPACK_* variable into CPackConfig.cmake.
set(CPACK_SENTRY_ENABLED "0")
set(CPACK_SENTRY_KEY "")
set(CPACK_SENTRY_ENVELOPE_URL "")
set(CPACK_SENTRY_DSN_FULL "")
# Deliberately PROJECT_VERSION: that is what the client bakes as its Sentry release and what CI passes to
# `sentry-cli debug-files upload`, so installer and client events group under one release. It is not the
# CPACK_PACKAGE_VERSION the installer logs in its own log line — on Development tags PROJECT_VERSION carries a
# "-<label>" suffix (getversion.cmake) while CPACK_PACKAGE_VERSION is only MAJOR.MINOR.PATCH.
set(CPACK_SENTRY_RELEASE "${PROJECT_VERSION}")

# Canonical <scheme>://<key>@<host>/<project-id>. Deliberately unanchored, so a trailing slash still matches.
# A self-hosted DSN with a path prefix (e.g. GlitchTip behind a subpath) does not match and falls through to the
# disabled-and-warn branch instead of deriving a wrong endpoint from the tail of the path.
if(COCKATRICE_SENTRY_DSN MATCHES "(https?)://([^@]+)@([^/]+)/([0-9]+)")
  set(CPACK_SENTRY_ENABLED "1")
  set(CPACK_SENTRY_KEY "${CMAKE_MATCH_2}")
  # Scheme comes from the DSN, not an assumption: self-hosted (GlitchTip) DSNs may be plain http.
  set(CPACK_SENTRY_ENVELOPE_URL "${CMAKE_MATCH_1}://${CMAKE_MATCH_3}/api/${CMAKE_MATCH_4}/envelope/")
  set(CPACK_SENTRY_DSN_FULL "${COCKATRICE_SENTRY_DSN}")
elseif(NOT COCKATRICE_SENTRY_DSN STREQUAL "")
  message(WARNING "COCKATRICE_SENTRY_DSN is set but malformed "
                  "(expected <scheme>://<key>@<host>/<project-id>); installer log reporting disabled"
  )
endif()
