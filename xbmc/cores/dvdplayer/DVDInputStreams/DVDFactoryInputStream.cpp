/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#include "DVDFactoryInputStream.h"
#include "DVDInputStream.h"
#include "DVDInputStreamFile.h"
#include "DVDInputStreamNavigator.h"
#include "DVDInputStreamFFmpeg.h"
#include "DVDInputStreamPVRManager.h"
#include "DVDInputStreamTV.h"
#include "DVDInputStreamRTMP.h"
#ifdef HAVE_LIBBLURAY
#include "DVDInputStreamBluray.h"
#endif
#ifdef ENABLE_DVDINPUTSTREAM_STACK
#include "DVDInputStreamStack.h"
#endif
#include "FileItem.h"
#include "storage/MediaManager.h"
#include "URL.h"
#include "filesystem/File.h"
#include "utils/URIUtils.h"

/* PLEX */
#include "GUISettings.h"
/* END PLEX */

CDVDInputStream* CDVDFactoryInputStream::CreateInputStream(IDVDPlayer* pPlayer, const std::string& file, const std::string& content)
{
  CFileItem item(file.c_str(), false);

  if(file.substr(0, 6) == "rtp://"
     || file.substr(0, 7) == "rtsp://"
     || file.substr(0, 6) == "sdp://"
     || file.substr(0, 6) == "udp://"
     || file.substr(0, 6) == "tcp://"
     || file.substr(0, 6) == "mms://"
     || file.substr(0, 7) == "mmst://"
     || file.substr(0, 7) == "mmsh://"
     || (item.IsInternetStream() && item.IsType(".m3u8")))
    return new CDVDInputStreamFFmpeg();
#ifdef ENABLE_DVDINPUTSTREAM_STACK
  else if(file.substr(0, 8) == "stack://")
    return new CDVDInputStreamStack();
#endif
#ifdef HAS_LIBRTMP
  else if(file.substr(0, 7) == "rtmp://"
       || file.substr(0, 8) == "rtmpt://"
       || file.substr(0, 8) == "rtmpe://"
       || file.substr(0, 9) == "rtmpte://"
       || file.substr(0, 8) == "rtmps://")
    return new CDVDInputStreamRTMP();
#endif

  /* PLEX */
  if ((file.substr(0, 13) == "plexserver://"
    || file.substr(0, 7) == "http://"
    || file.substr(0, 8) == "https://") && (g_guiSettings.GetBool("videoplayer.useffmpegavio")))
  {
    CURL url(file);
    if (!boost::starts_with(url.GetFileName(), "sync/exchange/google/"))
      return new CDVDInputStreamFFmpeg();
    else
      return new CDVDInputStreamFile();
  }
  else
  /* END PLEX */
    // our file interface handles all these types of streams
    return (new CDVDInputStreamFile());
}
