#include "PlexBusyIndicator.h"
#include "JobManager.h"
#include "dialogs/GUIDialogBusy.h"
#include "guilib/GUIWindowManager.h"
#include "settings/AdvancedSettings.h"
#include "boost/foreach.hpp"
#include "log.h"
#include "Application.h"
#include "PlexJobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
CPlexBusyIndicator::CPlexBusyIndicator()
  : m_blockEvent(true)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexBusyIndicator::blockWaitingForJob(CJob* job, IJobCallback* callback, CFileItemListPtr *result)
{
  CSingleLock lk(m_section);
  m_blockEvent.Reset();
  int id = CJobManager::GetInstance().AddJob(job, this, CJob::PRIORITY_HIGH);

  m_callbackMap[id] = callback;
  m_resultMap[id] = result;

  lk.Leave();

  // wait an initial 300ms if this is a fast operation.
  if (m_blockEvent.WaitMSec(g_advancedSettings.m_videoBusyDialogDelay_ms))
    return true;

  CGUIDialogBusy* busy = NULL;

  if (g_application.IsCurrentThread())
  {
    busy = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
    if (busy)
      busy->Show();
  }

  lk.Enter();

  bool success = true;
  while (m_callbackMap.size() > 0)
  {
    lk.Leave();
#ifdef TARGET_RASPBERRY_PI
    while (!m_blockEvent.WaitMSec(100))
#else
    while (!m_blockEvent.WaitMSec(20))
#endif
    {
      g_windowManager.ProcessRenderLoop(false);
      if (busy && busy->IsCanceled())
      {
        lk.Enter();
        std::pair<int, IJobCallback*> p;
        BOOST_FOREACH(p, m_callbackMap)
          CJobManager::GetInstance().CancelJob(p.first);

        // Let's get out of here.
        m_callbackMap.clear();
        m_resultMap.clear();
        m_blockEvent.Set();
        success = false;
        lk.Leave();
        break;
      }
    }
    lk.Enter();
  }

  if (busy)
    busy->Close();

  return success;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexBusyIndicator::OnJobComplete(unsigned int jobID, bool success, CJob* job)
{
  CSingleLock lk(m_section);

  if (m_callbackMap.find(jobID) != m_callbackMap.end())
  {
    IJobCallback* cb = m_callbackMap[jobID];
    m_callbackMap.erase(jobID);

    if (cb)
    {
      lk.Leave();
      cb->OnJobComplete(jobID, success, job);
      lk.Enter();
    }

    if (m_resultMap[jobID])
    {
      CPlexJob *plexjob = dynamic_cast<CPlexJob*>(job);
      if (plexjob)
      {
        *m_resultMap[jobID] = plexjob->getResult();
      }
    }

    m_resultMap.erase(jobID);

    if (m_callbackMap.size() == 0)
    {
      CLog::Log(LOGDEBUG, "CPlexBusyIndicator::OnJobComplete nothing more blocking, let's leave");
      m_blockEvent.Set();
    }
  }
  else
  {
    CLog::Log(LOGDEBUG, "CPlexBusyIndicator::OnJobComplete ouch, we got %d that we don't have a callback for?", jobID);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPlexBusyIndicator::OnJobProgress(unsigned int jobID, unsigned int progress,
                                       unsigned int total, const CJob* job)
{
  CSingleLock lk(m_section);

  if (m_callbackMap.find(jobID) != m_callbackMap.end())
  {
    IJobCallback* cb = m_callbackMap[jobID];
    lk.Leave();
    if (cb)
      cb->OnJobProgress(jobID, progress, total, job);
  }
  else
  {
    CLog::Log(LOGDEBUG, "CPlexBusyIndicator::OnJobProgress ouch, we got %d that we don't have a callback for?", jobID);
  }
}
