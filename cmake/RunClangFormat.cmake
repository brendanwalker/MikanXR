# RunClangFormat.cmake
#
# Standalone clang-format driver, runnable WITHOUT a configured build tree:
#
#   cmake -P cmake/RunClangFormat.cmake -- --check   # verify (nonzero exit on violations)
#   cmake -P cmake/RunClangFormat.cmake -- --fix     # reformat in place
#
# This same script backs the `format` / `format-check` CMake targets (see
# cmake/ClangFormat.cmake) and the CI format-check job, so local and CI always
# run identical logic. Sources are globbed at run time (under src/ only), so
# newly added files are always covered and thirdparty/ is never touched.
#
# Override the tool with -DCLANG_FORMAT_EXE=/path/to/clang-format before -P.

cmake_minimum_required(VERSION 3.15)

# This script lives in <repo>/cmake, so the repo root is one level up.
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# -- Parse mode (default: check) --------------------------------------------
set(MODE "check")
math(EXPR _argc_minus_1 "${CMAKE_ARGC} - 1")
foreach(i RANGE 0 ${_argc_minus_1})
	if("${CMAKE_ARGV${i}}" STREQUAL "--fix")
		set(MODE "fix")
	elseif("${CMAKE_ARGV${i}}" STREQUAL "--check")
		set(MODE "check")
	endif()
endforeach()

# -- Locate clang-format ----------------------------------------------------
if(NOT CLANG_FORMAT_EXE)
	find_program(CLANG_FORMAT_EXE NAMES clang-format)
endif()
if(NOT CLANG_FORMAT_EXE)
	message(FATAL_ERROR
		"clang-format not found. Install it (e.g. `pip install clang-format==19.1.5`, "
		"or use the copy bundled with Visual Studio 2022) and ensure it is on PATH, "
		"or pass -DCLANG_FORMAT_EXE=/path/to/clang-format.")
endif()

execute_process(COMMAND "${CLANG_FORMAT_EXE}" --version
	OUTPUT_VARIABLE _cf_version OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Using ${_cf_version} (${CLANG_FORMAT_EXE})")

# -- Collect sources under src/ ---------------------------------------------
file(GLOB_RECURSE SOURCE_FILES
	"${REPO_ROOT}/src/*.cpp"
	"${REPO_ROOT}/src/*.c"
	"${REPO_ROOT}/src/*.h"
	"${REPO_ROOT}/src/*.hpp"
	"${REPO_ROOT}/src/*.inl")

list(LENGTH SOURCE_FILES _file_count)
if(_file_count EQUAL 0)
	message(FATAL_ERROR "No source files found under ${REPO_ROOT}/src")
endif()
message(STATUS "clang-format ${MODE}: ${_file_count} files under src/")

# -- Run in batches (avoids Windows command-line length limits) -------------
set(BATCH_SIZE 100)
set(_had_violations FALSE)
set(_batch "")
set(_in_batch 0)

function(_run_batch files)
	if("${files}" STREQUAL "")
		return()
	endif()
	if(MODE STREQUAL "fix")
		execute_process(
			COMMAND "${CLANG_FORMAT_EXE}" --style=file -i ${files}
			WORKING_DIRECTORY "${REPO_ROOT}"
			RESULT_VARIABLE _rv)
		if(NOT _rv EQUAL 0)
			message(FATAL_ERROR "clang-format failed while fixing (exit ${_rv})")
		endif()
	else()
		# --dry-run --Werror: exit nonzero and print diagnostics if any file
		# would be reformatted, without modifying anything.
		execute_process(
			COMMAND "${CLANG_FORMAT_EXE}" --style=file --dry-run --Werror ${files}
			WORKING_DIRECTORY "${REPO_ROOT}"
			RESULT_VARIABLE _rv)
		if(NOT _rv EQUAL 0)
			set(_had_violations TRUE PARENT_SCOPE)
		endif()
	endif()
endfunction()

foreach(_file IN LISTS SOURCE_FILES)
	list(APPEND _batch "${_file}")
	math(EXPR _in_batch "${_in_batch} + 1")
	if(_in_batch GREATER_EQUAL ${BATCH_SIZE})
		_run_batch("${_batch}")
		if(_had_violations)
			set(_any_violations TRUE)
		endif()
		set(_batch "")
		set(_in_batch 0)
	endif()
endforeach()
_run_batch("${_batch}")
if(_had_violations)
	set(_any_violations TRUE)
endif()

# -- Report -----------------------------------------------------------------
if(MODE STREQUAL "fix")
	message(STATUS "clang-format: reformatting complete.")
elseif(_any_violations)
	message(FATAL_ERROR
		"clang-format check failed: some files are not formatted. "
		"Run `cmake --build build --target format` (or "
		"`cmake -P cmake/RunClangFormat.cmake -- --fix`) to fix them.")
else()
	message(STATUS "clang-format check passed: all files conform.")
endif()
