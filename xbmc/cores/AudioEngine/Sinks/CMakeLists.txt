add_sources(AESinkNULL.cpp AESinkNULL.h)

if(DEFINED HAS_ALSA)
  add_sources(AESinkALSA.cpp AESinkALSA.h)
  add_subdirectory(alsa)
endif()

if(DEFINED HAVE_LIBPULSEAUDIO)
  add_sources(AESinkPULSE.cpp AESinkPULSE.h)
endif()

if(TARGET_WIN32)
  add_sources(AESinkDirectSound.cpp AESinkDirectSound.h)
  add_sources(AESinkWASAPI.cpp AESinkWASAPI.h)
endif(TARGET_WIN32)

if(TARGET_OSX)
  add_sources(AESinkDARWINOSX.cpp AESinkDARWINOSX.h)
  add_subdirectory(osx)
endif(TARGET_OSX)

if(TARGET_RPI)
  add_sources(AESinkPi.cpp AESinkPi.h)
endif(TARGET_RPI)

if(TARGET_COMMON_LINUX OR TARGET_FREEBSD)
  add_sources(AESinkOSS.cpp AESinkOSS.h)
endif(TARGET_COMMON_LINUX OR TARGET_FREEBSD)
