
#ifndef RASPBERRYPI_PROC_READER
#define RASPBERRYPI_PROC_READER

#include <string>
#include <fstream>
//#include "LocalizeStrings.h"
#include "utils/log.h"
#include "StringUtils.h"

CStdString readProcCPUInfoValue(CStdString keyname);


#endif //RASPBERRYPI_PROC_READER
