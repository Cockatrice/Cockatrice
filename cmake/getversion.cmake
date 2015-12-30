find_package(Git)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} log -1 --date=short "--pretty=%h"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE res_var
		OUTPUT_VARIABLE GIT_COM_ID
	)
	if( NOT ${res_var} EQUAL 0 )
		set( GIT_COMMIT_ID "unknown")
		message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
	endif()
	string( REPLACE "\n" "" GIT_COMMIT_ID "${GIT_COM_ID}" )

	execute_process(
		COMMAND ${GIT_EXECUTABLE} log -1 --date=short "--pretty=%cd"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE res_var
		OUTPUT_VARIABLE GIT_COM_DATE
	)
	if( NOT ${res_var} EQUAL 0 )
		set( GIT_COMMIT_DATE "unknown")
		set( GIT_COMMIT_DATE_FRIENDLY "unknown")
		message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
	endif()
	string( REPLACE "\n" "" GIT_COMMIT_DATE_FRIENDLY "${GIT_COM_DATE}" )
	string( REPLACE "-" "" GIT_COMMIT_DATE "${GIT_COMMIT_DATE_FRIENDLY}" )
else()
	set( GIT_COMMIT_ID "unknown")
	set( GIT_COMMIT_DATE "unknown")
	set( GIT_COMMIT_DATE_FRIENDLY "unknown")
	message( WARNING "Git not found. Build will not contain git revision info." )
endif()

set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "1~git${GIT_COMMIT_DATE}.${GIT_COMMIT_ID}")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_VERSION_FRIENDLY "${GIT_COMMIT_ID} (${GIT_COMMIT_DATE_FRIENDLY})")

