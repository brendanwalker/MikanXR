# Step into the subdirectories
MESSAGE(STATUS "Stepping into fast_obj")
add_subdirectory(fast_obj)
set_target_properties(fast_obj_lib PROPERTIES FOLDER ThirdParty)

MESSAGE(STATUS "Stepping into IxWebSocket")
add_subdirectory(IxWebSocket)
set_target_properties(ixwebsocket PROPERTIES FOLDER ThirdParty)

MESSAGE(STATUS "Stepping into RML")
add_subdirectory(RmlUI)
set_target_properties(RmlCore PROPERTIES FOLDER ThirdParty)
set_target_properties(RmlDebugger PROPERTIES FOLDER ThirdParty)