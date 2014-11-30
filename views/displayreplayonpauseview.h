#ifndef __DISPLAYREPLAYONPAUSEVIEW_H
#define __DISPLAYREPLAYONPAUSEVIEW_H

#include "../libtemplate/template.h"
#include "view.h"
#include "viewhelpers.h"

class cDisplayReplayOnPauseView : public cView, public cViewHelpers {
private:
    int delay;
    virtual void Action(void);
public:
    cDisplayReplayOnPauseView(cTemplateViewElement *tmplViewElement);
    virtual ~cDisplayReplayOnPauseView();
    void Render(void);
    void Flush(void) { DoFlush(); };
};
#endif //__DISPLAYREPLAYONPAUSEVIEW_H
