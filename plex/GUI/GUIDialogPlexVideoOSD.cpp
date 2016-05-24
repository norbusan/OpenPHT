#include "GUIDialogPlexVideoOSD.h"
#include "guilib/GUIWindowManager.h"
#include "Application.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
CGUIDialogPlexVideoOSD::CGUIDialogPlexVideoOSD()
  : m_openedFromPause(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialogPlexVideoOSD::OnAction(const CAction &action)
{
  int id = action.GetID();
  switch (id)
  {
    case ACTION_NEXT_ITEM:
    case ACTION_PREV_ITEM:
      return false;

    case ACTION_NAV_BACK:
      if (m_openedFromPause || g_application.IsPaused())
      {
        g_application.StopPlaying();
        return true;
      }
      break;

    case ACTION_SHOW_INFO:
      return true;
  }

  return CGUIDialogVideoOSD::OnAction(action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialogPlexVideoOSD::OnMessage(CGUIMessage &message)
{
  if (message.GetMessage() == GUI_MSG_WINDOW_INIT)
  {
    m_openedFromPause = (message.GetStringParam(0) == "pauseOpen");
    if (m_openedFromPause)
      SetAutoClose(500);
  }
  return CGUIDialogVideoOSD::OnMessage(message);
}
