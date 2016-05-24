#include "GUIDialogPlexAudioSubtitlePicker.h"
#include "plex/PlexTypes.h"
#include "LocalizeStrings.h"
#include "GUIWindowManager.h"
#include "FileItem.h"
#include "Client/PlexServerManager.h"
#include "Client/PlexMediaServerClient.h"
#include "ApplicationMessenger.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <string>

#include "PlexUtils.h"
#include "PlexApplication.h"
#include "Application.h"
#include "Settings.h"
#include "settings/GUISettings.h"

CGUIDialogPlexPicker::CGUIDialogPlexPicker(int id, const CStdString& xml, bool audio)
  : CGUIDialogSelect(id, xml)
{
  m_audio = audio;
}

bool
CGUIDialogPlexPicker::OnMessage(CGUIMessage &msg)
{
  if (msg.GetMessage() == GUI_MSG_WINDOW_INIT)
  {
    SetHeading(g_localizeStrings.Get(m_audio ? 52100 : 52101));
    if (g_application.IsPlaying() && g_application.CurrentFileItemPtr())
      SetFileItem(g_application.CurrentFileItemPtr());
    else if (g_plexApplication.m_preplayItem)
      SetFileItem(g_plexApplication.m_preplayItem);
  }
  
  bool ret = CGUIDialogSelect::OnMessage(msg);

  if (msg.GetMessage() == GUI_MSG_CLICKED)
  {
    if (msg.GetParam1() == ACTION_SELECT_ITEM)
    {
      UpdateStreamSelection();
    }
  }

  return ret;
}

void
CGUIDialogPlexPicker::SetFileItem(CFileItemPtr& fileItem)
{
  if (!fileItem || fileItem->m_mediaItems.empty())
    return;

  m_fileItem = fileItem;
  
  if (!m_audio)
  {
    // Subtitles must always have a None entry as well.
    CFileItem* noneItem = new CFileItem;
    
    noneItem->SetLabel(g_localizeStrings.Get(231));
    noneItem->SetProperty("id", 0);
    noneItem->SetProperty("streamType", boost::lexical_cast<std::string>(PLEX_STREAM_SUBTITLE));
    noneItem->Select(false);
    
    Add(noneItem);
  }
  
  CFileItemPtr part;
  for (int i = 0; !part && i < fileItem->m_mediaItems.size(); i++)
    for (int j = 0; !part && j < fileItem->m_mediaItems[i]->m_mediaParts.size(); j++)
      if (fileItem->GetPath() == fileItem->m_mediaItems[i]->m_mediaParts[j]->GetPath())
        part = fileItem->m_mediaItems[i]->m_mediaParts[j];
  if (!part && !fileItem->m_mediaItems.empty() && !fileItem->m_mediaItems[0]->m_mediaParts.empty())
    part = fileItem->m_mediaItems[0]->m_mediaParts[0];
  if (!part)
    return;

  int index = 0;
  bool hasSelection = false;
  for (int y = 0; y < part->m_mediaPartStreams.size(); y ++)
  {
    CFileItemPtr stream = part->m_mediaPartStreams[y];
    int streamType = stream->GetProperty("streamType").asInteger();
    if (stream && ((m_audio && streamType == PLEX_STREAM_AUDIO) ||
                   (!m_audio && streamType == PLEX_STREAM_SUBTITLE)))
    {
      if (stream->IsSelected())
        hasSelection = true;
      Add(stream.get());
      index++;
    }
  }
  
  if (!hasSelection && !m_audio)
    m_vecList->Get(0)->Select(true);
}

void CGUIDialogPlexPicker::UpdateStreamSelection()
{
  CFileItemPtr selectedStream;
  for (int i = 0; i < m_vecList->Size() ; i ++)
  {
    if (m_vecList->Get(i)->IsSelected())
      selectedStream = m_vecList->Get(i);
  }
  
  if (!selectedStream)
    return;
  
  PlexUtils::SetSelectedStream(m_fileItem, selectedStream);

  if (g_application.CurrentFileItemPtr()->GetPath() == m_fileItem->GetPath() &&
      g_application.IsPlayingVideo() && g_application.m_pPlayer)
  {
    IPlayer *player = g_application.m_pPlayer;
    int64_t streamId = selectedStream->GetProperty("id").asInteger();
    bool audioStreamChanged = false;

    if (m_audio)
    {
      if (player->GetAudioStreamPlexID() != streamId)
      {
        player->SetAudioStreamPlexID(streamId);
        g_settings.m_currentVideoSettings.m_AudioStream = player->GetAudioStream();
        audioStreamChanged = true;
      }
    }
    else
    {
      bool visible = streamId != 0;
      player->SetSubtitleVisible(visible);

      if (visible)
      {
        player->SetSubtitleStreamPlexID(streamId);
        g_settings.m_currentVideoSettings.m_SubtitleStream = player->GetSubtitle();
      }
    }

    if (m_fileItem->GetProperty("plexDidTranscode").asBoolean() && (audioStreamChanged || g_guiSettings.GetBool("plexmediaserver.transcodesubtitles")))
      /* since we are transcoding, we need to stop the current session and restart it on the server */
      CApplicationMessenger::Get().MediaRestart(false);
  }
}

bool
CGUIDialogPlexPicker::ShowAndGetInput(CFileItemPtr& fileItem, bool audio)
{
  CGUIDialogPlexPicker *dialog = (CGUIDialogPlexPicker*)g_windowManager.GetWindow(audio ? WINDOW_DIALOG_PLEX_AUDIO_PICKER : WINDOW_DIALOG_PLEX_SUBTITLE_PICKER);
  if (!dialog) return false;
  
  dialog->Reset();
  
  //dialog->SetButtonText(g_localizeStrings.Get(222));
  dialog->SetHeading(g_localizeStrings.Get(audio ? 52100 : 52101));
  dialog->SetFileItem(fileItem);
  dialog->DoModal();
  if (dialog->m_bConfirmed)
    dialog->UpdateStreamSelection();
  
  return dialog->m_bConfirmed;
}
