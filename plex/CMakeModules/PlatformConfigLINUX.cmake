# vim: setlocal syntax=cmake:

######################### Compiler CFLAGS
set(EXTRA_CFLAGS "-fPIC -DPIC")

######################### CHECK LIBRARIES / FRAMEWORKS
if(UNIX)
  set(CMAKE_REQUIRED_FLAGS "-D__LINUX_USER__")
endif()

option(USE_INTERNAL_FFMPEG "Use internal FFmpeg?" ON)

set(LINK_PKG
  Freetype
  SDL
  OpenGL
  ZLIB
  JPEG
  X11
  SQLite3
  PCRE
  Lzo2
  FriBiDi
  Fontconfig
  YAJL
  microhttpd
  Crypto
  TinyXML
  GLEW
  Iconv
  Avahi
  Xrandr
  LibDl
  LibRt
  FLAC
  DBUS
  DRM
)

if(NOT OPENELEC)
  list(APPEND LINK_PKG SDL_image)
endif()

if(NOT USE_INTERNAL_FFMPEG)
  list(APPEND LINK_PKG FFmpeg)
else()
  set(FFMPEG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/ffmpeg ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/src/ffmpeg-build)
  list(APPEND CONFIG_PLEX_LINK_LIBRARIES 
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libavcodec.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libavformat.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libavutil.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libpostproc.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libswscale.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libavfilter.so
       ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/lib/libswresample.so)
endif()

if(ENABLE_PYTHON)
   list(APPEND LINK_PKG Python)
endif(ENABLE_PYTHON)

foreach(l ${LINK_PKG})
  plex_find_package(${l} 1 1)
endforeach()

find_package(Boost COMPONENTS thread system REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  list(APPEND CONFIG_PLEX_LINK_LIBRARIES ${Boost_LIBRARIES})
  set(HAVE_BOOST 1)
endif()

### install libs
set(INSTALL_LIB
  CURL
  PNG
  TIFF
  Vorbis
  Mpeg2
  Ass
  RTMP
  PLIST
)

foreach(l ${INSTALL_LIB})
  plex_find_package(${l} 1 2)
endforeach()

option(ENABLE_SHAIRPLAY "Enable ShairPlay?" ON)
if(ENABLE_SHAIRPLAY)
  plex_find_package(ShairPlay 1 2)
endif()

option(ENABLE_SHAIRPORT "Enable ShairPort?" OFF)
if(ENABLE_SHAIRPORT AND NOT ENABLE_SHAIRPLAY)
  plex_find_package(ShairPort 1 2)
endif()

option(ENABLE_VAAPI "Enable VAAPI?" ON)
if(ENABLE_VAAPI)
  plex_find_package(VAAPI 1 2)
endif()

option(ENABLE_VDPAU "Enable VDPAU?" ON)
if(ENABLE_VDPAU)
  plex_find_package(VDPAU 1 2)
endif()

option(ENABLE_CEC "Enable CEC?" ON)
if(ENABLE_CEC)
  plex_find_package(CEC 1 2)
endif()

plex_find_package(Threads 1 0)
if(CMAKE_USE_PTHREADS_INIT)
  message(STATUS "Using pthreads: ${CMAKE_THREAD_LIBS_INIT}")
  list(APPEND CONFIG_PLEX_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  set(HAVE_LIBPTHREAD 1)
endif()

plex_find_package(PulseAudio 0 1)
if(HAVE_LIBPULSEAUDIO)
  set(HAVE_LIBPULSE 1)
endif()
plex_find_package(Alsa 0 1)

plex_find_package(LibUSB 0 1)
plex_find_package(LibUDEV 0 1)

if(ENABLE_DVD_DRIVE)
  plex_find_package(CDIO 1 1)
endif(ENABLE_DVD_DRIVE)

if(NOT LIBUSB_FOUND AND NOT LIBUDEV_FOUND)
  message(WARNING "No USB support")
endif()

if(VAAPI_FOUND)
  list(APPEND CONFIG_PLEX_LINK_LIBRARIES ${VAAPI_LIBRARIES})
  include_directories(${VAAPI_INCLUDE_DIR})
  set(HAVE_LIBVA 1)
endif()

plex_get_soname(CURL_SONAME ${CURL_LIBRARY})

####
if(DEFINED X11_FOUND)
  set(HAVE_X11 1)
endif()

if(DEFINED OPENGL_FOUND)
  set(HAVE_LIBGL 1)
endif()

if(DEFINED DBUS_FOUND)
  include_directories(${DBUS_INCLUDE_DIR} ${DBUS_ARCH_INCLUDE_DIR})
  set(HAVE_DBUS 1)
endif()

#### default lircdevice
if(NOT DEFINED LIRC_DEVICE)
  set(LIRC_DEVICE "/var/run/lirc/lircd")
endif()

#### on linux we want to use a "easy" name
set(EXECUTABLE_NAME "plexhometheater")

if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  set(ARCH "x86_64-linux")
else()
  set(ARCH "i486-linux")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif()
  endif()
endif()

## remove annying useless warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sequence-point")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-array-bounds")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-value")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-maybe-uninitialized")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-array-bounds")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")

set(LIBPATH bin)
set(BINPATH bin)
set(RESOURCEPATH share/XBMC)

set(PLEX_LINK_WRAPPED "-Wl,--unresolved-symbols=ignore-all -Wl,-wrap,_IO_getc -Wl,-wrap,_IO_getc_unlocked -Wl,-wrap,_IO_putc -Wl,-wrap,__fgets_chk -Wl,-wrap,__fprintf_chk -Wl,-wrap,__fread_chk -Wl,-wrap,__fxstat64 -Wl,-wrap,__lxstat64 -Wl,-wrap,__printf_chk -Wl,-wrap,__read_chk -Wl,-wrap,__vfprintf_chk -Wl,-wrap,__xstat64 -Wl,-wrap,_stat -Wl,-wrap,calloc -Wl,-wrap,clearerr -Wl,-wrap,close -Wl,-wrap,closedir -Wl,-wrap,dlopen -Wl,-wrap,fclose -Wl,-wrap,fdopen -Wl,-wrap,feof -Wl,-wrap,ferror -Wl,-wrap,fflush -Wl,-wrap,fgetc -Wl,-wrap,fgetpos -Wl,-wrap,fgetpos64 -Wl,-wrap,fgets -Wl,-wrap,fileno -Wl,-wrap,flockfile -Wl,-wrap,fopen -Wl,-wrap,fopen64 -Wl,-wrap,fprintf -Wl,-wrap,fputc -Wl,-wrap,fputs -Wl,-wrap,fread -Wl,-wrap,free -Wl,-wrap,freopen -Wl,-wrap,fseek -Wl,-wrap,fseeko64 -Wl,-wrap,fsetpos -Wl,-wrap,fsetpos64 -Wl,-wrap,fstat -Wl,-wrap,ftell -Wl,-wrap,ftello64 -Wl,-wrap,ftrylockfile -Wl,-wrap,funlockfile -Wl,-wrap,fwrite -Wl,-wrap,getc -Wl,-wrap,getc_unlocked -Wl,-wrap,getmntent -Wl,-wrap,ioctl -Wl,-wrap,lseek -Wl,-wrap,lseek64 -Wl,-wrap,malloc -Wl,-wrap,open -Wl,-wrap,open64 -Wl,-wrap,opendir -Wl,-wrap,popen -Wl,-wrap,printf -Wl,-wrap,read -Wl,-wrap,readdir -Wl,-wrap,readdir64 -Wl,-wrap,realloc -Wl,-wrap,rewind -Wl,-wrap,rewinddir -Wl,-wrap,setvbuf -Wl,-wrap,ungetc -Wl,-wrap,vfprintf -Wl,-wrap,write")

set(PLEX_LINK_WHOLEARCHIVE -Wl,--whole-archive)
set(PLEX_LINK_NOWHOLEARCHIVE -Wl,--no-whole-archive)

option(OPENELEC "Are we building OpenELEC dist?" OFF)
if(OPENELEC)
  add_definitions(-DTARGET_OPENELEC)
else()
  set(LIBPATH lib/openpht)
  set(BINPATH lib/openpht)
  set(RESOURCEPATH share/openpht)
  configure_file("${plexdir}/Resources/openpht.sh.in" "${CMAKE_BINARY_DIR}/openpht.sh" @ONLY)
  configure_file("${plexdir}/Resources/openpht.desktop.in" "${CMAKE_BINARY_DIR}/openpht.desktop" @ONLY)
  install(PROGRAMS "${CMAKE_BINARY_DIR}/openpht.sh" DESTINATION bin COMPONENT RUNTIME RENAME openpht)
  install(FILES "${CMAKE_BINARY_DIR}/openpht.desktop" DESTINATION share/applications COMPONENT RUNTIME)
  install(FILES "${plexdir}/Resources/plex-icon-256.png" DESTINATION share/pixmaps COMPONENT RUNTIME RENAME openpht.png)
endif(OPENELEC)

############ Add our definitions
add_definitions(-DTARGET_LINUX)
