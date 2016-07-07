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
#include "system.h"

#include "AEFactory.h"

#include "Engines/ActiveAE/ActiveAE.h"

#include "settings/GUISettings.h"

IAE* CAEFactory::AE = NULL;
static float  g_fVolume = 1.0f;
static bool   g_bMute = false;

IAE *CAEFactory::GetEngine()
{
  return AE;
}

bool CAEFactory::LoadEngine()
{
  /* can only load the engine once, XBMC restart is required to change it */
  if (AE)
    return false;

  AE = new ActiveAE::CActiveAE();

  if (AE && !AE->CanInit())
  {
    delete AE;
    AE = NULL;
  }

  return AE != NULL;
}

void CAEFactory::UnLoadEngine()
{
  if(AE)
  {
    AE->Shutdown();
    delete AE;
    AE = NULL;
  }
}

bool CAEFactory::StartEngine()
{
  if (!AE)
    return false;

  if (AE->Initialize())
    return true;

  delete AE;
  AE = NULL;
  return false;
}

bool CAEFactory::Suspend()
{
  if(AE)
    return AE->Suspend();

  return false;
}

bool CAEFactory::Resume()
{
  if(AE)
    return AE->Resume();

  return false;
}

bool CAEFactory::IsSuspended()
{
  if(AE)
    return AE->IsSuspended();

  /* No engine to process audio */
  return true;
}

/* engine wrapping */
IAESound *CAEFactory::MakeSound(const std::string &file)
{
  if(AE)
    return AE->MakeSound(file);
  
  return NULL;
}

void CAEFactory::FreeSound(IAESound *sound)
{
  if(AE)
    AE->FreeSound(sound);
}

void CAEFactory::SetSoundMode(const int mode)
{
  if(AE)
    AE->SetSoundMode(mode);
}

void CAEFactory::OnSettingsChange(std::string setting)
{
  if(AE)
    AE->OnSettingsChange(setting);
}

void CAEFactory::EnumerateOutputDevices(AEDeviceList &devices, bool passthrough)
{
  if(AE)
    AE->EnumerateOutputDevices(devices, passthrough);
}

void CAEFactory::VerifyOutputDevice(std::string &device, bool passthrough)
{
  AEDeviceList devices;
  EnumerateOutputDevices(devices, passthrough);
  std::string firstDevice;

  for (AEDeviceList::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt)
  {
    /* remember the first device so we can default to it if required */
    if (firstDevice.empty())
      firstDevice = deviceIt->second;

    if (deviceIt->second == device)
      return;
    else if (deviceIt->first == device)
    {
      device = deviceIt->second;
      return;
    }
  }

  /* if the device wasnt found, set it to the first viable output */
  device = firstDevice;
}

std::string CAEFactory::GetDefaultDevice(bool passthrough)
{
  if(AE)
    return AE->GetDefaultDevice(passthrough);

  return "default";
}

bool CAEFactory::SupportsRaw(AEDataFormat format, int samplerate)
{
  // check if passthrough is enabled
  if (!g_guiSettings.GetBool("audiooutput.passthrough"))
    return false;

  // fixed config disabled passthrough
  if (g_guiSettings.GetInt("audiooutput.config") == AE_CONFIG_FIXED)
    return false;

  // check if the format is enabled in settings
  if (format == AE_FMT_AC3 && !g_guiSettings.GetBool("audiooutput.ac3passthrough"))
    return false;
  if (format == AE_FMT_DTS && !g_guiSettings.GetBool("audiooutput.dtspassthrough"))
    return false;
  if (format == AE_FMT_EAC3 && !g_guiSettings.GetBool("audiooutput.eac3passthrough"))
    return false;
  if (format == AE_FMT_TRUEHD && !g_guiSettings.GetBool("audiooutput.truehdpassthrough"))
    return false;
  if (format == AE_FMT_DTSHD && !g_guiSettings.GetBool("audiooutput.dtshdpassthrough"))
    return false;

  if(AE)
    return AE->SupportsRaw(format, samplerate);

  return false;
}

bool CAEFactory::SupportsSilenceTimeout()
{
  if(AE)
    return AE->SupportsSilenceTimeout();

  return false;
}

bool CAEFactory::HasStereoAudioChannelCount()
{
  if(AE)
    return AE->HasStereoAudioChannelCount();
  return false;
}

bool CAEFactory::HasHDAudioChannelCount()
{
  if(AE)
    return AE->HasHDAudioChannelCount();
  return false;
}

/**
  * Returns true if current AudioEngine supports at lest two basic quality levels
  * @return true if quality setting is supported, otherwise false
  */
bool CAEFactory::SupportsQualitySetting(void) 
{
  if (!AE)
    return false;

  return ((AE->SupportsQualityLevel(AE_QUALITY_LOW)? 1 : 0) + 
          (AE->SupportsQualityLevel(AE_QUALITY_MID)? 1 : 0) +
          (AE->SupportsQualityLevel(AE_QUALITY_HIGH)? 1 : 0)) >= 2; 
}
  
void CAEFactory::SetMute(const bool enabled)
{
  if(AE)
    AE->SetMute(enabled);

  g_bMute = enabled;
}

bool CAEFactory::IsMuted()
{
  if(AE)
    return AE->IsMuted();

  return g_bMute || (g_fVolume == 0.0f);
}

float CAEFactory::GetVolume()
{
  if(AE)
    return AE->GetVolume();

  return g_fVolume;
}

void CAEFactory::SetVolume(const float volume)
{
  if(AE)
    AE->SetVolume(volume);
  else
    g_fVolume = volume;
}

void CAEFactory::Shutdown()
{
  if(AE)
    AE->Shutdown();
}

IAEStream *CAEFactory::MakeStream(enum AEDataFormat dataFormat, unsigned int sampleRate, 
  unsigned int encodedSampleRate, CAEChannelInfo channelLayout, unsigned int options)
{
  if(AE)
    return AE->MakeStream(dataFormat, sampleRate, encodedSampleRate, channelLayout, options);

  return NULL;
}

IAEStream *CAEFactory::FreeStream(IAEStream *stream)
{
  if(AE)
    return AE->FreeStream(stream);

  return NULL;
}

void CAEFactory::GarbageCollect()
{
  if(AE)
    AE->GarbageCollect();
}

void CAEFactory::RegisterAudioCallback(IAudioCallback* pCallback)
{
  if (AE)
    AE->RegisterAudioCallback(pCallback);
}

void CAEFactory::UnregisterAudioCallback()
{
  if (AE)
    AE->UnregisterAudioCallback();
}

bool CAEFactory::IsSettingVisible(const std::string &value)
{
  if (value.empty() || !AE)
    return false;

  return AE->IsSettingVisible(value);
}

void CAEFactory::KeepConfiguration(unsigned int millis)
{
  if (AE)
    AE->KeepConfiguration(millis);
}

void CAEFactory::DeviceChange()
{
  if (AE)
    AE->DeviceChange();
}
