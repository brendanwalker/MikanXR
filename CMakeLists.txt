cmake_minimum_required(VERSION 3.15)
project(Mikan VERSION 1.0.0)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(cmake/Environment.cmake)
include(cmake/Version.cmake)

set(MIKAN_PROJECT_NAME "Mikan_${MIKAN_VERSION_STRING}")
set(MIKAN_ROOT_INSTALL_PATH "${ROOT_DIR}/dist")
set(MIKAN_DEBUG_INSTALL_PATH "${MIKAN_ROOT_INSTALL_PATH}/${ARCH_LABEL}/Debug")
set(MIKAN_RELEASE_INSTALL_PATH "${MIKAN_ROOT_INSTALL_PATH}/${ARCH_LABEL}/Release")
file(MAKE_DIRECTORY "${MIKAN_ROOT_INSTALL_PATH}")
file(WRITE "${MIKAN_ROOT_INSTALL_PATH}/version.txt" "${MIKAN_PROJECT_NAME}")

include(cmake/ThirdParty.cmake)
include(cmake/Installer.cmake)

# Step into the subdirectories
add_subdirectory(src)