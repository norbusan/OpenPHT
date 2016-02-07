#!/bin/bash

MAKEFLAGS=""
BGPROCESSFILE="$2"

if [ "$1" == "clean" ]
then
  if [ -d .libs ]
  then
    rm -r .libs
  fi
  make distclean
fi

if [ $NUMBER_OF_PROCESSORS > 1 ]; then
  MAKEFLAGS=-j$NUMBER_OF_PROCESSORS
fi

if [ ! -d .libs ]; then
  mkdir .libs
fi

# add --enable-debug (remove --disable-debug ofc) to get ffmpeg log messages in xbmc.log
# the resulting debug dll's are twice to fourth time the size of the release binaries

OPTIONS="
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
--enable-dxva2 \
--cpu=i686 \
--enable-gnutls"

./configure --extra-cflags="-fno-common -Iinclude-xbmc-win32/dxva2 -DNDEBUG" --extra-ldflags="-L/xbmc/system/players/dvdplayer" ${OPTIONS} &&

make $MAKEFLAGS &&
cp lib*/*.dll .libs/ &&
cp lib*/*.lib .libs/ &&
cp .libs/avcodec-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avformat-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avutil-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/avfilter-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/postproc-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/swresample-*.dll /xbmc/system/players/dvdplayer/ &&
cp .libs/swscale-*.dll /xbmc/system/players/dvdplayer/

#remove the bgprocessfile for signaling the process end
echo deleting $BGPROCESSFILE
rm $BGPROCESSFILE
