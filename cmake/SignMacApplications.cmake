# This script re-signs all apps after CPack packages them. This is necessary because CPack modifies
# the library references used by Cockatrice to App relative paths, invalidating the code signature.
string(LENGTH "$ENV{MACOS_CERTIFICATE_NAME}" MACOS_CERTIFICATE_NAME_LEN)

if(APPLE AND MACOS_CERTIFICATE_NAME_LEN GREATER 0)
  set(APPLICATIONS "cockatrice" "servatrice" "oracle")
  foreach(app_name IN LISTS APPLICATIONS)
    set(FULL_APP_PATH "${CPACK_TEMPORARY_INSTALL_DIRECTORY}/${app_name}.app")

    message(STATUS "Signing Interior Dynamically Loaded Libraries for ${app_name}.app")
    execute_process(COMMAND "find" "${FULL_APP_PATH}" "-name" "*.dylib" OUTPUT_VARIABLE INTERIOR_DLLS)
    string(REPLACE "\n" ";" INTERIOR_DLLS_LIST ${INTERIOR_DLLS})

    foreach(INTERIOR_DLL IN LISTS INTERIOR_DLLS_LIST)
      execute_process(
        COMMAND "codesign" "--sign" "$ENV{MACOS_CERTIFICATE_NAME}" "--entitlements" "../.ci/macos.entitlements"
                "--options" "runtime" "--force" "--deep" "--timestamp" "--verbose" "${INTERIOR_DLL}"
      )
    endforeach()

    message(STATUS "Signing Exterior Applications ${app_name}.app")
    execute_process(
      COMMAND "codesign" "--sign" "$ENV{MACOS_CERTIFICATE_NAME}" "--entitlements" "../.ci/macos.entitlements"
              "--options" "runtime" "--force" "--deep" "--timestamp" "--verbose" "${FULL_APP_PATH}"
    )
  endforeach()
endif()
