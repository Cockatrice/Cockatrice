find_package(Git)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --long --always
		RESULT_VARIABLE res_var
		OUTPUT_VARIABLE GIT_COM_ID
	)
	if( NOT ${res_var} EQUAL 0 )
		set( GIT_COMMIT_ID "git commit id unknown")
		message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
	endif()
	string( REPLACE "\n" "" GIT_COMMIT_ID "${GIT_COM_ID}" )
else()
	set( GIT_COMMIT_ID "unknown (git not found!)")
	message( WARNING "Git not found. Build will not contain git revision info." )
endif()

set( hstring "extern const char *VERSION_STRING\;\n" )
set( cppstring "const char * VERSION_STRING = \"${GIT_COMMIT_ID}\"\;\n")

file(WRITE version_string.cpp.txt ${cppstring} )
file(WRITE version_string.h.txt ${hstring} )

execute_process(
	COMMAND ${CMAKE_COMMAND} -E copy_if_different version_string.h.txt ${CMAKE_CURRENT_BINARY_DIR}/version_string.h
)
execute_process(
	COMMAND ${CMAKE_COMMAND} -E copy_if_different version_string.cpp.txt ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
)
