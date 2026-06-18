# Step into the subdirectories
if(MIKAN_WITH_GSTREAMER)
  MESSAGE(STATUS "Stepping into MikanGStreamerVideo")
  add_subdirectory(MikanGStreamerVideo)
endif()
MESSAGE(STATUS "Stepping into MikanSteamVR")
add_subdirectory(MikanSteamVR)
MESSAGE(STATUS "Stepping into MikanWMFVideo")
add_subdirectory(MikanWMFVideo)