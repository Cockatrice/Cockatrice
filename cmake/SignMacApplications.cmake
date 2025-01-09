# This script re-signs all apps after CPack packages them. This is necessary because CPack modifies
# the library references used by Cockatrice to App relative paths, invalidating the code signature.
if(APPLE)
    set(APPLICATIONS "cockatrice" "servatrice" "oracle" "dbconverter")
    foreach(app_name IN LISTS APPLICATIONS)
        set(FULL_APP_PATH "${CPACK_TEMPORARY_INSTALL_DIRECTORY}/${app_name}.app")
        message(STATUS "Re-signing ${app_name}.app")
        execute_process(COMMAND
                "codesign"
                "--sign"
                    "Developer ID Application: Zachary Halpern (STKV3NKYBF)"
                "--entitlements"
                    "../.ci/macos.entitlements"
                "--options"
                    "runtime"
                "--force"
                "--deep"
                "--timestamp"
                "--verbose"
                "${FULL_APP_PATH}")
    endforeach()
endif()