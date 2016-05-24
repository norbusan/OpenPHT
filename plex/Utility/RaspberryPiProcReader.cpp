
#include "RaspberryPiProcReader.h"

CStdString readProcCPUInfoValue(CStdString keyname)
{
  //std::string foo = "Serial";
  int index;
  int keycheck;
  CStdString result = "";
  std::ifstream input( "/proc/cpuinfo" );
  for( CStdString line; getline( input, line ); )
  {
    index = line.find(":");
    keycheck = line.find(keyname);
    if (index>=0 && keycheck == 0)
    {
      result = line.substr(index+2,line.length()-1);
      CLog::Log(LOGDEBUG,"Read value %s for %s", result.c_str(), keyname.c_str());
    }
  }
  return result;
}


