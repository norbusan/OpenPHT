//
//  PlexJobs.cpp
//  Plex Home Theater
//
//  Created by Tobias Hieta on 2013-08-14.
//
//

#include "PlexJobs.h"
#include "FileSystem/PlexDirectory.h"

#include "FileSystem/PlexFile.h"

#include "TextureCache.h"
#include "File.h"
#include "utils/Crc32.h"
#include "PlexFile.h"
#include "video/VideoInfoTag.h"
#include "Stopwatch.h"
#include "PlexUtils.h"
#include "xbmc/Util.h"
#include "ApplicationMessenger.h"

#define TEXTURE_CACHE_BUFFER_SIZE 131072

////////////////////////////////////////////////////////////////////////////////
bool CPlexHTTPFetchJob::DoWork()
{
  return m_http.Get(m_url.Get(), m_data);
}

////////////////////////////////////////////////////////////////////////////////
bool CPlexHTTPFetchJob::operator==(const CJob* job) const
{
  const CPlexHTTPFetchJob *f = static_cast<const CPlexHTTPFetchJob*>(job);
  return m_url.Get() == f->m_url.Get();
}

////////////////////////////////////////////////////////////////////////////////
bool CPlexDirectoryFetchJob::DoWork()
{
  return m_dir.GetDirectory(m_url.Get(), m_items);
}

////////////////////////////////////////////////////////////////////////////////
CFileItemListPtr CPlexDirectoryFetchJob::getResult()
{
  CFileItemListPtr list = CFileItemListPtr(new CFileItemList());
  list->Copy(m_items);
  return list;
}

////////////////////////////////////////////////////////////////////////////////
bool CPlexMediaServerClientJob::DoWork()
{
  bool success = false;
  
  if (m_verb == "PUT")
    success = m_http.Put(m_url.Get(), m_data);
  else if (m_verb == "GET")
    success = m_http.Get(m_url.Get(), m_data);
  else if (m_verb == "DELETE")
    success = m_http.Delete(m_url.Get(), m_data);
  else if (m_verb == "POST")
    success = m_http.Post(m_url.Get(), m_postData, m_data);
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////
bool CPlexVideoThumbLoaderJob::DoWork()
{
  if (!m_item->IsPlexMediaServer())
    return false;

  CStdStringArray art;
  art.push_back("smallThumb");
  art.push_back("smallPoster");
  art.push_back("smallGrandparentThumb");
  art.push_back("banner");

  int i = 0;
  BOOST_FOREACH(CStdString artKey, art)
  {
    if (m_item->HasArt(artKey) &&
        !CTextureCache::Get().HasCachedImage(m_item->GetArt(artKey)))
      CTextureCache::Get().BackgroundCacheImage(m_item->GetArt(artKey));

    if (ShouldCancel(i++, art.size()))
      return false;
  }

  return true;
}

using namespace XFILE;

////////////////////////////////////////////////////////////////////////////////////////
bool
CPlexDownloadFileJob::DoWork()
{
  CFile file;
  CURL theUrl(m_url);
  m_http.SetRequestHeader("X-Plex-Client", PLEX_TARGET_NAME);

  if (!file.OpenForWrite(m_destination, true))
  {
    CLog::Log(LOGWARNING, "[DownloadJob] Couldn't open file %s for writing", m_destination.c_str());
    return false;
  }

  if (m_http.Open(theUrl))
  {
    CLog::Log(LOGINFO, "[DownloadJob] Downloading %s to %s", m_url.c_str(), m_destination.c_str());

    bool done = false;
    bool failed = false;
    int64_t read;
    int64_t leftToDownload = m_http.GetLength();
    int64_t total = leftToDownload;

    while (!done)
    {
      char buffer[4096];
      read = m_http.Read(buffer, 4096);
      if (read > 0)
      {
        leftToDownload -= read;
        file.Write(buffer, read);
        done = ShouldCancel(total-leftToDownload, total);
        if(done) failed = true;
      }
      else if (read == 0)
      {
        done = true;
        failed = total == 0;
        continue;
      }

      if (total == 0)
        done = true;
    }

    CLog::Log(LOGINFO, "[DownloadJob] Done with the download.");

    m_http.Close();
    file.Close();

    return !failed;
  }

  CLog::Log(LOGWARNING, "[DownloadJob] Failed to download file.");
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexThemeMusicPlayerJob::DoWork()
{
  if (m_themeUrl.empty())
    return false;

  Crc32 crc;
  crc.ComputeFromLowerCase(m_themeUrl);

  CStdString hex;
  hex.Format("%08x", (unsigned int)crc);

  m_fileToPlay = "special://masterprofile/ThemeMusicCache/" + hex + ".mp3";

  if (!XFILE::CFile::Exists(m_fileToPlay))
  {
    CPlexFile plex;
    CFile localFile;

    if (!localFile.OpenForWrite(m_fileToPlay, true))
    {
      CLog::Log(LOGWARNING, "CPlexThemeMusicPlayerJob::DoWork failed to open %s for writing.", m_fileToPlay.c_str());
      return false;
    }

    bool failed = false;

    if (plex.Open(m_themeUrl))
    {
      bool done = false;
      int64_t read = 0;

      while(!done)
      {
        char buffer[4096];
        read = plex.Read(buffer, 4096);
        if (read > 0)
        {
          localFile.Write(buffer, read);
          done = ShouldCancel(0, 0);
          if (done) failed = true;
        }
        else if (read == 0)
        {
          done = true;
          continue;
        }
      }
    }

    CLog::Log(LOGDEBUG, "CPlexThemeMusicPlayerJob::DoWork cached %s => %s", m_themeUrl.c_str(), m_fileToPlay.c_str());

    plex.Close();
    localFile.Close();

    return !failed;
  }
  else
    return true;
}

#ifdef TARGET_RASPBERRY_PI

bool CPlexUpdaterJob::DoWork()
{
  CStdString command, output;
  CStdString contents;
  CStdString kernel, system, kernel_md5, system_md5, post_update;
  int step  = 1;
  int steps = 9;
  m_continue = true;
  m_cancelled = false;


  if (!m_updating)
  {
    m_updating = true;

    m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);
    if (m_dlgProgress)
    {
      m_dlgProgress->SetHeading(2);
    }
    m_dlgProgress->SetHeading("Updating RasPlex");
    m_dlgProgress->StartModal();
    m_dlgProgress->ShowProgressBar(true);
   
    if (m_continue)
    {
      SetProgress("Preparing directory", step++, steps); // 1
      command = "mkdir -p /storage/.update/tmp";
      StreamExec(command);
    }
    if (m_continue)
    {

      SetProgress("Extracting update... this will take a minute or two.", step++, steps); // 2
      command = "tar -xf "+CSpecialProtocol::TranslatePath(m_localBinary)+" -C /storage/.update/tmp";
      output = StreamExec(command);
    }
    if (m_continue)
    {

      SetProgress("Examining archive...", step++, steps); // 3
      command = "find /storage/.update/tmp/";
      contents = StreamExec(command);
    }
    if (m_continue)
    {

      SetProgress("Checking archive integrity...", step++, steps); // 4
      command = "echo \""+contents+"\" | tr \" \" \"\n\" | grep KERNEL$ | sed ':a;N;$!ba;s/\\n//g' | tr \"\n\" \" \"";
      kernel  = StreamExec(command);
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::DoWork: kernel: %s", kernel.c_str());

      command = "echo \""+contents+"\" | tr \" \" \"\n\" | grep SYSTEM$ | sed ':a;N;$!ba;s/\\n//g' | tr \"\n\" \" \"";
      system  = StreamExec(command);
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::DoWork: system: %s", system.c_str());

      command = "echo \""+contents+"\" | tr \" \" \"\n\" | grep KERNEL.md5 | sed ':a;N;$!ba;s/\\n//g' | tr \"\n\" \" \"";
      kernel_md5  = StreamExec(command);
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::DoWork: kernel check: %s", kernel_md5.c_str());

      command = "echo \""+contents+"\" | tr \" \" \"\n\" | grep SYSTEM.md5 | sed ':a;N;$!ba;s/\\n//g' | tr \"\n\" \" \"";
      system_md5  = StreamExec(command);
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::DoWork: system check: %s", system_md5.c_str());

      command = "echo \""+contents+"\" | tr \" \" \"\n\" | grep post_update.sh | sed ':a;N;$!ba;s/\\n//g' | tr \"\n\" \" \"";
      post_update = StreamExec(command); 
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::DoWork: post_update: %s", post_update.c_str());
    }
    if (m_continue)
    {

      SetProgress("Examining archive...", step++, steps); // 5
      // check if any of the above were empty
    }
    if (m_continue)
    {

      SetProgress("Validating checksums", step++, steps); // 6

      /*
      // compute and compare checksums 
      kernel_check=`/bin/md5sum $KERNEL | awk '{print $1}'`
      system_check=`/bin/md5sum $SYSTEM | awk '{print $1}'`

      kernelmd5=`cat $KERNELMD5 | awk '{print $1}'`
      systemmd5=`cat $SYSTEMMD5 | awk '{print $1}'`

      [ "$kernel_check" != "$kernelmd5" ] && abort 'Kernel checksum mismatch'
      [ "$system_check" != "$systemmd5" ] && abort 'System checksum mismatch'
      */
    }
    if (m_continue)
    {
      SetProgress("Preparing update files", step++, steps); // 7
      StreamExec("mv "+kernel+" "+system+" "+kernel_md5+" "+system_md5+" /storage/.update");
      // move the files to the update directory
    }
    if (m_continue)
    {
      SetProgress("Running post-update steps", step++, steps); // 8

      /*
      POST_UPDATE_PATH=/storage/.post_update.sh
      if [ -n $POST_UPDATE ] && [-f "$POST_UPDATE" ];then
        notify 'Running post update script'
        cp $POST_UPDATE $POST_UPDATE_PATH
        post_update
        notify 'Post-update complete!'
      fi
      */
    }
    if (m_continue)
    {
      // remove temporary folders
      SetProgress("Cleaning up", step++, steps); // 9
      StreamExec("rm -rf /storage/.update/tmp");
      StreamExec("rm "+CSpecialProtocol::TranslatePath(m_localBinary));
    }
    if (m_continue)
    {
      m_autoupdater -> WriteUpdateInfo();
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Update is complete!", "System will reboot twice to apply.", 10000, false);
      m_updating = false;
      CApplicationMessenger::Get().Restart();
    }
  }else{
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Invalid action", "Update already in progress.", 10000, false);
  }                                                                                                                                 

}

CStdString CPlexUpdaterJob::StreamExec(CStdString command)
{
  CStdString output = "";
  char* errorstr;
  command += " 2>&1";
  CLog::Log(LOGNOTICE,"CPlexUpdaterJob::StreamExec : Executing '%s'", command.c_str());
  FILE* fp = popen(command.c_str(), "r");
  if(!fp)
  {
    errorstr = strerror(errno);
    CLog::Log(LOGNOTICE,"Failed to open process, got error %s", errorstr );
    m_continue = false;
    CancelUpdate(errorstr);
    return output;
  }

  char buffer[128];
  CStdString commandOutput; 

  while(!feof(fp) ){
    if (! m_cancelled)
    {
      m_cancelled = m_dlgProgress->IsCanceled();
      if (m_cancelled)
      {
        CancelUpdate("Update cancelled by user");
        return output;
      }
    }
    if ( fgets(buffer, sizeof(buffer), fp)!=NULL )
    {
      output += buffer;
      commandOutput = CStdString(buffer);
      CLog::Log(LOGNOTICE, "CPlexUpdaterJob::StreamExec: %s",commandOutput.c_str());
    }
  }

  int retcode = pclose(fp);

  if (retcode)
  {
    CLog::Log(LOGERROR,"CPlexUpdaterJob::StreamExec: error %d while running command", retcode);
  }

  return output;
}


void CPlexUpdaterJob::CancelUpdate(char* error)
{

  CLog::Log(LOGERROR,"UpdateCancelled");
  StreamExec("rm -rf /storage/.update/tmp");
  StreamExec("rm /storage/.plexht/userdata/plexupdateinfo.xml");
  m_dlgProgress->Close();
  CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Update failed!", error, 10000, false);
  m_continue = false;
  m_updating = false;
}


void CPlexUpdaterJob::SetProgress(char* message, int step, int steps)
{
  CStdString progressMsg;
  CStdString update;
  float percentage = ((step * 100 / steps)/100);

  update.Format( " %d / %d : '%s'", step, steps, message );

  m_dlgProgress->SetLine(0, update);

  if (percentage > 0)
    progressMsg.Format( "Progress : %2d%%", percentage);
  else
    progressMsg = "";

  m_dlgProgress->SetLine(2, progressMsg);
  m_dlgProgress->SetPercentage(percentage);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexRecursiveFetchJob::DoWork()
{
  CUtil::GetRecursiveListing(m_url, *m_list, m_exts);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexTextureCacheJob::CacheTexture(CBaseTexture **texture)
{
  // unwrap the URL as required
  std::string additional_info;
  unsigned int width, height;
  CStdString image = DecodeImageURL(m_url, width, height, additional_info);

  m_details.updateable = additional_info != "music" && UpdateableURL(image);

  // generate the hash
  m_details.hash = GetImageHash(image);
  if (m_details.hash.empty())
    return false;
  else if (m_details.hash == m_oldHash)
    return true;

  unsigned char buffer[TEXTURE_CACHE_BUFFER_SIZE];
  bool outputFileOpenned = false;

  // our buffer cache plus some padding
  m_inputFile.SetBufferSize(TEXTURE_CACHE_BUFFER_SIZE + 1024);

  if (m_inputFile.Open(image))
  {
    while (true)
    {
      int bytesRead = m_inputFile.Read(buffer, TEXTURE_CACHE_BUFFER_SIZE);
      if (bytesRead == 0)
        break;

      // eventually open output file depending upon filetype
      if (!outputFileOpenned)
      {
        // we want to check the HTTP fail code
        if (m_inputFile.GetLastHTTPResponseCode() > 299)
        {
          CLog::Log(LOGERROR, "CPlexTextureCacheJob::CacheTexture failed to get image from %s, got code: %ld", m_url.c_str(), m_inputFile.GetLastHTTPResponseCode());
          m_inputFile.Close();
          return false;
        }

        // we need to check if its a jpg or png
        if ((buffer[0] == 0xFF) && (buffer[1] == 0xD8))
        {
          m_details.file = m_cachePath + ".jpg";
        }
        else if ((buffer[0]) == 0x89 && (buffer[1] == 0x50))
        {
          m_details.file = m_cachePath + ".png";
        }
        else
        {
          CLog::Log(LOGERROR, "CPlexTextureCacheJob::CacheTexture invalid image header at URL: %s", m_url.c_str());
          m_inputFile.Close();
          return false;
        }

        // now open the file
        if (m_outputFile.OpenForWrite(CTextureCache::GetCachedPath(m_details.file), true))
        {
          outputFileOpenned = true;
        }
        else
        {
          m_inputFile.Close();
          CLog::Log(LOGERROR,"CPlexTextureCacheJob::CacheTexture unable to open output file %s",CTextureCache::GetCachedPath(m_details.file).c_str());
          return false;
        }
      }

      m_outputFile.Write(buffer, bytesRead);
    }

    m_outputFile.Flush();
    m_inputFile.Close();
    m_outputFile.Close();
	
    if (texture)
      *texture = CTextureCacheJob::LoadImage(CTextureCache::GetCachedPath(m_details.file), width, height, additional_info);
	
    return true;
  }
  else
  {
    CLog::Log(LOGERROR,"CPlexTextureCacheJob::CacheTexture unable to open input file %s",image.c_str());
    return false;
  }
}
