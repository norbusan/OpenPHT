#pragma once

#include <string>
#include "threads/Thread.h"
#include "GUIInfoManager.h"

#ifdef TARGET_RASPBERRY_PI
#include "RaspberryPiProcReader.h"
#include "dialogs/GUIDialogOK.h"
#endif

class CrashSubmitter : public CThread
{
  public:
    CrashSubmitter() : CThread("CrashSubmitter") { Create(true); }

    void Process();

    static void UploadCrashReports();
    static bool UploadFile(const CStdString& path);

  private:
    static void Upload();
    static CStdString GetDumpData(const CStdString &path);
    static std::string ExtractVersionFromCrashDump(const CStdString& path);
};
