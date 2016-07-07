# vim: setlocal syntax=cmake:

add_definitions(-DTARGET_AML)

if($ENV{PROJECT} STREQUAL "WeTek_Play")
  add_definitions(-DTARGET_WETEK_PLAY)
elseif($ENV{PROJECT} STREQUAL "WeTek_Core")
  add_definitions(-DTARGET_WETEK_CORE)
elseif($ENV{PROJECT} STREQUAL "Odroid_C2")
  add_definitions(-DTARGET_ODROID_C2)
endif()

message(STATUS "Building for AMLogic")

######################### Compiler CFLAGS
set(EXTRA_CFLAGS "-fPIC -DPIC")

######################### CHECK LIBRARIES / FRAMEWORKS
option(USE_INTERNAL_FFMPEG "Use internal FFmpeg?" OFF)

set(LINK_PKG
  Freetype
  ZLIB
  JPEG
  SQLite3
  PCRE
  Lzo2
  FriBiDi
  Fontconfig
  YAJL
  microhttpd
  Crypto
  TinyXML
  Iconv
  Avahi
  LibDl
  LibRt
  FLAC
  DBUS
)

if(NOT USE_INTERNAL_FFMPEG)
  list(APPEND LINK_PKG FFmpeg)
else()
  set(FFMPEG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/lib/ffmpeg ${CMAKE_BINARY_DIR}/lib/ffmpeg/ffmpeg/src/ffmpeg-build)
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
  plex_find_package(${l} 1 0)
endforeach()

option(ENABLE_SHAIRPLAY "Enable ShairPlay?" ON)
if(ENABLE_SHAIRPLAY)
  plex_find_package(ShairPlay 1 0)
endif()

option(ENABLE_SHAIRPORT "Enable ShairPort?" OFF)
if(ENABLE_SHAIRPORT AND NOT ENABLE_SHAIRPLAY)
  plex_find_package(ShairPort 1 0)
endif()

option(ENABLE_CEC "Enable CEC?" ON)
if(ENABLE_CEC)
  plex_find_package(CEC 1 0)
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

if(NOT LIBUSB_FOUND AND NOT LIBUDEV_FOUND)
  message(WARNING "No USB support")
endif()


plex_get_soname(CURL_SONAME ${CURL_LIBRARY})

####
if(DEFINED DBUS_FOUND)
  include_directories(${DBUS_INCLUDE_DIR} ${DBUS_ARCH_INCLUDE_DIR})
  set(HAVE_DBUS 1)
endif()

#### default lircdevice
if(NOT DEFINED LIRC_DEVICE)
  set(LIRC_DEVICE "/run/lirc/lircd")
endif()

#### on linux we want to use a "easy" name
set(EXECUTABLE_NAME "plexhometheater")

set(ARCH "arm")
set(USE_OPENGLES 1)
set(USE_OMXLIB 0)
set(USE_OPENMAX 0)
set(USE_PULSE 0)
set(DISABLE_PROJECTM 1)
set(USE_TEXTUREPACKER_NATIVE_ROOT 0)

set(BUILD_DVDCSS 0)
set(SKIP_CONFIG_DVDCSS 1)
set(DVDREAD_CFLAGS "-D_XBMC -UHAVE_DVDCSS_DVDCSS_H")

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

plex_find_library(EGL 0 0  system/usr/lib 1)
plex_find_library(GLESv2 0  0 system/usr/lib 1)
plex_find_library(amadec 0  0 system/usr/lib 1)
plex_find_library(amavutils 0  0 system/usr/lib 1)
plex_find_library(amcodec 0  0 system/usr/lib 1)

#needed for the commandline flag CMAKE_INCLUDE_PATH
foreach(path ${CMAKE_INCLUDE_PATH})
    include_directories(${path})
endforeach()

set(LIBPATH bin)
set(BINPATH bin)
set(RESOURCEPATH share/XBMC)

set(PLEX_LINK_WRAPPED "-Wl,--unresolved-symbols=ignore-all -Wl,-wrap,_IO_getc -Wl,-wrap,_IO_getc_unlocked -Wl,-wrap,_IO_putc -Wl,-wrap,__fgets_chk -Wl,-wrap,__fprintf_chk -Wl,-wrap,__fread_chk -Wl,-wrap,__fxstat64 -Wl,-wrap,__lxstat64 -Wl,-wrap,__printf_chk -Wl,-wrap,__read_chk -Wl,-wrap,__vfprintf_chk -Wl,-wrap,__xstat64 -Wl,-wrap,_stat -Wl,-wrap,calloc -Wl,-wrap,clearerr -Wl,-wrap,close -Wl,-wrap,closedir -Wl,-wrap,dlopen -Wl,-wrap,fclose -Wl,-wrap,fdopen -Wl,-wrap,feof -Wl,-wrap,ferror -Wl,-wrap,fflush -Wl,-wrap,fgetc -Wl,-wrap,fgetpos -Wl,-wrap,fgetpos64 -Wl,-wrap,fgets -Wl,-wrap,fileno -Wl,-wrap,flockfile -Wl,-wrap,fopen -Wl,-wrap,fopen64 -Wl,-wrap,fprintf -Wl,-wrap,fputc -Wl,-wrap,fputs -Wl,-wrap,fread -Wl,-wrap,free -Wl,-wrap,freopen -Wl,-wrap,fseek -Wl,-wrap,fseeko64 -Wl,-wrap,fsetpos -Wl,-wrap,fsetpos64 -Wl,-wrap,fstat -Wl,-wrap,ftell -Wl,-wrap,ftello64 -Wl,-wrap,ftrylockfile -Wl,-wrap,funlockfile -Wl,-wrap,fwrite -Wl,-wrap,getc -Wl,-wrap,getc_unlocked -Wl,-wrap,getmntent -Wl,-wrap,ioctl -Wl,-wrap,lseek -Wl,-wrap,lseek64 -Wl,-wrap,malloc -Wl,-wrap,open -Wl,-wrap,open64 -Wl,-wrap,opendir -Wl,-wrap,popen -Wl,-wrap,printf -Wl,-wrap,read -Wl,-wrap,readdir -Wl,-wrap,readdir64 -Wl,-wrap,realloc -Wl,-wrap,rewind -Wl,-wrap,rewinddir -Wl,-wrap,setvbuf -Wl,-wrap,ungetc -Wl,-wrap,vfprintf -Wl,-wrap,write")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} system/usr/lib )

set(PLEX_LINK_WHOLEARCHIVE -Wl,--whole-archive)
set(PLEX_LINK_NOWHOLEARCHIVE -Wl,--no-whole-archive)

option(OPENELEC "Are we building OpenELEC dist?" ON)
if(OPENELEC)
  add_definitions(-DTARGET_OPENELEC)
endif(OPENELEC)

############ Add our definitions
add_definitions(
  -DTARGET_LINUX
  -DHAS_GLES=2
  -DHAS_LIBAMCODEC
  -DHAS_BUILTIN_SYNC_ADD_AND_FETCH
  -DHAS_BUILTIN_SYNC_SUB_AND_FETCH
  -DHAS_BUILTIN_SYNC_VAL_COMPARE_AND_SWAP
)
