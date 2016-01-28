set(VERSION_STRING_CPP "${PROJECT_BINARY_DIR}/version_string.cpp")
set(VERSION_STRING_H "${PROJECT_BINARY_DIR}/version_string.h")
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

set( hstring "extern const char *VERSION_STRING\;
extern const char *VERSION_DATE\;\n" )
set( cppstring "const char *VERSION_STRING = \"${PROJECT_VERSION_FRIENDLY}\"\;
const char *VERSION_DATE = \"${GIT_COMMIT_DATE_FRIENDLY}\"\;\n")

file(WRITE ${PROJECT_BINARY_DIR}/version_string.cpp.txt ${cppstring} )
file(WRITE ${PROJECT_BINARY_DIR}/version_string.h.txt ${hstring} )

execute_process(
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_BINARY_DIR}/version_string.h.txt ${VERSION_STRING_H}
)
execute_process(
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_BINARY_DIR}/version_string.cpp.txt ${VERSION_STRING_CPP}
)

