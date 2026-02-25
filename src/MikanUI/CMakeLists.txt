cmake_minimum_required(VERSION 3.19)

project(MikanUI)

# Normalize CEF_ROOT path to use forward slashes to avoid escape character issues
file(TO_CMAKE_PATH "${CEF_ROOT}" CEF_ROOT)

# CEF Configuration
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Use the CEF CMake configuration
find_package(CEF REQUIRED)

# MikanUI executable sources
set(MIKAN_UI_SOURCES
    main.cpp
    MikanUIApp.cpp
    MikanUIClient.cpp
)

set(MIKAN_UI_HEADERS
    MikanUIApp.h
    MikanUIClient.h
)

# Add executable target
add_executable(${PROJECT_NAME} WIN32 ${MIKAN_UI_SOURCES} ${MIKAN_UI_HEADERS})

# Set CEF properties
SET_EXECUTABLE_TARGET_PROPERTIES(${PROJECT_NAME})

# Add CEF include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CEF_ROOT})

# Link CEF libraries and Windows libraries
target_link_libraries(${PROJECT_NAME}
    libcef_dll_wrapper
    "$<$<CONFIG:Debug>:${CEF_ROOT}/Debug/libcef.lib>"
    "$<$<CONFIG:Release>:${CEF_ROOT}/Release/libcef.lib>"
    shlwapi
    crypt32
    wintrust
)

# Copy CEF binary and resource files to the target output directory
COPY_FILES("${PROJECT_NAME}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
COPY_FILES("${PROJECT_NAME}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Copy web resources to output directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)
