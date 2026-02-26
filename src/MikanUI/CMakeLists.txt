cmake_minimum_required(VERSION 3.19)

project(MikanUI)

# Normalize CEF_ROOT path to use forward slashes to avoid escape character issues
file(TO_CMAKE_PATH "${CEF_ROOT}" CEF_ROOT)

# CEF Configuration
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Use the CEF CMake configuration
find_package(CEF REQUIRED)

list(APPEND MIKAN_UI_INCL_DIRS
  ${CEF_ROOT}
  ${CMAKE_CURRENT_LIST_DIR}/src/cpp
  ${MIKAN_LIBRARIES_DIR}/MikanUtility/Public)

list(APPEND MIKAN_UI_REQ_LIBS
  libcef_dll_wrapper
  "$<$<CONFIG:Debug>:${CEF_ROOT}/Debug/libcef.lib>"
  "$<$<CONFIG:Release>:${CEF_ROOT}/Release/libcef.lib>"
  shlwapi
  crypt32
  wintrust
  MikanUtility)

# C++ source files
file(GLOB MIKAN_UI_CPP_SOURCES src/cpp/*.cpp)
file(GLOB MIKAN_UI_CPP_HEADERS src/cpp/*.h)

# TypeScript source files (for IDE display)
file(GLOB MIKAN_UI_TS_SOURCES src/ts/*.ts)

# Web resource files
file(GLOB MIKAN_UI_RESOURCES_HTML resources/*.html)
file(GLOB MIKAN_UI_RESOURCES_CSS resources/css/*.css)
file(GLOB MIKAN_UI_RESOURCES_JS resources/js/*.js)

# Add executable target
add_executable(${PROJECT_NAME} WIN32
    ${MIKAN_UI_CPP_SOURCES}
    ${MIKAN_UI_CPP_HEADERS}
    ${MIKAN_UI_TS_SOURCES}
    ${MIKAN_UI_RESOURCES_HTML}
    ${MIKAN_UI_RESOURCES_CSS}
    ${MIKAN_UI_RESOURCES_JS}
)

# Organize files in Visual Studio filters
source_group("CPP" FILES ${MIKAN_UI_CPP_SOURCES} ${MIKAN_UI_CPP_HEADERS})
source_group("TypeScript" FILES ${MIKAN_UI_TS_SOURCES})
source_group("CSS" FILES ${MIKAN_UI_RESOURCES_CSS})
source_group("JavaScript" FILES ${MIKAN_UI_RESOURCES_JS})
source_group("HTML" FILES ${MIKAN_UI_RESOURCES_HTML})

# Set CEF properties
SET_EXECUTABLE_TARGET_PROPERTIES(${PROJECT_NAME})

# Add CEF include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${MIKAN_UI_INCL_DIRS})

# Link CEF libraries and Windows libraries
target_link_libraries(${PROJECT_NAME} ${MIKAN_UI_REQ_LIBS})

# Copy CEF binary and resource files to the target output directory
COPY_FILES("${PROJECT_NAME}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
COPY_FILES("${PROJECT_NAME}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Build TypeScript sources (if npm and TypeScript are available)
find_program(NPM_EXECUTABLE npm)
if(NPM_EXECUTABLE)
    message(STATUS "Found npm, will build TypeScript sources")

    # Install dependencies if needed
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/.package-lock.json
        COMMAND ${NPM_EXECUTABLE} install
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Installing npm dependencies for MikanUI"
    )

    # Compile TypeScript
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/resources/js/app.js
        COMMAND ${NPM_EXECUTABLE} run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/.package-lock.json
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ts/app.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ts/native-bridge.ts
        COMMENT "Compiling TypeScript for MikanUI"
    )

    # Add TypeScript compilation as a dependency
    add_custom_target(MikanUI_TypeScript
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/js/app.js
    )
    add_dependencies(${PROJECT_NAME} MikanUI_TypeScript)
else()
    message(WARNING "npm not found, TypeScript sources will not be compiled automatically")
endif()

# Copy web resources to output directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)
