# ClangFormat.cmake
#
# Adds two convenience targets that delegate to cmake/RunClangFormat.cmake
# (single source of truth, shared with the CI format-check job):
#
#   cmake --build build --target FormatFix     # reformat all sources in place
#   cmake --build build --target FormatCheck   # verify formatting (fails on violations)
#
# The targets are ALWAYS created so they show up in the IDE. clang-format is
# often not on PATH on Windows but ships with Visual Studio under
# VC/Tools/Llvm/bin, so we search there too. If it still can't be found at
# configure time, the targets are left in place and RunClangFormat.cmake emits
# a clear error only if/when they are actually built -- a normal build that
# never invokes them is unaffected.

file(GLOB _vs_llvm_dirs
	"$ENV{ProgramFiles}/Microsoft Visual Studio/*/*/VC/Tools/Llvm/bin"
	"$ENV{ProgramW6432}/Microsoft Visual Studio/*/*/VC/Tools/Llvm/bin"
	"$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/*/*/VC/Tools/Llvm/bin")
find_program(CLANG_FORMAT_EXE
	NAMES clang-format
	HINTS ${_vs_llvm_dirs}
	PATHS
		"$ENV{ProgramFiles}/LLVM/bin"
		"$ENV{ProgramW6432}/LLVM/bin")

if(CLANG_FORMAT_EXE)
	message(STATUS "clang-format found: ${CLANG_FORMAT_EXE} ('format'/'format-check' targets enabled)")
	# Forward the resolved path so the targets don't have to re-discover it.
	set(_cf_arg "-DCLANG_FORMAT_EXE=${CLANG_FORMAT_EXE}")
else()
	message(STATUS "clang-format not found at configure time; 'format'/'format-check' targets will look for it on PATH when built.")
	set(_cf_arg "")
endif()

set(_run_script "${CMAKE_CURRENT_LIST_DIR}/RunClangFormat.cmake")

add_custom_target(FormatFix
	COMMAND ${CMAKE_COMMAND} ${_cf_arg}
		-P "${_run_script}" -- --fix
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Reformatting sources with clang-format")

add_custom_target(FormatCheck
	COMMAND ${CMAKE_COMMAND} ${_cf_arg}
		-P "${_run_script}" -- --check
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMENT "Checking source formatting with clang-format")

# Group FormatFix / FormatCheck alongside ALL_BUILD / ZERO_CHECK / INSTALL in the IDE
# (Visual Studio, Xcode). USE_FOLDERS is enabled globally in Environment.cmake.
# CMAKE_PREDEFINED_TARGETS_FOLDER defaults to "CMakePredefinedTargets" when unset.
# Harmless no-op for non-IDE generators like Ninja.
if(CMAKE_PREDEFINED_TARGETS_FOLDER)
	set(_predef_folder "${CMAKE_PREDEFINED_TARGETS_FOLDER}")
else()
	set(_predef_folder "CMakePredefinedTargets")
endif()
set_target_properties(FormatFix FormatCheck PROPERTIES FOLDER "${_predef_folder}")
