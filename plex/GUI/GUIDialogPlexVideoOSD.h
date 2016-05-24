#ifndef GUIDIALOGPLEXVIDEOOSD_H
#define GUIDIALOGPLEXVIDEOOSD_H

#include "video/dialogs/GUIDialogVideoOSD.h"

class CGUIDialogPlexVideoOSD : public CGUIDialogVideoOSD
{
public:
  CGUIDialogPlexVideoOSD();
  virtual bool OnAction(const CAction &action);
  virtual bool OnMessage(CGUIMessage& message);

  bool IsOpenedFromPause() const { return m_openedFromPause; }
  void SetOpenedFromPause(bool onOff) { m_openedFromPause = onOff; }

private:
  bool m_openedFromPause;
};

#endif // GUIDIALOGPLEXVIDEOOSD_H
