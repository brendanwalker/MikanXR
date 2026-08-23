# Step into the subdirectories
# C# is only supported by the Visual Studio generators. Under Ninja
# (used in CI for sccache) enabling the CSharp language fails, so skip
# the C# bindings there. They are not part of the CI test targets.
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  MESSAGE(STATUS "Stepping into csharp")
  add_subdirectory(csharp)
else()
  MESSAGE(STATUS "Skipping csharp bindings (requires a Visual Studio generator)")
endif()

MESSAGE(STATUS "Stepping into typescript")
add_subdirectory(typescript)