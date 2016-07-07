#!/bin/bash

MAKEFLAGS=""
BGPROCESSFILE="$2"

LIBNAME=ffmpeg

if [ "$1" == "clean" ]
then
  if [ -d .libs ]
  then
    rm -r .libs
  fi
  make distclean
fi

if [ ! -d .libs ]; then
  mkdir .libs
fi

if [ $NUMBER_OF_PROCESSORS > 1 ]; then
  if [ $NUMBER_OF_PROCESSORS > 4 ]; then
    MAKEFLAGS=-j6
  else
    MAKEFLAGS=-j`expr $NUMBER_OF_PROCESSORS + $NUMBER_OF_PROCESSORS / 2`
  fi
fi

# add --enable-debug (remove --disable-debug ofc) to get ffmpeg log messages in xbmc.log
# the resulting debug dll's are twice to fourth time the size of the release binaries

OPTIONS="
--disable-muxers \
--disable-encoders \
--enable-shared \
--enable-memalign-hack \
--enable-gpl \
--enable-w32threads \
--enable-postproc \
--enable-zlib \
--disable-static \
--disable-debug \
--disable-ffplay \
--disable-ffserver \
--disable-ffmpeg \
--disable-ffprobe \
--disable-devices \
--disable-doc \
--disable-crystalhd \
--enable-muxer=spdif \
--enable-muxer=adts \
--enable-muxer=asf \
--enable-muxer=ipod \
--enable-muxer=ogg \
--enable-encoder=ac3 \
--enable-encoder=aac \
--enable-protocol=http \
--enable-protocol=https \
--enable-runtime-cpudetect \
--disable-d3d11va \
--enable-dxva2 \
--cpu=i686 \
--enable-gnutls \
--enable-libdcadec"

echo configuring $LIBNAME
./configure --target-os=mingw32 --extra-cflags="-DPTW32_STATIC_LIB" --extra-ldflags="-static-libgcc" ${OPTIONS} &&

make $MAKEFLAGS &&
cp lib*/*.dll .libs/ &&
cp lib*/*.lib .libs/ &&
cp .libs/avcodec-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avformat-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avutil-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avfilter-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/postproc-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/swresample-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/swscale-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avcodec.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/avformat.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/avutil.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/avfilter.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/postproc.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/swresample.lib /xbmc/project/BuildDependencies/lib/ &&
cp .libs/swscale.lib /xbmc/project/BuildDependencies/lib/

#remove the bgprocessfile for signaling the process end
if [ -f "$BGPROCESSFILE" ]; then
  rm $BGPROCESSFILE
fi
