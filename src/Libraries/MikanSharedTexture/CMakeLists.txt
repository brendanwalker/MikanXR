## Mikan Interprocess DLL
## =======================

MESSAGE(STATUS "Generating Mikan Shared Texture DLL Project")

file(GLOB MIKAN_SHARED_TEXTURE_PRIVATE_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp"
)

file(GLOB MIKAN_SHARED_TEXTURE_SPOUT2_DX_SRC
    "${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX/*.h"
)

file(GLOB MIKAN_SHARED_TEXTURE_PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Public/*.h"
)

file(GLOB MIKAN_SHARED_TEXTURE_LIBRARY_SRC
    ${MIKAN_SHARED_TEXTURE_PUBLIC}
    ${MIKAN_SHARED_TEXTURE_PRIVATE_SRC}
    ${MIKAN_SHARED_TEXTURE_SPOUT2_DX_SRC}
)

# Remove problematic files from the unity builds (header file inclusion ordering issues)
set_source_files_properties(${MIKAN_SHARED_TEXTURE_SPOUT2_DX_SRC} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

source_group("Public" FILES ${MIKAN_SHARED_TEXTURE_PUBLIC})
source_group("Private" FILES ${MIKAN_SHARED_TEXTURE_PRIVATE_SRC})
source_group("Private\\SpoutDX" FILES ${MIKAN_SHARED_TEXTURE_SPOUT2_DX_SRC})

list(APPEND MIKAN_SHARED_TEXTURE_INCL_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/Public
  ${CMAKE_CURRENT_LIST_DIR}/Private
  ${CMAKE_CURRENT_LIST_DIR}/Private/SpoutDX
  ${SPOUT2_INCLUDE_DIRS})

list(APPEND MIKAN_INTERPROCESS_REQ_LIBS
  ${SPOUT2_LIBRARIES})

add_library(MikanSharedTexture SHARED ${MIKAN_SHARED_TEXTURE_LIBRARY_SRC})
target_include_directories(MikanSharedTexture PRIVATE ${MIKAN_SHARED_TEXTURE_INCL_DIRS})
target_link_libraries(MikanSharedTexture PRIVATE ${MIKAN_INTERPROCESS_REQ_LIBS})
set_target_properties(MikanSharedTexture PROPERTIES FOLDER MikanLibraries)
set_target_properties(MikanSharedTexture PROPERTIES PUBLIC_HEADER "${MIKAN_SHARED_TEXTURE_PUBLIC}")
set_target_properties(MikanSharedTexture PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(MikanSharedTexture PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(MikanSharedTexture PRIVATE MIKAN_SHAREDTEXTURE_EXPORTS)

# Install - copy runtime dependencies to dist folder (for release)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows") 
  install(TARGETS MikanSharedTexture
      RUNTIME DESTINATION ${MIKAN_ARCH_INSTALL_PATH}
      LIBRARY DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      ARCHIVE DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/lib
      PUBLIC_HEADER DESTINATION ${MIKAN_ARCH_INSTALL_PATH}/include)
ELSE() #Linux/Darwin
ENDIF()