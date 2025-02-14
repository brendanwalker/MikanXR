PROJECT(MikanClientCSharp.Tests VERSION 1.0.0.0 LANGUAGES CSharp)
INCLUDE(CSharpUtilities)

# Create assembly info with current version.
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/bindings/csharp/AssemblyInfo.cs.template" "${CMAKE_BINARY_DIR}/Config/${PROJECT_NAME}/AssemblyInfo.cs")

file(GLOB MIKANCLIENT_CSHARP_TESTS_SRC
	"${CMAKE_CURRENT_LIST_DIR}/*.cs"
)

ADD_LIBRARY(${PROJECT_NAME} SHARED
    "${CMAKE_BINARY_DIR}/Config/${PROJECT_NAME}/AssemblyInfo.cs"
    ${MIKANCLIENT_CSHARP_TESTS_SRC}
)

# Find Nuget (install the latest CLI here: https://www.nuget.org/downloads).
find_program(NUGET nuget PATHS ${NUGET_PATH})
if(NOT NUGET)
    message(FATAL "CMake could not find the nuget command line tool. Please install it!")
else()
    # Copy the Nuget config file from source location to the CMake build directory.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config.template 
	${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)
    # Run Nuget using the .config file to installing any missing dependencies to the build directory.
	execute_process(COMMAND 
        ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Provide the path to the Nuget-installed references, in case this is a fresh project build.
set_property(TARGET ${PROJECT_NAME} PROPERTY 
    VS_DOTNET_REFERENCE_Newtonsoft.Json 
    ${CMAKE_BINARY_DIR}/packages/Newtonsoft.Json.4.0.3/lib/net40/Newtonsoft.Json.dll)

# Provide the path to the Nuget-installed references, in case this is a fresh project build.
set_property(TARGET ${PROJECT_NAME} PROPERTY 
    VS_DOTNET_REFERENCE_NUnit 
    ${CMAKE_BINARY_DIR}/packages/NUnit.4.2.2/lib/net462/nunit.framework.dll)
	
# Reference the MikanClientCSharp DLL
set_property(TARGET ${PROJECT_NAME} PROPERTY 
	VS_DOTNET_REFERENCE_MikanClientCSharp
     "${CMAKE_BINARY_DIR}/MikanClientCSharp.dll")
	
# Set designer and XAML properties.
CSHARP_SET_DESIGNER_CS_PROPERTIES(
    "${CMAKE_BINARY_DIR}/Config/${PROJECT_NAME}/AssemblyInfo.cs"
)

# Set CLR assembly properties.
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DOTNET_REFERENCES "System;System.Core;System.Memory"
    VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
)

# Put in an "ClientAPI" folder in the project viewer
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)

# Build MikanClientCSharp dependency first
add_dependencies(${PROJECT_NAME} MikanClientCSharp)