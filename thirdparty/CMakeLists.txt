# Step into the subdirectories
MESSAGE(STATUS "Stepping into fast_obj")
add_subdirectory(fast_obj)
set_target_properties(fast_obj_lib PROPERTIES FOLDER ThirdParty)

MESSAGE(STATUS "Stepping into IxWebSocket")
add_subdirectory(IxWebSocket)
set_target_properties(ixwebsocket PROPERTIES FOLDER ThirdParty)

# CEF libcef_dll_wrapper
if(DEFINED CEF_ROOT)
    MESSAGE(STATUS "Stepping into CEF libcef_dll_wrapper")
    # Normalize CEF_ROOT path
    file(TO_CMAKE_PATH "${CEF_ROOT}" CEF_ROOT)
    # Set CMAKE_MODULE_PATH for CEF
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
    # Find CEF package
    find_package(CEF REQUIRED)
    # Add libcef_dll_wrapper
    add_subdirectory(${CEF_ROOT}/libcef_dll ${CMAKE_CURRENT_BINARY_DIR}/libcef_dll_wrapper)
    set_target_properties(libcef_dll_wrapper PROPERTIES FOLDER ThirdParty)
endif()