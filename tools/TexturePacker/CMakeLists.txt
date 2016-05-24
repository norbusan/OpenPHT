if(OPENELEC)
  project("TexturePacker")
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

  include(TestBigEndian)
  TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

  configure_file(config_plex.h.in config.h)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_definitions(-DHAVE_CONFIG_H -D_LINUX)

  set(root ${PROJECT_SOURCE_DIR}/../..)
  set(plexdir ${root}/plex)
  set(CMAKE_MODULE_PATH ${plexdir}/CMakeModules ${CMAKE_MODULE_PATH})

  include(CMakeFunctions)

  add_subdirectory(${root}/lib/libsquish ${CMAKE_CURRENT_BINARY_DIR}/libsquish)

  plex_find_package(SDL 1 1)
  plex_find_package(SDL_image 1 1)
  plex_find_package(Lzo2 1 1)
endif(OPENELEC)

set(SRC md5.cpp SDL_anigif.cpp XBTFWriter.cpp XBMCTex.cpp ${root}/xbmc/guilib/XBTF.cpp)
if(TARGET_WIN32)
  set(SRC ${SRC} Win32/dirent.c Win32/dirent.h)
endif(TARGET_WIN32)
include_directories(${root}/lib ${root}/xbmc ${CMAKE_BINARY_DIR}/xbmc ${root}/xbmc/linux)

if(CMAKE_SSE_CFLAGS)
  add_definitions(-DSQUISH_USE_SSE=2)

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_SSE_CFLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_SSE_CFLAGS}")

  if("x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang" OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xGNU")
    set(CMAKE_C_FLAGS_DEBUG "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O3")
  elseif("x${CMAKE_C_COMPILER_ID}" STREQUAL "xMSVC")
    set(CMAKE_C_FLAGS_DEBUG "/MDd /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /O2")
  endif("x${CMAKE_C_COMPILER_ID}" STREQUAL "xClang" OR "x${CMAKE_C_COMPILER_ID}" STREQUAL "xGNU")

endif(CMAKE_SSE_CFLAGS)

if(TARGET_WIN32)
  include_directories(Win32)
endif(TARGET_WIN32)

add_definitions(-DUSE_LZO_PACKING)
add_executable(TexturePacker ${SRC})
target_link_libraries(TexturePacker
  ${CONFIG_LIBRARY_SDL_IMAGE}
  ${CONFIG_LIBRARY_SDL}
  ${CONFIG_LIBRARY_STDC++}
  lib_squish
)
if(NOT TARGET_WIN32)
  target_link_libraries(TexturePacker ${CONFIG_LIBRARY_LZO2})
else(NOT TARGET_WIN32)
  find_library(LIB_LZO lzo ${root}/lib/win32/liblzo)
  target_link_libraries(TexturePacker ${LIB_LZO})
endif(NOT TARGET_WIN32)
