#include "PlexAnalytics.h"

#include "URL.h"
#include "settings/GUISettings.h"
#include "Settings.h"
#include "threads/Timer.h"
#include "filesystem/CurlFile.h"
#include "FileSystem/PlexFile.h"
#include "Application.h"
#include "interfaces/AnnouncementManager.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "cores/VideoRenderers/RenderManager.h"
#include "log.h"
#include "PlexJobs.h"
#include "JobManager.h"
#include "GUIInfoManager.h"
#include "PlexApplication.h"
#include "GUIWindowSlideShow.h"
#include "GUIWindowManager.h"

#define ANALYTICS_TID_PHT "UA-6111912-18"

#define INITIAL_EVENT_DELAY   3
#define PING_INTERVAL_SECONDS 13500

///////////////////////////////////////////////////////////////////////////////////////////////////
CPlexAnalytics::CPlexAnalytics() : m_firstEvent(true), m_numberOfPlays(0)
{

  ANNOUNCEMENT::CAnnouncementManager::AddAnnouncer(this);

  /*
  m_defaultArgs["v"] = "1";
  m_defaultArgs["tid"] = ANALYTICS_TID_PMS;
  m_defaultArgs["ul"] = "en-us";
  m_defaultArgs["an"] = "Plex Media Server";
  m_defaultArgs["av"] = Cocoa_GetAppVersion();
  m_defaultArgs["cid"] = GetMachineIdentifier();
  */
  m_baseOptions.AddOption("v", "1");
  m_baseOptions.AddOption("tid", ANALYTICS_TID_PHT);
  m_baseOptions.AddOption("ul", g_guiSettings.GetString("locale.language"));
#ifdef TARGET_RASPBERRY_PI
  m_baseOptions.AddOption("an", "RasPlex");
#elif defined(TARGET_OPENELEC)
  m_baseOptions.AddOption("an", "OpenPHT-Embedded");
#else
  m_baseOptions.AddOption("an", "OpenPHT");
#endif
  m_baseOptions.AddOption("av", g_infoManager.GetVersion());
  m_baseOptions.AddOption("cid", g_guiSettings.GetString("system.uuid"));

  /* let's jump through some hoops to get some resolution info */
#if 0
  RESOLUTION res = g_renderManager.GetResolution();
  RESOLUTION_INFO resInfo = g_settings.m_ResInfo[res];
  CStdString resStr;
  resStr.Format("%dx%d", resInfo.iWidth, resInfo.iHeight);

  m_baseOptions.AddOption("sr", resStr);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::startLogging()
{
  g_plexApplication.timer->RemoveTimeout(this);
  g_plexApplication.timer->SetTimeout(INITIAL_EVENT_DELAY * 1000, this);
  m_sessionLength.restart();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::stopLogging()
{
  g_plexApplication.timer->RemoveTimeout(this);
  m_sessionLength.restart();
  ANNOUNCEMENT::CAnnouncementManager::RemoveAnnouncer(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::didUpgradeEvent(bool success, const std::string &fromVersion, const std::string &toVersion, bool delta)
{
  CUrlOptions o;
  o.AddOption("cd9", (int)delta);
  o.AddOption("cd10", fromVersion);
  o.AddOption("cd11", toVersion);
  trackEvent("App", "Upgrade", success ? "success" : "failed", (int)success, o);
}

typedef std::pair<std::string, std::string> strPair;

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::setCustomDimensions(CUrlOptions &options)
{
  /*
    Custom Dimension 1 (cd1=): X-Plex-Product
    Custom Dimension 2 (cd2=): X-Plex-Client-Identifier
    Custom Dimension 3 (cd3=): X-Plex-Platform
    Custom Dimension 4 (cd4=): X-Plex-Platform-Version
    Custom Dimension 5 (cd5=): X-Plex-Device
    Custom Dimension 6 (cd6=): X-Plex-Version
   */

  std::vector<strPair> headers = XFILE::CPlexFile::GetHeaderList();
  strPair pair;

  BOOST_FOREACH(pair, headers)
  {
    if (pair.first == "X-Plex-Product")
      options.AddOption("cd1", pair.second);
    if (pair.first == "X-Plex-Client-Identifier")
      options.AddOption("cd2", pair.second);
    /* Instead of sending X-Plex-Platform in cd3, that is always set to
     * Plex Home Theater for PHT, we are going to send the OS since this
     * make much more sense for us to track.
     */
    if (pair.first == "X-Plex-Model")
      options.AddOption("cd3", pair.second);
    if (pair.first == "X-Plex-Platform-Version")
      options.AddOption("cd4", pair.second);
    if (pair.first == "X-Plex-Device")
      options.AddOption("cd5", pair.second);
    if (pair.first == "X-Plex-Version")
      options.AddOption("cd6", pair.second);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::trackEvent(const std::string &category, const std::string &action, const std::string &label, int64_t value, const CUrlOptions &args)
{
  //if (!g_guiSettings.GetBool("advanced.collectanalytics"))
    return;

  CUrlOptions opts(m_baseOptions);
  opts.AddOptions(args);

  opts.AddOption("t", "event");

  if (!category.empty())
    opts.AddOption("ec", category);
  if(!action.empty())
    opts.AddOption("ea", action);
  if (!label.empty())
    opts.AddOption("el", label);

  opts.AddOption("ev", boost::lexical_cast<std::string>(value));

  setCustomDimensions(opts);
  sendTrackingRequest(opts);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::sendPing()
{
  trackEvent("App", "Ping", "", 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::sendTrackingRequest(const CUrlOptions &request)
{
  //if (!g_guiSettings.GetBool("advanced.collectanalytics"))
    return;

  CURL u("http://www.google-analytics.com/collect");
  CPlexMediaServerClientJob *j = new CPlexMediaServerClientJob(u, "POST");
  j->m_postData = request.GetOptionsString();

  CLog::Log(LOGDEBUG, "CPlexAnalytics::sendTrackingRequest sending %s", j->m_postData.c_str());

#ifndef _DEBUG //don't send analytics for test builds
  CJobManager::GetInstance().AddJob(j, NULL, CJob::PRIORITY_LOW);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::sendPlaybackStop()
{
  m_cumulativeTimePlayed += m_playStopWatch.GetElapsedSeconds();
  m_playStopWatch.Stop();

  CLog::Log(LOGDEBUG, "CPlexAnalytics::Announce played item done, time played: %lld", m_cumulativeTimePlayed);

  trackEvent("Playback",
             m_currentItem->GetProperty("type").asString(),
             m_currentItem->GetProperty("identifier").asString(),
             m_cumulativeTimePlayed);
  g_plexApplication.timer->RestartTimeout(PING_INTERVAL_SECONDS * 1000, this);

  m_currentItem.reset();
  m_numberOfPlays += 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  //if (!g_guiSettings.GetBool("advanced.collectanalytics"))
    return;

  if (flag == ANNOUNCEMENT::System && (stricmp(sender, "xbmc") == 0))
  {
    if (stricmp(message, "OnQuit") == 0)
    {
      CUrlOptions o;
      o.AddOption("cm1", boost::lexical_cast<std::string>(m_numberOfPlays));
      trackEvent("App", "Shutdown", "", m_sessionLength.elapsed(), o);

      g_plexApplication.timer->RemoveTimeout(this);
    }
  }
  else if (flag == ANNOUNCEMENT::Player && (stricmp(sender, "xbmc") == 0))
  {
    if (stricmp(message, "OnPlay") == 0)
    {
      CFileItemPtr item = g_application.CurrentFileItemPtr();
      if (data["item"].isObject() && data["item"].isMember("type") && data["item"]["type"] == "picture")
      {
        CGUIWindowSlideShow *slideshow = (CGUIWindowSlideShow*)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
        if (slideshow && slideshow->IsActive())
          item = slideshow->GetCurrentSlide();
      }

      if (!item)
      {
        CLog::Log(LOGWARNING, "CPlexAnalytics::Announce no item found on PlayEvent");
        return;
      }

      if (!m_currentItem || (m_currentItem->GetPath() != item->GetPath()))
      {
        if (m_currentItem)
          // It doesn't seem like we have sent a stop event yet.
          sendPlaybackStop();

        m_currentItem = CFileItemPtr(new CFileItem(*item.get()));
        CLog::Log(LOGDEBUG, "CPlexAnalytics::Announce starting playing of a new item.");
        m_cumulativeTimePlayed = 0;
      }
      else
        CLog::Log(LOGDEBUG, "CPlexAnalytics::Announce resuming playback of current item %s", m_currentItem->GetPath().c_str());

      m_playStopWatch.StartZero();
    }
    else if (stricmp(message, "OnPause") == 0 && m_currentItem)
    {
      CLog::Log(LOGDEBUG, "CPlexAnalytics::Announce pausing playback");
      m_cumulativeTimePlayed += m_playStopWatch.GetElapsedSeconds();
      m_playStopWatch.Stop();
    }
    else if (stricmp(message, "OnStop") == 0 && m_currentItem)
      sendPlaybackStop();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAnalytics::OnTimeout()
{
  if (m_firstEvent)
  {
    m_firstEvent = false;
    CUrlOptions o("sc=start");
    trackEvent("App", "Startup", "", 1, o);
  }
  else
    sendPing();

  g_plexApplication.timer->SetTimeout(PING_INTERVAL_SECONDS * 1000, this);
}
