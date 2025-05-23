cmake_minimum_required(VERSION 3.15)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(MIKAN_PROJECT_NAME "Mikan_${MIKAN_VERSION_STRING}")
set(MIKAN_ROOT_INSTALL_PATH "${ROOT_DIR}/dist")
file(MAKE_DIRECTORY "${MIKAN_ROOT_INSTALL_PATH}")
file(WRITE "${MIKAN_ROOT_INSTALL_PATH}/version.txt" "${MIKAN_PROJECT_NAME}")

# Any third party dependencies will install here
set(CMAKE_INSTALL_PREFIX ${MIKAN_ROOT_INSTALL_PATH}/deps)

project(Mikan LANGUAGES C CXX VERSION 1.0.0)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(cmake/Environment.cmake)
include(cmake/Version.cmake)
include(cmake/ThirdParty.cmake)
include(cmake/Installer.cmake)

# Mikan will install here
# ARCH_LABEL set in Environment.cmake
set(MIKAN_ARCH_INSTALL_PATH "${MIKAN_ROOT_INSTALL_PATH}/${ARCH_LABEL}")

# Step into the subdirectories
add_subdirectory(src)
add_subdirectory(bindings)
add_subdirectory(thirdparty)