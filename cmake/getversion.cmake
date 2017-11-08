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
	set(GIT_COMMIT_ID "${GIT_COM_ID}" PARENT_SCOPE)
	set(PROJECT_VERSION_LABEL "custom_${GIT_COM_ID}" PARENT_SCOPE)
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
	set(GIT_COMMIT_DATE_FRIENDLY "${GIT_COM_DATE}" PARENT_SCOPE)

	string(REPLACE "-" "" GIT_COM_DATE "${GIT_COM_DATE}")
	set(GIT_COMMIT_DATE "${GIT_COM_DATE}" PARENT_SCOPE)
endfunction()

function(clean_release_name name)
	# "name": "Cockatrice: Thopter Pie Network, Revision 2"

	# Remove all double quotes
	STRING(REPLACE "\"" "" name "${name}")
	# Remove json prefix "name: "
	STRING(REPLACE "  name: " "" name "${name}")
	# Remove "cockatrice" name
	STRING(REPLACE "Cockatrice" "" name "${name}")
	# Remove all unwanted chars
	STRING(REGEX REPLACE "[^A-Za-z0-9_ ]" "" name "${name}")
	# Strip (trim) whitespaces
	STRING(STRIP "${name}" name)
	# Replace all spaces with underscores
	STRING(REPLACE " " "_" name "${name}")

	set(GIT_TAG_RELEASENAME "${name}" PARENT_SCOPE)
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
	# YYYY-MM-DD-Release-MAJ.MIN.PATCH
	# YYYY-MM-DD-Development-MAJ.MIN.PATCH-betaX
	string(REPLACE "-" ";" GIT_TAG_EXPLODED "${GIT_TAG}")
	string(REPLACE "." ";" GIT_TAG_EXPLODED "${GIT_TAG_EXPLODED}")

	# Sanity checks: length
	list(LENGTH GIT_TAG_EXPLODED GIT_TAG_LISTCOUNT)
	if(${GIT_TAG_LISTCOUNT} LESS 7 OR ${GIT_TAG_LISTCOUNT} GREATER 8)
		message(WARNING "Invalid tag format, got ${GIT_TAG_LISTCOUNT} tokens")
		return()
	endif()

	# Year
	list(GET GIT_TAG_EXPLODED 0 GIT_TAG_YEAR)
	if(${GIT_TAG_YEAR} LESS 2017 OR ${GIT_TAG_LISTCOUNT} GREATER 2100)
		message(WARNING "Invalid tag year ${GIT_TAG_YEAR}")
		return()
	endif()

	# Month
	list(GET GIT_TAG_EXPLODED 1 GIT_TAG_MONTH)
	if(${GIT_TAG_MONTH} LESS 1 OR ${GIT_TAG_MONTH} GREATER 12)
		message(WARNING "Invalid tag month ${GIT_TAG_MONTH}")
		return()
	endif()

	# Day
	list(GET GIT_TAG_EXPLODED 2 GIT_TAG_DAY)
	if(${GIT_TAG_DAY} LESS 1 OR ${GIT_TAG_DAY} GREATER 31)
		message(WARNING "Invalid tag day ${GIT_TAG_DAY}")
		return()
	endif()

	# Type
	list(GET GIT_TAG_EXPLODED 3 GIT_TAG_TYPE)
	if(NOT(${GIT_TAG_TYPE} STREQUAL "Release" OR ${GIT_TAG_TYPE} STREQUAL "Development"))
		message(WARNING "Invalid tag type ${GIT_TAG_TYPE}")
		return()
	endif()

	# Major
	list(GET GIT_TAG_EXPLODED 4 GIT_TAG_MAJOR)
	if(${GIT_TAG_MAJOR} LESS 0 OR ${GIT_TAG_MAJOR} GREATER 99)
		message(WARNING "Invalid tag major version ${GIT_TAG_MAJOR}")
		return()
	endif()

	# Minor
	list(GET GIT_TAG_EXPLODED 5 GIT_TAG_MINOR)
	if(${GIT_TAG_MINOR} LESS 0 OR ${GIT_TAG_MINOR} GREATER 99)
		message(WARNING "Invalid tag minor version ${GIT_TAG_MINOR}")
		return()
	endif()

	# Patch
	list(GET GIT_TAG_EXPLODED 6 GIT_TAG_PATCH)
	if(${GIT_TAG_PATCH} LESS 0 OR ${GIT_TAG_PATCH} GREATER 99)
		message(WARNING "Invalid tag patch version ${GIT_TAG_PATCH}")
		return()
	endif()

	# Label
	if(${GIT_TAG_LISTCOUNT} EQUAL 8)
		list(GET GIT_TAG_EXPLODED 7 GIT_TAG_LABEL)
	else()
		SET(GIT_TAG_LABEL "")
	endif()

	# Override hardcoded version with the informations from the tag
	set(PROJECT_VERSION_MAJOR ${GIT_TAG_MAJOR} PARENT_SCOPE)
	set(PROJECT_VERSION_MINOR ${GIT_TAG_MINOR} PARENT_SCOPE)
	set(PROJECT_VERSION_PATCH ${GIT_TAG_PATCH} PARENT_SCOPE)
	set(PROJECT_VERSION_LABEL ${GIT_TAG_LABEL} PARENT_SCOPE)

	if(${GIT_TAG_TYPE} STREQUAL "Development")
		set(PROJECT_VERSION_LABEL ${GIT_TAG_LABEL} PARENT_SCOPE)
	elseif(${GIT_TAG_TYPE} STREQUAL "Release")
		set(PROJECT_VERSION_LABEL "" PARENT_SCOPE)

		# get version name from github
		set(GIT_TAG_TEMP_FILE "${PROJECT_BINARY_DIR}/tag_informations.txt")
		set(GIT_TAG_TEMP_URL "https://api.github.com/repos/Cockatrice/Cockatrice/releases/tags/${GIT_TAG}")
		message(STATUS "Fetching tag informations from ${GIT_TAG_TEMP_URL}")
		file(REMOVE "${GIT_TAG_TEMP_FILE}")
		file(DOWNLOAD "${GIT_TAG_TEMP_URL}" "${GIT_TAG_TEMP_FILE}" STATUS status LOG log INACTIVITY_TIMEOUT 30 TIMEOUT 300 SHOW_PROGRESS)
    	list(GET status 0 err)
    	list(GET status 1 msg)
	    if(err)
	    	message(WARNING "Download failed with error ${msg}: ${log}")
	    	return()
    	endif()
    	file(STRINGS "${GIT_TAG_TEMP_FILE}" GIT_TAG_RAW_RELEASENAME REGEX "\"name\": \"" LIMIT_COUNT 1)

    	clean_release_name("${GIT_TAG_RAW_RELEASENAME}")
    	set(PROJECT_VERSION_RELEASENAME "${GIT_TAG_RELEASENAME}" PARENT_SCOPE)
	endif()

endfunction()

# START OF MAIN

# fallback defaults
set(GIT_COMMIT_ID "unknown")
set(GIT_COMMIT_DATE "unknown")
set(GIT_COMMIT_DATE_FRIENDLY "unknown")
set(PROJECT_VERSION_LABEL "custom_unknown")
set(PROJECT_VERSION_RELEASENAME "")

find_package(Git)
if(GIT_FOUND)
	get_commit_id()
	get_commit_date()
	get_tag_name(${GIT_COMMIT_ID})
else()
	message( WARNING "Git not found. Build will not contain git revision info." )
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
