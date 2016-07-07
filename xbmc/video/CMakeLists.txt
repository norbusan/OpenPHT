add_subdirectory(dialogs)
add_subdirectory(videosync)
add_subdirectory(windows)

find_all_sources(. video_SRCS)
list(REMOVE_ITEM video_SRCS ./VideoThumbLoader.cpp)
add_sources(${video_SRCS})
