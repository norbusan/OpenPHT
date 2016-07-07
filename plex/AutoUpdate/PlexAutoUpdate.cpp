//
//  PlexAutoUpdate.cpp
//  Plex
//
//  Created by Tobias Hieta <tobias@plexapp.com> on 2012-10-24.
//  Copyright 2012 Plex Inc. All rights reserved.
//

#include "PlexAutoUpdate.h"
#include <boost/foreach.hpp>
#include "FileSystem/PlexDirectory.h"
#include "FileItem.h"
#include "PlexJobs.h"
#include "File.h"
#include "Directory.h"
#include "utils/URIUtils.h"
#include "settings/GUISettings.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "ApplicationMessenger.h"
#include "filesystem/SpecialProtocol.h"
#include "PlexApplication.h"
#include "Client/MyPlex/MyPlexManager.h"
#include "LocalizeStrings.h"
#include "utils/SystemInfo.h"

#include "xbmc/Util.h"
#include "XBDateTime.h"
#include "GUIInfoManager.h"
#include "Application.h"
#include "PlexAnalytics.h"
#include "GUIUserMessages.h"

#ifdef TARGET_WINDOWS
#include "win32/WIN32Util.h"
#endif

using namespace XFILE;

//#define UPDATE_DEBUG 1

///////////////////////////////////////////////////////////////////////////////////////////////////
CPlexAutoUpdate::CPlexAutoUpdate()
  : m_forced(false), m_isSearching(false), m_isDownloading(false), m_ready(false), m_percentage(0)
{
  m_url = CURL("https://services.openpht.tv/updater/check.xml");

  m_searchFrequency = 21600000; /* 6 hours */

  CheckInstalledVersion();

  CLog::Log(LOGDEBUG,"CPlexAutoUpdate : Creating Updater, auto=%d",g_guiSettings.GetBool("updates.auto"));
  g_plexApplication.timer->SetTimeout(5 * 1000, this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::CheckInstalledVersion()
{
  if (g_application.GetReturnedFromAutoUpdate())
  {
    CStdString currentVersion = g_infoManager.GetVersion();
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::CheckInstalledVersion We are returning from a autoupdate with version %s", currentVersion.c_str());

    std::string version, packageHash, fromVersion;
    bool isDelta;

    if (GetUpdateInfo(version, isDelta, packageHash, fromVersion))
    {
      if (version != currentVersion)
      {
        CLog::Log(LOGDEBUG, "CPlexAutoUpdate::CheckInstalledVersion Seems like we failed to upgrade from %s to %s, will NOT try this version again.", fromVersion.c_str(), currentVersion.c_str());
      }
      else
      {
        CLog::Log(LOGDEBUG, "CPlexAutoUpdate::CheckInstalledVersion Seems like we succeeded to upgrade correctly!");
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::OnTimeout()
{
  CFileItemList list;
  CPlexDirectory dir;
  m_isSearching = true;

  CLog::Log(LOGDEBUG,"CPlexAutoUpdate::OnTimeout Starting");

  CStdString currentVersion = g_infoManager.GetVersion();
  m_url.SetOption("version", currentVersion);
  m_url.SetOption("build", PLEX_BUILD_TAG);

#if defined(TARGET_DARWIN)
  m_url.SetOption("distribution", "macosx");
#elif defined(TARGET_WINDOWS)
  m_url.SetOption("distribution", "windows");
#elif defined(TARGET_RASPBERRY_PI)
  m_url.SetOption("distribution", "rasplex");
#elif defined(TARGET_OPENELEC)
  m_url.SetOption("distribution", "openelec");
#elif defined(TARGET_LINUX)
  m_url.SetOption("distribution", "linux");
#else
  m_url.SetOption("distribution", "unknown");
#endif

#ifdef TARGET_RASPBERRY_PI
  m_url.SetOption("serial", readProcCPUInfoValue("Serial"));
  m_url.SetOption("revision", readProcCPUInfoValue("Revision"));
#endif

  int channel = g_guiSettings.GetInt("updates.channel");
  m_url.SetOption("channel", boost::lexical_cast<std::string>(channel));

#ifdef TARGET_WINDOWS
  CStdString vcredistVersion;
  // Microsoft Visual C++ 2015 Redistributable (x86)
  if (CWIN32Util::GetInstallerDependenciesVersion("{23daf363-3020-4059-b3ae-dc4ad39fed19}", vcredistVersion))
    m_url.SetOption("vcredist14", vcredistVersion);
  // Microsoft Visual C++ 2013 Redistributable (x86)
  if (CWIN32Util::GetInstallerDependenciesVersion("{f65db027-aff3-4070-886a-0d87064aabb1}", vcredistVersion))
    m_url.SetOption("vcredist12", vcredistVersion);
#endif

  std::vector<std::string> alreadyTriedVersion = GetAllInstalledVersions();
  CFileItemList updates;

  bool manualUpdateRequired = false;

  dir.SetUserAgent(g_sysinfo.GetUserAgent());
  if (dir.GetDirectory(m_url, list))
  {
    m_isSearching = false;

    if (list.Size() > 0)
    {
      for (int i = 0; i < list.Size(); i++)
      {
        CFileItemPtr updateItem = list.Get(i);
        if (updateItem->HasProperty("version") &&
            updateItem->GetProperty("autoupdate").asBoolean() &&
            updateItem->GetProperty("version").asString() != currentVersion)
        {
          CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnTimeout got version %s from update endpoint", updateItem->GetProperty("version").asString().c_str());
          if (std::find(alreadyTriedVersion.begin(), alreadyTriedVersion.end(), updateItem->GetProperty("version").asString()) == alreadyTriedVersion.end())
            updates.Add(updateItem);
          else
          {
            CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnTimeout We have already tried to install %s, skipping it.", updateItem->GetProperty("version").asString().c_str());
            manualUpdateRequired = true;
          }
        }
        else if (updateItem->HasProperty("version") &&
            !updateItem->GetProperty("autoupdate").asBoolean() &&
            updateItem->GetProperty("version").asString() != currentVersion)
        {
          manualUpdateRequired = true;
        }
      }
    }
  }

  CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnTimeout found %d candidates", updates.Size());
  CFileItemPtr selectedItem;

  for (int i = 0; i < updates.Size(); i++)
  {
    if (!selectedItem)
      selectedItem = updates.Get(i);
    else
    {
      CDateTime time1, time2;
      time1.SetFromDBDateTime(selectedItem->GetProperty("unprocessed_createdAt").asString().substr(0, 19));
      time2.SetFromDBDateTime(list.Get(i)->GetProperty("unprocessed_createdAt").asString().substr(0, 19));

      if (time2 > time1)
        selectedItem = updates.Get(i);
    }
  }

  if (selectedItem)
  {
    CLog::Log(LOGINFO, "CPlexAutoUpdate::OnTimeout update found! %s", selectedItem->GetProperty("version").asString().c_str());
    if (m_forced || g_guiSettings.GetBool("updates.auto"))
      DownloadUpdate(selectedItem);

    if (m_forced)
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Update available!", "A new version is downloading in the background", 10000, false);
      m_forced = false;
    }
    return;
  }

  if (manualUpdateRequired)
  {
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnTimeout manual update available");

    if (m_forced)
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Update available!", "A new version requires manual update", 10000, false);
      m_forced = false;
    }
  }
  else
  {
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnTimeout no updates available");

    if (m_forced)
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "No update available!", "You are up-to-date!", 10000, false);
      m_forced = false;
    }
  }

  if (g_guiSettings.GetBool("updates.auto"))
    g_plexApplication.timer->SetTimeout(m_searchFrequency, this);

  m_isSearching = false;
  CGUIMessage msg(GUI_MSG_UPDATE, WINDOW_SETTINGS_SYSTEM, WINDOW_SETTINGS_MYPICTURES);
  CApplicationMessenger::Get().SendGUIMessage(msg, WINDOW_SETTINGS_SYSTEM);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CFileItemPtr CPlexAutoUpdate::GetPackage(CFileItemPtr updateItem)
{
  CFileItemPtr deltaItem, fullItem;

  if (updateItem && updateItem->m_mediaItems.size() > 0)
  {
    for (int i = 0; i < updateItem->m_mediaItems.size(); i ++)
    {
      CFileItemPtr package = updateItem->m_mediaItems[i];
      if (package->GetProperty("delta").asBoolean())
        deltaItem = package;
      else
        fullItem = package;
    }
  }

  if (deltaItem)
    return deltaItem;

  return fullItem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexAutoUpdate::NeedDownload(const std::string& localFile, const std::string& expectedHash, bool isManifest)
{
#ifdef TARGET_OPENELEC
  // Manifest is not needed in OpenELEC
  if (isManifest)
    return false;
#endif

  if (CFile::Exists(localFile, false) && PlexUtils::GetSHA1SumFromURL(CURL(localFile)) == expectedHash)
  {
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::DownloadUpdate we already have %s with correct SHA", localFile.c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::DownloadUpdate(CFileItemPtr updateItem)
{
  if (m_downloadItem)
    return;

  m_downloadPackage = GetPackage(updateItem);
  if (!m_downloadPackage)
    return;

  m_isDownloading = true;
  m_downloadItem = updateItem;
  m_needManifest = m_needBinary = m_needApplication = false;

  CDirectory::Create("special://temp/autoupdate");

  CStdString manifestUrl = m_downloadPackage->GetProperty("manifestPath").asString();
  CStdString updateUrl = m_downloadPackage->GetProperty("filePath").asString();
//  CStdString applicationUrl = m_downloadItem->GetProperty("updateApplication").asString();

  bool isDelta = m_downloadPackage->GetProperty("delta").asBoolean();
  std::string packageStr = isDelta ? "delta" : "full";
  m_localManifest = "special://temp/autoupdate/manifest-" + m_downloadItem->GetProperty("version").asString() + "." + packageStr + ".xml.bz2";
  m_localBinary = "special://temp/autoupdate/" + m_downloadPackage->GetProperty("fileName").asString();

  if (NeedDownload(m_localManifest, m_downloadPackage->GetProperty("manifestHash").asString(), true))
  {
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::DownloadUpdate need %s", manifestUrl.c_str());
    CJobManager::GetInstance().AddJob(new CPlexDownloadFileJob(manifestUrl, m_localManifest), this, CJob::PRIORITY_LOW);
    m_needManifest = true;
  }

  if (NeedDownload(m_localBinary, m_downloadPackage->GetProperty("fileHash").asString(), false))
  {
    CLog::Log(LOGDEBUG, "CPlexAutoUpdate::DownloadUpdate need %s", m_localBinary.c_str());
    CJobManager::GetInstance().AddJob(new CPlexDownloadFileJob(updateUrl, m_localBinary), this, CJob::PRIORITY_LOW);
    m_needBinary = true;
  }

  if (!m_needBinary && !m_needManifest)
    ProcessDownloads();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> CPlexAutoUpdate::GetAllInstalledVersions() const
{
  CXBMCTinyXML doc;
  std::vector<std::string> versions;

  if (!doc.LoadFile("special://profile/plexupdateinfo.xml"))
    return versions;

  if (!doc.RootElement())
    return versions;

  for (TiXmlElement *version = doc.RootElement()->FirstChildElement(); version; version = version->NextSiblingElement())
  {
    std::string verStr;
    if (version->QueryStringAttribute("version", &verStr) == TIXML_SUCCESS)
      versions.push_back(verStr);
  }

  return versions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexAutoUpdate::GetUpdateInfo(std::string& verStr, bool& isDelta, std::string& packageHash, std::string& fromVersion) const
{
  CXBMCTinyXML doc;

  if (!doc.LoadFile("special://profile/plexupdateinfo.xml"))
    return false;

  if (!doc.RootElement())
    return false;

  int installedTm = 0;
  for (TiXmlElement *version = doc.RootElement()->FirstChildElement(); version; version = version->NextSiblingElement())
  {
    int iTm;
    if (version->QueryIntAttribute("installtime", &iTm) == TIXML_SUCCESS)
    {
      if (iTm > installedTm)
        installedTm = iTm;
      else
        continue;
    }
    else
      continue;

    if (version->QueryStringAttribute("version", &verStr) != TIXML_SUCCESS)
      continue;

#ifndef TARGET_OPENELEC
    if (version->QueryBoolAttribute("delta", &isDelta) != TIXML_SUCCESS)
      continue;

    if (version->QueryStringAttribute("packageHash", &packageHash) != TIXML_SUCCESS)
      continue;
#endif

    if (version->QueryStringAttribute("fromVersion", &fromVersion) != TIXML_SUCCESS)
      continue;
  }

  CLog::Log(LOGDEBUG, "CPlexAutoUpdate::GetUpdateInfo Latest version we tried to install is %s", verStr.c_str());

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::WriteUpdateInfo()
{
  CXBMCTinyXML doc;

  if (!doc.LoadFile("special://profile/plexupdateinfo.xml"))
    doc = CXBMCTinyXML();

  TiXmlElement *versions = doc.RootElement();
  if (!versions)
  {
    doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
    versions = new TiXmlElement("versions");
    doc.LinkEndChild(versions);
  }

  TiXmlElement thisVersion("version");
  thisVersion.SetAttribute("version", m_downloadItem->GetProperty("version").asString());
#ifndef TARGET_OPENELEC
  thisVersion.SetAttribute("delta", m_downloadPackage->GetProperty("delta").asBoolean());
  thisVersion.SetAttribute("packageHash", m_downloadPackage->GetProperty("manifestHash").asString());
#endif
  thisVersion.SetAttribute("fromVersion", std::string(g_infoManager.GetVersion()));
  thisVersion.SetAttribute("installtime", time(NULL));

  if (versions->FirstChildElement())
    versions->InsertBeforeChild(versions->FirstChildElement(), thisVersion);
  else
    versions->InsertEndChild(thisVersion);

  doc.SaveFile("special://profile/plexupdateinfo.xml");


  CURL callback = CURL("https://services.openpht.tv/updater/updated.xml");

  callback.SetOption("version", m_downloadItem->GetProperty("version").asString());
  callback.SetOption("fromVersion", g_infoManager.GetVersion());
  callback.SetOption("build", PLEX_BUILD_TAG);

#if defined(TARGET_DARWIN)
  callback.SetOption("distribution", "macosx");
#elif defined(TARGET_WINDOWS)
  callback.SetOption("distribution", "windows");
#elif defined(TARGET_RASPBERRY_PI)
  callback.SetOption("distribution", "rasplex");
#elif defined(TARGET_OPENELEC)
  callback.SetOption("distribution", "openelec");
#elif defined(TARGET_LINUX)
  callback.SetOption("distribution", "linux");
#else
  callback.SetOption("distribution", "unknown");
#endif

#ifdef TARGET_RASPBERRY_PI
  callback.SetOption("serial", readProcCPUInfoValue("Serial"));
  callback.SetOption("revision", readProcCPUInfoValue("Revision"));
#endif

  int channel = g_guiSettings.GetInt("updates.channel");
  callback.SetOption("channel", boost::lexical_cast<std::string>(channel));

  CStdString data;
  XFILE::CPlexFile http;
  http.SetUserAgent(g_sysinfo.GetUserAgent());
  bool httpSuccess = http.Get(callback.Get(), data);
  if (httpSuccess)
    CLog::Log(LOGNOTICE, "CPlexAutoUpdate::WriteUpdateInfo updated, got %s seconds", data.c_str());
  else
    CLog::Log(LOGERROR, "CPlexAutoUpdate::WriteUpdateInfo failed to update, got %s seconds", data.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::ProcessDownloads()
{
  CStdString verStr;
  verStr.Format("Version %s is now ready to be installed.", GetUpdateVersion());
  if (!g_application.IsPlayingFullScreenVideo())
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Update available!", verStr, 10000, false);

  CGUIMessage msg(GUI_MSG_UPDATE, PLEX_AUTO_UPDATER, 0);
  CApplicationMessenger::Get().SendGUIMessage(msg, WINDOW_HOME);

  msg = CGUIMessage(GUI_MSG_UPDATE, WINDOW_SETTINGS_SYSTEM, WINDOW_SETTINGS_MYPICTURES);
  CApplicationMessenger::Get().SendGUIMessage(msg, WINDOW_SETTINGS_SYSTEM);

  m_isDownloading = false;
  m_ready = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  CPlexDownloadFileJob *fj = static_cast<CPlexDownloadFileJob*>(job);
  if (fj && success)
  {
    if (fj->m_destination == m_localManifest)
    {
      if (NeedDownload(m_localManifest, m_downloadPackage->GetProperty("manifestHash").asString(), true))
      {
        CLog::Log(LOGWARNING, "CPlexAutoUpdate::OnJobComplete failed to download manifest, SHA mismatch. Retrying in %d seconds", m_searchFrequency);
        return;
      }

      CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnJobComplete got manifest.");
      m_needManifest = false;
    }
    else if (fj->m_destination == m_localBinary)
    {
      if (NeedDownload(m_localBinary, m_downloadPackage->GetProperty("fileHash").asString(), false))
      {
        CLog::Log(LOGWARNING, "CPlexAutoUpdate::OnJobComplete failed to download update, SHA mismatch. Retrying in %d seconds", m_searchFrequency);
        return;
      }

      CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnJobComplete got update binary.");
      m_needBinary = false;
    }
    else
      CLog::Log(LOGDEBUG, "CPlexAutoUpdate::OnJobComplete What is %s", fj->m_destination.c_str());
  }
  else if (!success)
  {
    CLog::Log(LOGWARNING, "CPlexAutoUpdate::OnJobComplete failed to run a download job, will retry in %d milliseconds.", m_searchFrequency);
    g_plexApplication.timer->SetTimeout(m_searchFrequency, this);
    return;
  }

  if (!m_needApplication && !m_needBinary && !m_needManifest)
    ProcessDownloads();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
  const CPlexDownloadFileJob *fj = static_cast<const CPlexDownloadFileJob*>(job);
  if (!fj || fj->m_destination != m_localBinary)
    return;

  int percentage = (int)((float)progress / float(total) * 100.0);
  if (percentage > m_percentage)
  {
    m_percentage = percentage;
    CGUIMessage msg(GUI_MSG_UPDATE, WINDOW_SETTINGS_SYSTEM, WINDOW_SETTINGS_MYPICTURES);
    CApplicationMessenger::Get().SendGUIMessage(msg, WINDOW_SETTINGS_SYSTEM);
  }
}

#ifdef TARGET_POSIX
#include <signal.h>
#endif

#ifdef TARGET_DARWIN_OSX
#include "DarwinUtils.h"
#endif
 
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
std::string quoteArgs(const std::list<std::string>& arguments)
{
	std::string quotedArgs;
	for (std::list<std::string>::const_iterator iter = arguments.begin();
	     iter != arguments.end();
	     iter++)
	{
		std::string arg = *iter;

		bool isQuoted = !arg.empty() &&
		                 arg.at(0) == '"' &&
		                 arg.at(arg.size()-1) == '"';

		if (!isQuoted && arg.find(' ') != std::string::npos)
		{
			arg.insert(0,"\"");
			arg.append("\"");
		}
		quotedArgs += arg;
		quotedArgs += " ";
	}
	return quotedArgs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(TARGET_DARWIN_OSX) || defined(TARGET_WINDOWS)
void CPlexAutoUpdate::UpdateAndRestart()
{  
  /* first we need to copy the updater app to our tmp directory, it might change during install.. */
  CStdString updaterPath;
  CUtil::GetHomePath(updaterPath);

#ifdef TARGET_DARWIN_OSX
  updaterPath += "/tools/updater";
#elif TARGET_WINDOWS
  updaterPath += "\\updater.exe";
#endif

#ifdef TARGET_DARWIN_OSX
  std::string updater = CSpecialProtocol::TranslatePath("special://temp/autoupdate/updater");
#elif TARGET_WINDOWS
  std::string updater = CSpecialProtocol::TranslatePath("special://temp/autoupdate/updater.exe");
#endif

  if (!CopyFile(updaterPath.c_str(), updater.c_str(), false))
  {
    CLog::Log(LOGWARNING, "CPlexAutoUpdate::UpdateAndRestart failed to copy %s to %s", updaterPath.c_str(), updater.c_str());
    return;
  }

#ifdef TARGET_POSIX
  chmod(updater.c_str(), 0755);
#endif

  std::string script = CSpecialProtocol::TranslatePath(m_localManifest);
  std::string packagedir = CSpecialProtocol::TranslatePath("special://temp/autoupdate");
  CStdString appdir;

#ifdef TARGET_DARWIN_OSX
  char installdir[2*MAXPATHLEN];

  uint32_t size;
  GetDarwinBundlePath(installdir, &size);

  appdir = std::string(installdir);

#elif TARGET_WINDOWS
  CUtil::GetHomePath(appdir);
#endif

  WriteUpdateInfo();

#ifdef TARGET_POSIX
  CStdString args;
  args.Format("--install-dir \"%s\" --package-dir \"%s\" --script \"%s\" --auto-close", appdir, packagedir, script);

  CStdString exec;
  exec.Format("\"%s\" %s", updater, args);

  CLog::Log(LOGDEBUG, "CPlexAutoUpdate::UpdateAndRestart going to run %s", exec.c_str());
  fprintf(stderr, "Running: %s\n", exec.c_str());

  pid_t pid = fork();
  if (pid == -1)
  {
    CLog::Log(LOGWARNING, "CPlexAutoUpdate::UpdateAndRestart major fail when installing update, can't fork!");
    return;
  }
  else if (pid == 0)
  {
    /* hack! we don't know the parents all open file descriptiors, so we need
     * to loop over them and kill them :( not nice! */
    struct rlimit rlim;

    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
      fprintf(stderr, "Couldn't get the max number of fd's!");
      exit(1);
    }

    int maxFd = rlim.rlim_cur;
    fprintf(stderr, "Total number of fd's %d\n", maxFd);
    for (int i = 3; i < maxFd; ++i)
      close(i);

    /* Child */
    pid_t parentPid = getppid();
    fprintf(stderr, "Waiting for PHT to quit...\n");

    time_t start = time(NULL);

    while(kill(parentPid, SIGHUP) == 0)
    {
      /* wait for parent process 30 seconds... */
      if ((time(NULL) - start) > 30)
      {
        fprintf(stderr, "PHT still haven't quit after 30 seconds, let's be a bit more forceful...sending KILL to %d\n", parentPid);
        kill(parentPid, SIGKILL);
        usleep(1000 * 100);
        start = time(NULL);
      }
      else
        usleep(1000 * 10);
    }

    fprintf(stderr, "PHT seems to have quit, running updater\n");

    system(exec.c_str());

    _exit(0);
  }
  else
  {
    CApplicationMessenger::Get().Quit();
  }
#elif TARGET_WINDOWS
  DWORD pid = GetCurrentProcessId();

  std::list<std::string> args;
  args.push_back(updater);

  args.push_back("--wait");
  args.push_back(boost::lexical_cast<std::string>(pid));
  
  args.push_back("--install-dir");
  args.push_back(appdir);

  args.push_back("--package-dir");
  args.push_back(packagedir);

  args.push_back("--script");
  args.push_back(script);

  args.push_back("--auto-close");

  char *arguments = strdup(quoteArgs(args).c_str());

  CLog::Log(LOGDEBUG, "CPlexAutoUpdate::UpdateAndRestart going to run %s", arguments);

	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo,sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo,sizeof(processInfo));

  // set a working directory, otherwise it will fail
  // to remove the current dir.
  //
  char tmpDir[MAX_PATH + 1];
  if (GetTempPath(MAX_PATH, tmpDir) == 0)
    strcpy(tmpDir, "c:\\");

  if (CreateProcess(updater.c_str(), arguments, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, tmpDir, &startupInfo, &processInfo) == 0)
  {
    CLog::Log(LOGWARNING, "CPlexAutoUpdate::UpdateAndRestart CreateProcess failed! %d", GetLastError());
  }
  else
  {
    //CloseHandle(pInfo.hProcess);
    //CloseHandle(pInfo.hProcess);
    CApplicationMessenger::Get().Quit();
  }

  free(arguments);

#endif
}

#else

void CPlexAutoUpdate::UpdateAndRestart()
{
#ifdef TARGET_OPENELEC
  CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Preparing update", "System will reboot twice to apply.", 10000, false);
  if (XFILE::CFile::Rename(m_localBinary, "/storage/.update/" + m_downloadPackage->GetProperty("fileName").asString()))
  {
    WriteUpdateInfo();
    CApplicationMessenger::Get().Restart();
  }
#endif
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::ForceVersionCheckInBackground()
{
  m_forced = true;
  m_isSearching = true;

  // restart with a short time out, just to make sure that we get it running in the background thread
  g_plexApplication.timer->RestartTimeout(1, this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::ResetTimer()
{
  if (g_guiSettings.GetBool("updates.auto"))
  {
    g_plexApplication.timer->RestartTimeout(m_searchFrequency, this);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexAutoUpdate::PokeFromSettings()
{
  if (g_guiSettings.GetBool("updates.auto"))
    g_plexApplication.timer->RestartTimeout(m_searchFrequency, this);
  else if (!g_guiSettings.GetBool("updates.auto"))
    g_plexApplication.timer->RemoveTimeout(this);
}
