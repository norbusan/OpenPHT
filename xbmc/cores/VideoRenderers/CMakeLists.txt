add_subdirectory(VideoShaders)
find_all_sources(. videorenderers_SRCS)

if(NOT TARGET_RPI AND NOT TARGET_AML)
  list(REMOVE_ITEM videorenderers_SRCS ./LinuxRendererGLES.cpp)
endif()

if(NOT TARGET_RPI)
  list(REMOVE_ITEM videorenderers_SRCS ./MMALRenderer.cpp)
endif()

if(NOT WIN32)
  list(REMOVE_ITEM videorenderers_SRCS ./OverlayRendererDX.cpp)
  list(REMOVE_ITEM videorenderers_SRCS ./WinRenderer.cpp)
endif()

if (NOT TARGET_OSX AND NOT TARGET_POSIX)
  list(REMOVE_ITEM videorenderers_SRCS ./LinuxRendererGL.cpp)
endif()

if (NOT TARGET_OSX AND NOT TARGET_POSIX)
  list(REMOVE_ITEM videorenderers_SRCS ./OverlayRendererGL.cpp)
endif()

add_sources(${videorenderers_SRCS})
