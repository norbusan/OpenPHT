include(ExternalProject)

if(TARGET_LINUX)
  set(OS linux)
elseif(TARGET_DARWIN)
  set(OS darwin)
elseif(TARGET_FREEBSD)
  set(OS freebsd)
endif()


if(TARGET_FREEBSD)
  set(CONFIG_FLAGS --target-os=${OS} --cc=${CMAKE_C_COMPILER} --cxx=${CMAKE_CXX_COMPILER} --extra-cflags=-I${CMAKE_PREFIX_PATH}/include --extra-ldflags=-L${CMAKE_PREFIX_PATH}/lib --disable-altivec --disable-amd3dnow --disable-avisynth --disable-crystalhd --disable-debug --disable-decoder=mpeg_xvmc --disable-devices --disable-doc --disable-dxva2 --disable-encoders --disable-extra-warnings --disable-ffmpeg --disable-ffplay --disable-ffprobe --disable-ffserver --disable-frei0r --disable-gray --disable-hardcoded-tables --disable-indevs --disable-iwmmxt --disable-libdc1394 --disable-libdirac --disable-libfaac --disable-libfreetype --disable-libgsm --disable-libmp3lame --disable-libnut --disable-libopencore-amrnb --disable-libopencore-amrwb --disable-libopencv --disable-libopenjpeg --disable-librtmp --disable-libschroedinger --disable-libspeex --disable-libtheora --disable-libvo-aacenc --disable-libvo-amrwbenc --disable-libvorbis --disable-libvpx --disable-libx264 --disable-libxavs --disable-libxvid --disable-memalign-hack --disable-mlib --disable-mmi --disable-muxers --disable-nonfree --disable-outdevs --disable-small --disable-sram --disable-static --disable-symver --disable-vda --disable-version3 --disable-vis --disable-w32threads --disable-x11grab --enable-aandct --enable-asm --enable-avcodec --enable-avdevice --enable-avfilter --enable-avformat --enable-bsfs --enable-bzlib --enable-dct --enable-demuxers --enable-fastdiv --enable-fft --enable-filters --enable-golomb --enable-gpl --enable-huffman --enable-hwaccels --enable-libvorbis --enable-logging --enable-lpc --enable-mdct --enable-muxer=adts --enable-muxer=ogg --enable-muxer=spdif --enable-network --enable-optimizations --enable-parsers --enable-pic --enable-postproc --enable-protocol=http --enable-pthreads --enable-rdft --enable-runtime-cpudetect --enable-shared --enable-stripping --enable-swscale --enable-swscale-alpha --enable-vaapi --enable-vdpau --enable-yasm --enable-zlib --host-libs=-lm)
elseif(NOT TARGET_RPI)
  # TODO: --enable-libdcadec
  set(CONFIG_FLAGS --target-os=${OS} --cc=${CMAKE_C_COMPILER} --cxx=${CMAKE_CXX_COMPILER} --sysinclude="${CMAKE_PREFIX_PATH}/usr/include" --cpu=${CMAKE_SYSTEM_PROCESSOR} --enable-pic --enable-shared --disable-static --disable-debug --disable-muxers --disable-encoders --disable-devices --disable-doc --disable-ffplay --disable-ffmpeg --disable-sdl --disable-ffprobe --disable-ffserver --enable-gpl --enable-runtime-cpudetect --enable-postproc --enable-pthreads --enable-muxer=spdif --enable-muxer=adts --enable-muxer=asf --enable-muxer=ipod --enable-encoder=ac3 --enable-encoder=aac --enable-protocol=http --enable-protocol=https --enable-gnutls --enable-vdpau --enable-vaapi --enable-libvorbis --enable-muxer=ogg --enable-encoder=libvorbis)
else()
  set(CONFIG_FLAGS --target-os=${OS} --disable-muxers --disable-encoders --disable-devices --disable-doc --disable-ffplay --disable-ffmpeg --disable-ffprobe --disable-ffserver --disable-vda --disable-crystalhd --disable-decoder=mpeg_xvmc --disable-debug --disable-amd3dnow --disable-libvorbis --enable-gpl --enable-postproc --enable-shared --disable-static --enable-pthreads --enable-muxer=spdif --enable-muxer=adts --enable-encoder=ac3 --enable-encoder=aac --enable-protocol=http --enable-runtime-cpudetect)

  list(APPEND CONFIG_FLAGS --enable-hardcoded-tables --enable-pic
  --cpu=arm1176jzf-s --arch=arm --enable-cross-compile
  --cross-prefix=${TARGET_PREFIX} --sysroot=${SYSROOT_PREFIX}
  --sysinclude="${SYSROOT_PREFIX}/usr/include" )

endif()

ExternalProject_Add(
  ffmpeg
  URL .
  PREFIX ffmpeg
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> ${CONFIG_FLAGS}
  BUILD_COMMAND make -j 4
  INSTALL_COMMAND make install
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libavcodec.so.56.60.100 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libavcodec.so.56)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libavformat.so.56.40.101 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libavformat.so.56)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libavutil.so.54.31.100 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libavutil.so.54)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libpostproc.so.53.3.100 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libpostproc.so.53)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libswscale.so.3.1.101 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libswscale.so.3)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libavfilter.so.5.40.101 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libavfilter.so.5)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg/lib/libswresample.so.1.2.101 DESTINATION ${LIBPATH}/system/players/dvdplayer RENAME libswresample.so.1)
