#include "PlexServerVersion.h"

#include "utils/StringUtils.h"
#include "log.h"

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////
CPlexServerVersion::CPlexServerVersion()
{
  isDev = false;
  major = minor = micro = build = patch = 0;
  gitrev = "";
  isValid = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CPlexServerVersion::CPlexServerVersion(const std::string& versionString)
{
  isDev = false;
  major = minor = micro = build = patch = 0;
  gitrev = "";
  isValid = false;

  parse(versionString);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CPlexServerVersion::parse(const std::string& versionString)
{
  std::vector<std::string> firstSplit = StringUtils::Split(versionString, "-");
  CStdString versionStr;

  if (firstSplit.size() != 2)
  {
    gitrev = "";
    versionStr = versionString;
  }
  else
  {
    gitrev = firstSplit.at(1);
    versionStr = firstSplit.at(0);
  }

  std::vector<std::string> dotsplit = StringUtils::Split(versionStr, ".");

  if (dotsplit.size() > 5 || dotsplit.size() < 4)
  {
    CLog::Log(LOGWARNING, "CPlexServerVersion::parse could not split apart: %s",
              firstSplit.at(0).c_str());
    return false;
  }

  try
  {
    major = boost::lexical_cast<int>(dotsplit.at(0));
    minor = boost::lexical_cast<int>(dotsplit.at(1));
    micro = boost::lexical_cast<int>(dotsplit.at(2));
    if (dotsplit.size() == 5)
    {
      patch = boost::lexical_cast<int>(dotsplit.at(3));
      if (dotsplit.at(4) != "dev")
      {
        build = boost::lexical_cast<int>(dotsplit.at(4));
      }
      else
      {
        isDev = true;
        build = 0;
      }
    }
    else if (dotsplit.size() == 4)
    {
      patch = 0;
      if (dotsplit.at(3) != "dev")
      {
        build = boost::lexical_cast<int>(dotsplit.at(3));
      }
      else
      {
        isDev = true;
        build = 0;
      }
    }
  }
  catch (...)
  {
    CLog::Log(LOGWARNING, "CPlexServerVersion::parse Failed to convert version to integers");
    return false;
  }

  isValid = true;

  return true;
}
