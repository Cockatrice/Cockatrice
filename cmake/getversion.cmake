# HELPER FUNCTIONS

function(get_commit_id)
  # get last commit hash
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --abbrev=7 --date=short "--pretty=%h"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE res_var
    OUTPUT_VARIABLE GIT_COM_ID
  )
  if(NOT ${res_var} EQUAL 0)
    message(WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info.")
    return()
  endif()

  string(REPLACE "\n" "" GIT_COM_ID "${GIT_COM_ID}")
  set(GIT_COMMIT_ID
      "${GIT_COM_ID}"
      PARENT_SCOPE
  )
  set(PROJECT_VERSION_LABEL
      "custom-${GIT_COM_ID}"
      PARENT_SCOPE
  )
endfunction()

function(get_commit_date)
  # get last commit date
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --date=short "--pretty=%cd"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE res_var
    OUTPUT_VARIABLE GIT_COM_DATE
  )
  if(NOT ${res_var} EQUAL 0)
    message(WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info.")
    return()
  endif()

  string(REPLACE "\n" "" GIT_COM_DATE "${GIT_COM_DATE}")
  set(GIT_COMMIT_DATE_FRIENDLY
      "${GIT_COM_DATE}"
      PARENT_SCOPE
  )

  string(REPLACE "-" "" GIT_COM_DATE "${GIT_COM_DATE}")
  set(GIT_COMMIT_DATE
      "${GIT_COM_DATE}"
      PARENT_SCOPE
  )
endfunction()

function(get_tag_name commit)
  if(${commit} STREQUAL "unknown")
    return()
  endif()

  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --exact-match --tags ${commit}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE res_var
    OUTPUT_VARIABLE GIT_TAG
    ERROR_VARIABLE GIT_TAG_ERR
  )

  if((NOT ${res_var} EQUAL 0) OR (${GIT_TAG_ERR} MATCHES "fatal: no tag exactly matches.*"))
    message(STATUS "Commit is not a release or prerelease (no git tag found)")
    return()
  endif()

  string(REPLACE "\n" "" GIT_TAG "${GIT_TAG}")
  message(STATUS "Commit is a release or prerelease, git tag: ${GIT_TAG}")

  # Extract information from tag:
  # MAJ.MIN.PATCH
  # MAJ.MIN.PATCH-beta or MAJ.MIN.PATCH-beta.X
  string(REPLACE "-" ";" GIT_TAG_EXPLODED "${GIT_TAG}")
  string(REPLACE "." ";" GIT_TAG_EXPLODED "${GIT_TAG_EXPLODED}")

  # Sanity checks: length
  list(LENGTH GIT_TAG_EXPLODED GIT_TAG_LISTCOUNT)
  if(${GIT_TAG_LISTCOUNT} LESS 3 OR ${GIT_TAG_LISTCOUNT} GREATER 5)
    message(WARNING "Invalid tag format, got ${GIT_TAG_LISTCOUNT} tokens")
    return()
  endif()

  # Major
  list(GET GIT_TAG_EXPLODED 0 GIT_TAG_MAJOR)
  if(${GIT_TAG_MAJOR} LESS 0 OR ${GIT_TAG_MAJOR} GREATER 99)
    message(WARNING "Invalid tag major version ${GIT_TAG_MAJOR}")
    return()
  endif()

  # Minor
  list(GET GIT_TAG_EXPLODED 1 GIT_TAG_MINOR)
  if(${GIT_TAG_MINOR} LESS 0 OR ${GIT_TAG_MINOR} GREATER 99)
    message(WARNING "Invalid tag minor version ${GIT_TAG_MINOR}")
    return()
  endif()

  # Patch
  list(GET GIT_TAG_EXPLODED 2 GIT_TAG_PATCH)
  if(${GIT_TAG_PATCH} LESS 0 OR ${GIT_TAG_PATCH} GREATER 99)
    message(WARNING "Invalid tag patch version ${GIT_TAG_PATCH}")
    return()
  endif()

  # Label
  # 3 Elements = No label = Stable release
  # 4 Elements = Beta label = Dev release, first beta
  # 5 Elements = Beta label = Dev release, subsequent beta
  if(${GIT_TAG_LISTCOUNT} EQUAL 4)
    list(GET GIT_TAG_EXPLODED 3 GIT_TAG_LABEL)
  elseif(${GIT_TAG_LISTCOUNT} EQUAL 5)
    list(GET GIT_TAG_EXPLODED 3 GIT_TAG_LABEL)
    list(GET GIT_TAG_EXPLODED 4 GIT_TAG_LABEL_NUM)
    set(GIT_TAG_LABEL ${GIT_TAG_LABEL} ${GIT_TAG_LABEL_NUM})
    string(REPLACE ";" "." GIT_TAG_LABEL "${GIT_TAG_LABEL}")
  else()
    set(GIT_TAG_LABEL "")
  endif()

  # Override hardcoded version with the informations from the tag
  set(PROJECT_VERSION_MAJOR
      ${GIT_TAG_MAJOR}
      PARENT_SCOPE
  )
  set(PROJECT_VERSION_MINOR
      ${GIT_TAG_MINOR}
      PARENT_SCOPE
  )
  set(PROJECT_VERSION_PATCH
      ${GIT_TAG_PATCH}
      PARENT_SCOPE
  )
  set(PROJECT_VERSION_LABEL
      ${GIT_TAG_LABEL}
      PARENT_SCOPE
  )

  if(${GIT_TAG_LISTCOUNT} EQUAL 3)
    # set release name from env var
    set(PROJECT_VERSION_RELEASENAME
        "${GIT_TAG_RELEASENAME}"
        PARENT_SCOPE
    )
  endif()

endfunction()

# START OF MAIN

# fallback defaults
set(GIT_COMMIT_ID "unknown")
set(GIT_COMMIT_DATE "")
set(GIT_COMMIT_DATE_FRIENDLY "")
set(PROJECT_VERSION_LABEL "")
set(PROJECT_VERSION_RELEASENAME "")

find_package(Git)
if(GIT_FOUND)
  get_commit_id()
  get_commit_date()
  get_tag_name(${GIT_COMMIT_ID})
else()
  message(WARNING "Git not found. Build will not contain git revision info.")
endif()

set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
if(PROJECT_VERSION_LABEL)
  set(PROJECT_VERSION "${PROJECT_VERSION}-${PROJECT_VERSION_LABEL}")
endif()

set(PROJECT_VERSION_FRIENDLY "${PROJECT_VERSION} (${GIT_COMMIT_DATE_FRIENDLY})")

# Format: <program name>[-ReleaseName]-MAJ.MIN.PATCH[-prerelease_label]
set(PROJECT_VERSION_FILENAME "${PROJECT_NAME}")
if(PROJECT_VERSION_RELEASENAME)
  set(PROJECT_VERSION_FILENAME "${PROJECT_VERSION_FILENAME}-${PROJECT_VERSION_RELEASENAME}")
endif()
set(PROJECT_VERSION_FILENAME "${PROJECT_VERSION_FILENAME}-${PROJECT_VERSION}")

message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Friendly project version: ${PROJECT_VERSION_FRIENDLY}")
message(STATUS "Project version filename: ${PROJECT_VERSION_FILENAME}")
