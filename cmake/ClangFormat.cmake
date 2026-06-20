# ClangFormat.cmake
#
# Adds two convenience targets that delegate to cmake/RunClangFormat.cmake
# (single source of truth, shared with the CI format-check job):
#
#   cmake --build build --target format         # reformat all sources in place
#   cmake --build build --target format-check   # verify formatting (fails on violations)
#
# Both targets are skipped silently if clang-format cannot be found, so a
# missing local tool never breaks a normal build.

find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(NOT CLANG_FORMAT_EXE)
	message(STATUS "clang-format not found; 'format' and 'format-check' targets disabled.")
	return()
endif()

message(STATUS "clang-format found: ${CLANG_FORMAT_EXE} ('format'/'format-check' targets enabled)")

set(_run_script "${CMAKE_CURRENT_LIST_DIR}/RunClangFormat.cmake")

add_custom_target(format
	COMMAND ${CMAKE_COMMAND}
		-DCLANG_FORMAT_EXE=${CLANG_FORMAT_EXE}
		-P "${_run_script}" -- --fix
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Reformatting sources with clang-format")

add_custom_target(format-check
	COMMAND ${CMAKE_COMMAND}
		-DCLANG_FORMAT_EXE=${CLANG_FORMAT_EXE}
		-P "${_run_script}" -- --check
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Checking source formatting with clang-format")
