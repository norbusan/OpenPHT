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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <algorithm>
#include "Monitor.h"
#include <math.h>

namespace XBMCAddon
{
  namespace xbmc
  {
    Monitor::Monitor() : AddonCallback("Monitor"), abortEvent(true)
    {
      TRACE;
      if (languageHook)
      {
        Id = languageHook->GetAddonId();
        languageHook->RegisterMonitorCallback(this);
      }
    }

    void Monitor::OnAbortRequested()
    {
      TRACE;
      abortEvent.Set();
      invokeCallback(new CallbackFunction<Monitor>(this,&Monitor::onAbortRequested));
    }

    bool Monitor::waitForAbort(double timeout)
    {
      TRACE;
      int timeoutMS = ceil(timeout * 1000);
      XbmcThreads::EndTime endTime(timeoutMS > 0 ? timeoutMS : XbmcThreads::EndTime::InfiniteValue);
      while (!endTime.IsTimePast())
      {
        {
          DelayedCallGuard dg(languageHook);
          unsigned int t = std::min(endTime.MillisLeft(), 100u);
          if (abortEvent.WaitMSec(t))
            return true;
        }
        if (languageHook)
          languageHook->MakePendingCalls();
      }
      return false;
    }

    bool Monitor::abortRequested()
    {
      TRACE;
      return abortEvent.Signaled();
    }

    Monitor::~Monitor()
    {
      TRACE;
      deallocating();
      DelayedCallGuard dg(languageHook);
      // we're shutting down so unregister me.
      if (languageHook)
      {
        DelayedCallGuard dc;
        languageHook->UnregisterMonitorCallback(this);
      }
    }
  }
}

