//
//  PlexTranscoderClientRPi.cpp
//  RasPlex
//
//  Created by Lionel Chazallon on 2014-03-07.
//
//

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>

#include "Client/PlexTranscoderClientRPi.h"
#include "plex/PlexUtils.h"
#include "log.h"
#include "settings/AdvancedSettings.h"
#include "settings/GUISettings.h"
#include "Client/PlexConnection.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "PlexMediaDecisionEngine.h"
#include "linux/RBP.h"

///////////////////////////////////////////////////////////////////////////////
CPlexTranscoderClientRPi::CPlexTranscoderClientRPi()
{
  m_maxVideoBitrate = 0;
  m_maxAudioBitrate = 0;

  // Here is as list of audio / video codecs that we support natively on RPi
  m_knownVideoCodecs = boost::assign::list_of<std::string>  ("h264") ("mpeg4") ("theora") ("vp6") ("vp8") ("vp6f").convert_to_container<std::set<std::string>>();
  m_knownAudioCodecs = boost::assign::list_of<std::string>  ("") ("aac") ("ac3") ("eac3") ("mp3") ("mp2") ("dca") ("dca-ma") ("flac") ("pcm") ("aac_latm") ("vorbis").convert_to_container<std::set<std::string>>();

  // check if optionnal codecs are here
  if ( g_RBP.GetCodecMpg2() )
  {
    m_knownVideoCodecs.insert("mpeg2video");
  }

  if ( g_RBP.GetCodecWvc1() )
  {
    m_knownAudioCodecs.insert("wmav2");
    m_knownAudioCodecs.insert("wmapro");

    m_knownVideoCodecs.insert("vc1");
    m_knownVideoCodecs.insert("mjpeg");
    m_knownVideoCodecs.insert("wmv3");
  }

#ifdef TARGET_RASPBERRY_PI_2
    m_knownVideoCodecs.insert("hevc");
    m_knownAudioCodecs.insert("truehd");
#endif

  for (CStdStringArray::iterator it = g_advancedSettings.m_knownVideoCodecs.begin(); it != g_advancedSettings.m_knownVideoCodecs.end(); it++)
  {
    if (boost::starts_with(*it, "-") || boost::starts_with(*it, "!"))
      m_knownVideoCodecs.erase(it->substr(1).c_str());
    else
      m_knownVideoCodecs.insert(it->c_str());
  }
  for (CStdStringArray::iterator it = g_advancedSettings.m_knownAudioCodecs.begin(); it != g_advancedSettings.m_knownAudioCodecs.end(); it++)
  {
    if (boost::starts_with(*it, "-") || boost::starts_with(*it, "!"))
      m_knownAudioCodecs.erase(it->substr(1).c_str());
    else
      m_knownAudioCodecs.insert(it->c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
bool CPlexTranscoderClientRPi::ShouldTranscode(CPlexServerPtr server, const CFileItem& item)
{
  if (!item.IsVideo())
    return false;

  if (!server || !server->GetActiveConnection())
    return false;

  CFileItemPtr selectedItem = CPlexMediaDecisionEngine::getSelectedMediaItem((item));

  bool bShouldTranscode = false;
  CStdString ReasonWhy;


  // grab some properties
  std::string container = selectedItem->GetProperty("container").asString(),
              videoCodec = selectedItem->GetProperty("mediaTag-videoCodec").asString(),
              audioCodec = selectedItem->GetProperty("mediaTag-audioCodec").asString();


  int videoResolution = selectedItem->GetProperty("mediaTag-videoResolution").asInteger(),
      videoBitRate = selectedItem->GetProperty("bitrate").asInteger(),
      videoWidth = selectedItem->GetProperty("width").asInteger(),
      videoHeight = selectedItem->GetProperty("height").asInteger(),
      audioChannels = selectedItem->GetProperty("mediaTag-audioChannels").asInteger();

  // default capping values
  m_maxVideoBitrate = 200000;
  m_maxAudioBitrate = 100000;
  int maxBitDepth = 8;

  // grab some other information in the audio / video streams
  int audioBitRate = 0;
  float videoFrameRate = 0;
  int bitDepth = 0;

  CFileItemPtr audioStream,videoStream;
  CFileItemPtr mediaPart = selectedItem->m_mediaParts.at(0);
  if (mediaPart)
  {
    if ((audioStream = PlexUtils::GetSelectedStreamOfType(mediaPart, PLEX_STREAM_AUDIO)))
      audioBitRate = audioStream->GetProperty("bitrate").asInteger();
    else
      CLog::Log(LOGERROR,"CPlexTranscoderClient::ShouldTranscodeRPi - AudioStream is empty");

    if ((videoStream = PlexUtils::GetSelectedStreamOfType(mediaPart, PLEX_STREAM_VIDEO)))
    {
      videoFrameRate = videoStream->GetProperty("frameRate").asFloat();
      bitDepth = videoStream->GetProperty("bitDepth").asInteger();
    }
    else
      CLog::Log(LOGERROR,"CPlexTranscoderClient::ShouldTranscodeRPi - VideoStream is empty");
  }
  else CLog::Log(LOGERROR,"CPlexTranscoderClient::ShouldTranscodeRPi - MediaPart is empty");

  // Dump The Video information
  CLog::Log(LOGDEBUG,"----------- Video information for '%s' -----------",selectedItem->GetPath().c_str());
  CLog::Log(LOGDEBUG,"-%16s : %s", "container",container.c_str());
  CLog::Log(LOGDEBUG,"-%16s : %s", "videoCodec",videoCodec.c_str());
  CLog::Log(LOGDEBUG,"-%16s : %d", "videoResolution",videoResolution);
  CLog::Log(LOGDEBUG,"-%16s : %3.3f", "videoFrameRate",videoFrameRate);
  CLog::Log(LOGDEBUG,"-%16s : %d", "bitDepth",bitDepth);
  CLog::Log(LOGDEBUG,"-%16s : %d", "bitrate",videoBitRate);
  CLog::Log(LOGDEBUG,"-%16s : %d", "width",videoWidth);
  CLog::Log(LOGDEBUG,"-%16s : %d", "height",videoHeight);
  CLog::Log(LOGDEBUG,"----------- Audio information -----------");
  CLog::Log(LOGDEBUG,"-%16s : %s", "audioCodec",audioCodec.c_str());
  CLog::Log(LOGDEBUG,"-%16s : %d", "audioChannels",audioChannels);
  CLog::Log(LOGDEBUG,"-%16s : %d", "audioBitRate",audioBitRate);

  bool isLocal = server->GetActiveConnection()->IsLocal();
  int localQuality = localBitrate();
  int remoteQuality = remoteBitrate();

  // check if video resolution is to large
  if (videoWidth > 1920 || videoHeight > 1080)
  {
    bShouldTranscode = true;
    ReasonWhy.Format("Video resolution to large: %dx%d", videoWidth, videoHeight);
  }
  // check if video resolution is to large for hevc
  else if (videoCodec == "hevc" && videoWidth > g_guiSettings.GetInt("plexmediaserver.limithevc"))
  {
    bShouldTranscode = true;
    ReasonWhy.Format("Video resolution to large: %dx%d", videoWidth, videoHeight);
  }
  // check if seetings are to transcoding for local media
  else if ( isLocal && localQuality > 0 && localQuality < videoBitRate )
  {
    bShouldTranscode = true;
    m_maxVideoBitrate = localQuality;
    ReasonWhy.Format("Settings require local transcoding to %d kbps",localQuality);
  }
  // check if seetings are to transcoding for remote media
  else if ( !isLocal && remoteQuality > 0 && remoteQuality < videoBitRate )
  {
    bShouldTranscode = true;
    m_maxVideoBitrate = remoteQuality;
    ReasonWhy.Format("Settings require remote transcoding to %d kbps",remoteQuality);
  }
  // check if Video Codec is natively supported
  else if (m_knownVideoCodecs.find(videoCodec) == m_knownVideoCodecs.end())
  {
    bShouldTranscode = true;
    ReasonWhy.Format("Unknown video codec : %s",videoCodec);
  }
  // check if Audio Codec is natively supported
  else if (m_knownAudioCodecs.find(audioCodec) == m_knownAudioCodecs.end())
  {
    bShouldTranscode = true;
    ReasonWhy.Format("Unknown audio codec : %s",audioCodec);
  }
  else if (bitDepth > maxBitDepth)
  {
    bShouldTranscode = true;
    ReasonWhy.Format("Video bitDepth is too high : %d (max : %d)",bitDepth,maxBitDepth);
  }
  else if (transcodeForced())
  {
    bShouldTranscode = true;
    ReasonWhy = "Settings require transcoding";
  }

  if (bShouldTranscode)
  {
    // cap the transcode bitrate for qualities
    if (m_maxVideoBitrate > 20000)
      m_maxVideoBitrate = 20000;

    CLog::Log(LOGDEBUG,"RPi ShouldTranscode decided to transcode, Reason : %s",ReasonWhy.c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Transcoding", ReasonWhy);
  }
  else
  {
    CLog::Log(LOGDEBUG,"RPi ShouldTranscode decided not to transcode");
  }

  return bShouldTranscode;
}

///////////////////////////////////////////////////////////////////////////////
std::string CPlexTranscoderClientRPi::GetCurrentBitrate(bool local)
{
  return boost::lexical_cast<std::string>(m_maxVideoBitrate);
}




