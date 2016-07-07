/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#include "DVDVideoCodec.h"
#include "windowing/WindowingFactory.h"
#include "settings/Settings.h"
#include "settings/GUISettings.h"

bool CDVDVideoCodec::IsSettingVisible(const std::string &condition, const std::string &value, const CSetting *setting)
{
  if (setting == NULL || value.empty())
    return false;

  const std::string &settingId = setting->GetSetting();

  // check if we are running on nvidia hardware
  std::string gpuvendor = g_Windowing.GetRenderVendor();
  std::transform(gpuvendor.begin(), gpuvendor.end(), gpuvendor.begin(), ::tolower);
  bool isNvidia = (gpuvendor.compare(0, 6, "nvidia") == 0);
  bool isIntel = (gpuvendor.compare(0, 5, "intel") == 0);

  // nvidia does only need mpeg-4 setting
  if (isNvidia) 
  {
    if (settingId == "videoplayer.usevdpaumpeg4")
      return true;

    return false; //will also hide intel settings on nvidia hardware
  }
  else if (isIntel) // intel needs vc1, mpeg-2 and mpeg4 setting
  {
    if (settingId == "videoplayer.usevaapimpeg4")
      return true;
    if (settingId == "videoplayer.usevaapivc1")
      return true;
    if (settingId == "videoplayer.usevaapimpeg2")
      return true;

    return false; //this will also hide nvidia settings on intel hardware
  }
  // if we don't know the hardware we are running on e.g. amd oss vdpau 
  // or fglrx with xvba-driver we show everything
  return true;
}

bool CDVDVideoCodec::IsCodecDisabled(const std::map<AVCodecID, std::string> &map, AVCodecID id)
{
  auto codec = map.find(id);
  if (codec != map.end())
  {
    return (!g_guiSettings.GetBool(codec->second.c_str()) ||
            !CDVDVideoCodec::IsSettingVisible("unused", "unused",
                                              g_guiSettings.GetSetting(codec->second.c_str())));
  }
  return false; //don't disable what we don't have
}
