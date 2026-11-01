# Windows installer written by Inno Setup, through CPack's own InnoSetup generator.
#
# Included from the WIN32 branch of the top-level CMakeLists.txt when
# COCKATRICE_WIN_INSTALLER is InnoSetup, which is the default. The NSIS template is kept
# next to this file rather than deleted so both installers can still be built from one tree
# and compared on a real upgrade.
#
# The generator writes [Files], [Dirs], [Icons], [Run] and [Registry] from the install()
# rules the project already declares, so nothing here repeats them. What is added is only
# what the generator has no opinion about: upgrade identity, closure behaviour for a
# running application, the stale-runtime [InstallDelete], and the migration off the NSIS
# installation (both script fragments below).

# The generator itself arrived in 3.27; cmake_minimum_required() here would reset the
# policies the top level already selected, so it is refused at the include site instead.
if(CMAKE_VERSION VERSION_LESS 3.27)
  message(FATAL_ERROR "COCKATRICE_WIN_INSTALLER=InnoSetup needs CMake >= 3.27 (found ${CMAKE_VERSION})")
endif()

# CPack may be told -DCPACK_GENERATOR=... by the caller. For the Windows installers the
# choice is made by COCKATRICE_WIN_INSTALLER instead (.ci/compile.sh --package NSIS or
# InnoSetup maps to it), because picking the generator here and the installer tool there can
# silently build both in one run. Only guard against asking for InnoSetup twice.
if(NOT "InnoSetup" IN_LIST CPACK_GENERATOR)
  set(CPACK_GENERATOR InnoSetup ${CPACK_GENERATOR})
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_INNOSETUP_ARCHITECTURE x64)
else()
  set(CPACK_INNOSETUP_ARCHITECTURE x86)
endif()

# Shortcuts and the finish-page run entry come from the executables CPack already knows.
# Only the client is offered: oracle.exe and servatrice.exe are tools, and the NSIS
# installer never created shortcuts for them either.
set(CPACK_PACKAGE_EXECUTABLES "cockatrice" "Cockatrice")
set(CPACK_INNOSETUP_RUN_EXECUTABLES "cockatrice")

set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
set(CPACK_INNOSETUP_PROGRAM_MENU_FOLDER "Cockatrice")
set(CPACK_INNOSETUP_ICON_FILE "${PROJECT_SOURCE_DIR}/cockatrice/resources/appicon.ico")
set(CPACK_INNOSETUP_IGNORE_README_PAGE ON)
# The uninstall entry is the only place a user sees the version they have after an update
# that replaced the folder, so keep it and give it the icon.
set(CPACK_INNOSETUP_CREATE_UNINSTALL_LINK ON)

# Upgrade identity. Never change this value: it is what makes a new build an upgrade of the
# previous one (same uninstall entry, same "previous app path", same tasks) instead of a
# second, parallel installation. No braces: a literal { in an .iss script opens a constant,
# and a GUID here buys nothing that this string does not already do.
set(CPACK_INNOSETUP_SETUP_AppId "Cockatrice-6F1C5B2A-9D84-4E37-B0A5-2C7E9A31D4F8")
set(CPACK_INNOSETUP_SETUP_AppName "Cockatrice")
set(CPACK_INNOSETUP_SETUP_DefaultGroupName "Cockatrice")

# An NSIS install registers nothing under Inno's own keys, so the default directory is
# resolved from the legacy registration instead of guessed. See GetDefaultInstallDir in
# cmake/InnoSetup.code.iss.
set(CPACK_INNOSETUP_SETUP_DefaultDirName "{code:GetDefaultInstallDir}")

set(CPACK_INNOSETUP_SETUP_Compression "lzma2/max")
set(CPACK_INNOSETUP_SETUP_SolidCompression ON)
# Replaces the CockLog/CockLogListing machinery: every run writes a log to %TEMP% without
# any script support, which is what a failed upgrade on a user's machine needs to be
# diagnosable after the fact.
set(CPACK_INNOSETUP_SETUP_SetupLogging ON)

set(CPACK_INNOSETUP_SETUP_PrivilegesRequired "admin")
# Users without administrator rights get a working installation instead of a dead end;
# /CURRENTUSER covers the same choice when driven by a package manager or the updater.
set(CPACK_INNOSETUP_SETUP_PrivilegesRequiredOverridesAllowed "dialog commandline")

# Restart Manager, not a PowerShell process scan. Setup detects the applications holding
# the files it is about to replace, closes them, and starts them again afterwards. This is
# the whole of what WaitForAppToClose/GuardAppsNotRunning/ForceCloseApps approximated in
# the NSIS template, including the uninstaller copies that had to repeat it.
set(CPACK_INNOSETUP_SETUP_CloseApplications ON)
set(CPACK_INNOSETUP_SETUP_RestartApplications ON)

# The payload is built for one architecture only; refuse to install it on a system that
# cannot run it. x64 (not x64compatible) keeps this valid on every Inno Setup 6.x.
if(CPACK_INNOSETUP_ARCHITECTURE STREQUAL "x64")
  set(CPACK_INNOSETUP_SETUP_ArchitecturesAllowed "x64")
  set(CPACK_INNOSETUP_SETUP_ArchitecturesInstallIn64BitMode "x64")
endif()

set(CPACK_INNOSETUP_SETUP_UninstallDisplayIcon "{app}\\cockatrice.exe")

# Sections the generator does not emit (the stale-runtime deletes), and the behaviour it
# cannot express (legacy migration, portable marker, C++ runtime bootstrap).
set(CPACK_INNOSETUP_EXTRA_SCRIPTS "${COCKATRICE_CMAKE_PATH}/InnoSetup.install.iss")
set(CPACK_INNOSETUP_CODE_FILES "${COCKATRICE_CMAKE_PATH}/InnoSetup.code.iss")

# The installer-log sender the failure hook in InnoSetup.code.iss offers after an upgrade
# that did not finish. CPack passes these as public ISCC macros (/Dname=value), so the extra
# script can embed the helper with a [Files] dontcopy entry and the code can extract it at
# send time. Undefined when the build carries no reporting, which is what the ifdef guards in
# both scripts test.
if(CPACK_SENTRY_ENABLED)
  set(CPACK_INNOSETUP_DEFINE_SentryHelperPath "${COCKATRICE_SENTRY_INSTALLER_LOG}")
endif()

message(STATUS "Windows installer: Inno Setup (CPack InnoSetup generator, ${CPACK_INNOSETUP_ARCHITECTURE})")
