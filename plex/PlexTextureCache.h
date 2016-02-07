#ifndef PLEXTEXTURECACHE_H
#define PLEXTEXTURECACHE_H

#include "TextureCache.h"

class CPlexTextureCache : public CTextureCache
{
public:
  CPlexTextureCache() {}
  ~CPlexTextureCache() {}

  virtual void Initialize() {}
  virtual void Deinitialize();
  virtual bool GetCachedTexture(const CStdString &url, CTextureDetails &details);
  virtual bool AddCachedTexture(const CStdString &image, const CTextureDetails &details);
  virtual void IncrementUseCount(const CTextureDetails &details);
  virtual bool SetCachedTextureValid(const CStdString &url, bool updateable);
  virtual bool ClearCachedTexture(const CStdString &url, CStdString &cacheFile);
};

#endif // PLEXTEXTURECACHE_H
