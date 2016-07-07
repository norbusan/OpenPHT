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

#pragma once

#include "AddonCallback.h"
#include "AddonString.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    /**
     * Monitor class.
     * 
     * Monitor() -- Creates a new Monitor to notify addon about changes.
     */
    class Monitor : public AddonCallback
    {
      String Id;
      CEvent abortEvent;
    public:
      Monitor();

#ifndef SWIG
      inline void    OnSettingsChanged() { TRACE; invokeCallback(new CallbackFunction<Monitor>(this,&Monitor::onSettingsChanged)); }
      inline void    OnScreensaverActivated() { TRACE; invokeCallback(new CallbackFunction<Monitor>(this,&Monitor::onScreensaverActivated)); }
      inline void    OnScreensaverDeactivated() { TRACE; invokeCallback(new CallbackFunction<Monitor>(this,&Monitor::onScreensaverDeactivated)); }
      inline void    OnDatabaseUpdated(const String &database) { TRACE; invokeCallback(new CallbackFunction<Monitor,const String>(this,&Monitor::onDatabaseUpdated,database)); }
      inline void    OnDatabaseScanStarted(const String &database) { TRACE; invokeCallback(new CallbackFunction<Monitor,const String>(this,&Monitor::onDatabaseScanStarted,database)); }

      inline const String& GetId() { return Id; }

      void OnAbortRequested();
#endif

      /**
       * onSettingsChanged() -- onSettingsChanged method.
       * 
       * Will be called when addon settings are changed
       */
      virtual void    onSettingsChanged() { TRACE; }

      /**
       * onScreensaverActivated() -- onScreensaverActivated method.
       * 
       * Will be called when screensaver kicks in
       */
      virtual void    onScreensaverActivated() { TRACE; }

      /**
       * onScreensaverDeactivated() -- onScreensaverDeactivated method.
       * 
       * Will be called when screensaver goes off
       */
      virtual void    onScreensaverDeactivated() { TRACE; }

      /**
       * onDatabaseUpdated(database) -- onDatabaseUpdated method.
       * 
       * database - video/music as stri
       * 
       * Will be called when database gets updated and return video or music to indicate which DB has been changed
       */
      virtual void    onDatabaseUpdated(const String database) { TRACE; }

      /**
       * onDatabaseScanStarted(database) -- onDatabaseScanStarted method.
       *
       * database - video/music as string
       *
       * Will be called when database update starts and return video or music to indicate which DB is being updated
       */
      virtual void    onDatabaseScanStarted(const String database) { TRACE; }
      
      /**
       * onAbortRequested() -- Deprecated, use waitForAbort() to be notified about this event.\n
       */
      virtual void    onAbortRequested() { TRACE; }


      /**
       * waitForAbort([timeout]) -- Block until abort is requested, or until timeout occurs. If an
       *                            abort requested have already been made, return immediately.
       *
       * Returns True when abort have been requested, False if a timeout is given and the operation times out.
       *
       * timeout : [opt] float - timeout in seconds. Default: no timeout.\n
       */
      bool waitForAbort(double timeout = -1);

      /**
       * abortRequested() -- Returns True if abort has been requested.
       */
      bool abortRequested();

      virtual ~Monitor();
    };
  }
};
