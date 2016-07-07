/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://xbmc.org
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

#include "PlayerController.h"
#include "dialogs/GUIDialogSlider.h"
#include "settings/AdvancedSettings.h"
#include "settings/GUISettings.h"
#include "settings/Settings.h"
#include "cores/IPlayer.h"
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUISliderControl.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "video/dialogs/GUIDialogAudioSubtitleSettings.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#include "cores/VideoRenderers/OverlayRendererGUI.h"
#endif
#include "Application.h"
#include "utils/StringUtils.h"

CPlayerController::CPlayerController()
  : m_sliderAction(0)
{
}

CPlayerController::~CPlayerController()
{
}

CPlayerController& CPlayerController::GetInstance()
{
  static CPlayerController instance;
  return instance;
}

bool CPlayerController::OnAction(const CAction &action)
{
  const unsigned int MsgTime = 300;
  const unsigned int DisplTime = 2000;

  if (g_application.m_pPlayer->IsPlayingVideo())
  {
    switch (action.GetID())
    {
      case ACTION_SHOW_SUBTITLES:
      {
        if (g_application.m_pPlayer->GetSubtitleCount() == 0)
          return true;

        bool subsOn = !g_application.m_pPlayer->GetSubtitleVisible();
        g_application.m_pPlayer->SetSubtitleVisible(subsOn);
        CStdString sub;
        if (subsOn)
        {
          SPlayerSubtitleStreamInfo info;
          g_application.m_pPlayer->GetSubtitleStreamInfo(CURRENT_STREAM, info);
          sub = (!info.name.empty()) ? CStdString(info.name) : g_localizeStrings.Get(231);
        }
        else
          sub = g_localizeStrings.Get(231);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                              g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_NEXT_SUBTITLE:
      case ACTION_CYCLE_SUBTITLE:
      {
        if (g_application.m_pPlayer->GetSubtitleCount() == 0)
          return true;

        int currentSub = g_application.m_pPlayer->GetSubtitle();
        bool currentSubVisible = true;

        if (g_application.m_pPlayer->GetSubtitleVisible())
        {
          if (++currentSub >= g_application.m_pPlayer->GetSubtitleCount())
          {
            currentSub = 0;
            if (action.GetID() == ACTION_NEXT_SUBTITLE)
            {
              g_application.m_pPlayer->SetSubtitleVisible(false);
              currentSubVisible = false;
            }
          }
          g_application.m_pPlayer->SetSubtitle(currentSub);
        }
        else if (action.GetID() == ACTION_NEXT_SUBTITLE)
        {
          g_application.m_pPlayer->SetSubtitleVisible(true);
        }

        CStdString sub;
        if (currentSubVisible)
        {
          SPlayerSubtitleStreamInfo info;
          g_application.m_pPlayer->GetSubtitleStreamInfo(currentSub, info);
          sub = (!info.name.empty()) ? CStdString(info.name) : g_localizeStrings.Get(231);
        }
        else
          sub = g_localizeStrings.Get(231);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_SUBTITLE_DELAY_MIN:
      {
        g_settings.m_currentVideoSettings.m_SubtitleDelay -= 0.1f;
        if (g_settings.m_currentVideoSettings.m_SubtitleDelay < -g_advancedSettings.m_videoSubsDelayRange)
          g_settings.m_currentVideoSettings.m_SubtitleDelay = -g_advancedSettings.m_videoSubsDelayRange;
        g_application.m_pPlayer->SetSubTitleDelay(g_settings.m_currentVideoSettings.m_SubtitleDelay);

        ShowSlider(action.GetID(), 22006, g_settings.m_currentVideoSettings.m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange);

        /* PLEX */
        CStdString delay;
        delay.Format("%.0f ms", g_settings.m_currentVideoSettings.m_SubtitleDelay * 1000);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(52623), delay , DisplTime, false, MsgTime);
        /* END PLEX */

        return true;
      }

      case ACTION_SUBTITLE_DELAY_PLUS:
      {
        g_settings.m_currentVideoSettings.m_SubtitleDelay += 0.1f;
        if (g_settings.m_currentVideoSettings.m_SubtitleDelay > g_advancedSettings.m_videoSubsDelayRange)
          g_settings.m_currentVideoSettings.m_SubtitleDelay = g_advancedSettings.m_videoSubsDelayRange;
        g_application.m_pPlayer->SetSubTitleDelay(g_settings.m_currentVideoSettings.m_SubtitleDelay);

        ShowSlider(action.GetID(), 22006, g_settings.m_currentVideoSettings.m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange);

        /* PLEX */
        CStdString delay;
        delay.Format("%.0f ms", g_settings.m_currentVideoSettings.m_SubtitleDelay * 1000);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(52623), delay , DisplTime, false, MsgTime);
        /* END PLEX */

        return true;
      }

      case ACTION_SUBTITLE_DELAY:
      {
        ShowSlider(action.GetID(), 22006, g_settings.m_currentVideoSettings.m_SubtitleDelay,
                                          -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                           g_advancedSettings.m_videoSubsDelayRange, true);
        return true;
      }

      case ACTION_AUDIO_DELAY:
      {
        ShowSlider(action.GetID(), 297, g_settings.m_currentVideoSettings.m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange, true);
        return true;
      }

      case ACTION_AUDIO_DELAY_MIN:
      {
        g_settings.m_currentVideoSettings.m_AudioDelay -= 0.025f;
        if (g_settings.m_currentVideoSettings.m_AudioDelay < -g_advancedSettings.m_videoAudioDelayRange)
          g_settings.m_currentVideoSettings.m_AudioDelay = -g_advancedSettings.m_videoAudioDelayRange;
        g_application.m_pPlayer->SetAVDelay(g_settings.m_currentVideoSettings.m_AudioDelay);

        ShowSlider(action.GetID(), 297, g_settings.m_currentVideoSettings.m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange);
        return true;
      }

      case ACTION_AUDIO_DELAY_PLUS:
      {
        g_settings.m_currentVideoSettings.m_AudioDelay += 0.025f;
        if (g_settings.m_currentVideoSettings.m_AudioDelay > g_advancedSettings.m_videoAudioDelayRange)
          g_settings.m_currentVideoSettings.m_AudioDelay = g_advancedSettings.m_videoAudioDelayRange;
        g_application.m_pPlayer->SetAVDelay(g_settings.m_currentVideoSettings.m_AudioDelay);

        ShowSlider(action.GetID(), 297, g_settings.m_currentVideoSettings.m_AudioDelay,
                                        -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                         g_advancedSettings.m_videoAudioDelayRange);
        return true;
      }

      case ACTION_AUDIO_NEXT_LANGUAGE:
      {
        if (g_application.m_pPlayer->GetAudioStreamCount() == 1)
          return true;

        int currentAudio = g_application.m_pPlayer->GetAudioStream();

        if (++currentAudio >= g_application.m_pPlayer->GetAudioStreamCount())
          currentAudio = 0;
        g_application.m_pPlayer->SetAudioStream(currentAudio);    // Set the audio stream to the one selected
        SPlayerAudioStreamInfo info;
        g_application.m_pPlayer->GetAudioStreamInfo(currentAudio, info);
        CStdString aud = info.name;
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(460), aud, DisplTime, false, MsgTime);
        return true;
      }

      case ACTION_ZOOM_IN:
      {
        g_settings.m_currentVideoSettings.m_CustomZoomAmount += 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomZoomAmount > 2.f)
          g_settings.m_currentVideoSettings.m_CustomZoomAmount = 2.f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 216, g_settings.m_currentVideoSettings.m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_ZOOM_OUT:
      {
        g_settings.m_currentVideoSettings.m_CustomZoomAmount -= 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomZoomAmount < 0.5f)
          g_settings.m_currentVideoSettings.m_CustomZoomAmount = 0.5f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 216, g_settings.m_currentVideoSettings.m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_INCREASE_PAR:
      {
        g_settings.m_currentVideoSettings.m_CustomPixelRatio += 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomPixelRatio > 2.f)
          g_settings.m_currentVideoSettings.m_CustomZoomAmount = 2.f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 217, g_settings.m_currentVideoSettings.m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_DECREASE_PAR:
      {
        g_settings.m_currentVideoSettings.m_CustomPixelRatio -= 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomZoomAmount < 0.5f)
          g_settings.m_currentVideoSettings.m_CustomPixelRatio = 0.5f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 217, g_settings.m_currentVideoSettings.m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_VSHIFT_UP:
      {
        g_settings.m_currentVideoSettings.m_CustomVerticalShift -= 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomVerticalShift < -2.0f)
          g_settings.m_currentVideoSettings.m_CustomVerticalShift = -2.0f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 225, g_settings.m_currentVideoSettings.m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_VSHIFT_DOWN:
      {
        g_settings.m_currentVideoSettings.m_CustomVerticalShift += 0.01f;
        if (g_settings.m_currentVideoSettings.m_CustomVerticalShift > 2.0f)
          g_settings.m_currentVideoSettings.m_CustomVerticalShift = 2.0f;
        g_settings.m_currentVideoSettings.m_ViewMode = VIEW_MODE_CUSTOM;
        g_renderManager.SetViewMode(VIEW_MODE_CUSTOM);
        ShowSlider(action.GetID(), 225, g_settings.m_currentVideoSettings.m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
        return true;
      }

      case ACTION_SUBTITLE_VSHIFT_UP:
      {
        RESOLUTION_INFO res_info = g_graphicsContext.GetResInfo();
        int subalign = g_guiSettings.GetInt("subtitles.align");
        if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
        {
          res_info.iSubtitles ++;
          if (res_info.iSubtitles >= res_info.iHeight)
            res_info.iSubtitles = res_info.iHeight - 1;

          ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        }
        else
        {
          res_info.iSubtitles --;
          if (res_info.iSubtitles < 0)
            res_info.iSubtitles = 0;

          if (subalign == SUBTITLE_ALIGN_MANUAL)
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
          else
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
        }
        g_graphicsContext.SetResInfo(g_graphicsContext.GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_SUBTITLE_VSHIFT_DOWN:
      {
        RESOLUTION_INFO res_info =  g_graphicsContext.GetResInfo();
        int subalign = g_guiSettings.GetInt("subtitles.align");
        if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
        {
          res_info.iSubtitles--;
          if (res_info.iSubtitles < 0)
            res_info.iSubtitles = 0;

          ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        }
        else
        {
          res_info.iSubtitles++;
          if (res_info.iSubtitles >= res_info.iHeight)
            res_info.iSubtitles = res_info.iHeight - 1;

          if (subalign == SUBTITLE_ALIGN_MANUAL)
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
          else
            ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
        }
        g_graphicsContext.SetResInfo(g_graphicsContext.GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_SUBTITLE_ALIGN:
      {
        RESOLUTION_INFO res_info = g_graphicsContext.GetResInfo();
        int subalign = g_guiSettings.GetInt("subtitles.align");

        subalign++;
        if (subalign > SUBTITLE_ALIGN_TOP_OUTSIDE)
          subalign = SUBTITLE_ALIGN_MANUAL;

        res_info.iSubtitles = res_info.iHeight - 1;

        g_guiSettings.SetInt("subtitles.align", subalign);
        CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                              g_localizeStrings.Get(21460),
                                              g_localizeStrings.Get(21461 + subalign), 
                                              TOAST_DISPLAY_TIME, false);
        g_graphicsContext.SetResInfo(g_graphicsContext.GetVideoResolution(), res_info);
        return true;
      }

      case ACTION_VOLAMP_UP:
      case ACTION_VOLAMP_DOWN:
      {
        // Don't allow change with passthrough audio
        if (g_application.m_pPlayer->IsPassthrough())
        {
          CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning,
                                                g_localizeStrings.Get(660),
                                                g_localizeStrings.Get(29802),
                                                TOAST_DISPLAY_TIME, false);
          return false;
        }

        float sliderMax = VOLUME_DRC_MAXIMUM / 100.0f;
        float sliderMin = VOLUME_DRC_MINIMUM / 100.0f;

        if (action.GetID() == ACTION_VOLAMP_UP)
          g_settings.m_currentVideoSettings.m_VolumeAmplification += 1.0f;
        else
          g_settings.m_currentVideoSettings.m_VolumeAmplification -= 1.0f;

        g_settings.m_currentVideoSettings.m_VolumeAmplification =
          std::max(std::min(g_settings.m_currentVideoSettings.m_VolumeAmplification, sliderMax), sliderMin);

        g_application.m_pPlayer->SetDynamicRangeCompression((long)(g_settings.m_currentVideoSettings.m_VolumeAmplification * 100));

        ShowSlider(action.GetID(), 660, g_settings.m_currentVideoSettings.m_VolumeAmplification, sliderMin, 1.0f, sliderMax);
        return true;
      }

      case ACTION_VOLAMP:
      {
        float sliderMax = VOLUME_DRC_MAXIMUM / 100.0f;
        float sliderMin = VOLUME_DRC_MINIMUM / 100.0f;
        ShowSlider(action.GetID(), 660,
                   g_settings.m_currentVideoSettings.m_VolumeAmplification,
                   sliderMin, 1.0f, sliderMax, true);
        return true;
      }

      default:
        break;
    }
  }
  return false;
}

void CPlayerController::ShowSlider(int action, int label, float value, float min, float delta, float max, bool modal)
{
  m_sliderAction = action;
  if (modal)
    CGUIDialogSlider::ShowAndGetInput(g_localizeStrings.Get(label), value, min, delta, max, this);
  else
    CGUIDialogSlider::Display(label, value, min, delta, max, this);
}

void CPlayerController::OnSliderChange(void *data, CGUISliderControl *slider)
{
  if (!slider)
    return;

  if (m_sliderAction == ACTION_ZOOM_OUT || m_sliderAction == ACTION_ZOOM_IN ||
      m_sliderAction == ACTION_INCREASE_PAR || m_sliderAction == ACTION_DECREASE_PAR ||
      m_sliderAction == ACTION_VSHIFT_UP || m_sliderAction == ACTION_VSHIFT_DOWN ||
      m_sliderAction == ACTION_SUBTITLE_VSHIFT_UP || m_sliderAction == ACTION_SUBTITLE_VSHIFT_DOWN)
  {
    std::string strValue = StringUtils::Format("%1.2f",slider->GetFloatValue());
    slider->SetTextValue(strValue);
  }
  else if (m_sliderAction == ACTION_VOLAMP_UP ||
          m_sliderAction == ACTION_VOLAMP_DOWN ||
          m_sliderAction == ACTION_VOLAMP)
    slider->SetTextValue(CGUIDialogAudioSubtitleSettings::FormatDecibel(slider->GetFloatValue(), 1.0f));
  else
    slider->SetTextValue(CGUIDialogAudioSubtitleSettings::FormatDelay(slider->GetFloatValue(), 0.025f));

  if (g_application.m_pPlayer->HasPlayer())
  {
    if (m_sliderAction == ACTION_AUDIO_DELAY)
    {
      g_settings.m_currentVideoSettings.m_AudioDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetAVDelay(g_settings.m_currentVideoSettings.m_AudioDelay);
    }
    else if (m_sliderAction == ACTION_SUBTITLE_DELAY)
    {
      g_settings.m_currentVideoSettings.m_SubtitleDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetSubTitleDelay(g_settings.m_currentVideoSettings.m_SubtitleDelay);
    }
    else if (m_sliderAction == ACTION_VOLAMP)
    {
      g_settings.m_currentVideoSettings.m_VolumeAmplification = slider->GetFloatValue();
      g_application.m_pPlayer->SetDynamicRangeCompression((long)(g_settings.m_currentVideoSettings.m_VolumeAmplification * 100));
    }
  }
}
