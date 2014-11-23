#ifndef __DISPLAYMENUVIEW_H
#define __DISPLAYMENUVIEW_H

#include "../libtemplate/template.h"
#include "viewhelpers.h"
#include "displaymenulistview.h"

class cDisplayMenuView : public cView {
protected:
    eMenuCategory cat;
    string menuTitle;
    string *buttonTexts;
    virtual void Action(void);
public:
    cDisplayMenuView(cTemplateView *tmplView, bool menuInit);
    virtual ~cDisplayMenuView();
    void SetMenuCat(eMenuCategory newCat) { cat = newCat; };
    void SetTitle(const char *title) {menuTitle = title; };
    virtual void SetChannel(const cChannel *channel) {};
    virtual const cChannel *GetChannel(void) { return NULL; };
    void SetButtonTexts(string *buttonTexts) { this->buttonTexts = buttonTexts; };
    bool DrawBackground(void);
    virtual bool DrawHeader(void);
    bool DrawDateTime(void);
    bool DrawColorButtons(void);
    bool DrawMessage(eMessageType type, const char *text);
    void DrawScrollbar(int numMax, int numDisplayed, int offset);
    virtual void DrawStaticViewElements(void) {};
    virtual bool DrawDynamicViewElements(void) { return false; };
    bool BackgroundImplemented(void);
};

class cDisplayMenuMainView : public cDisplayMenuView, public cViewHelpers {
private:
    bool initial;
    double lastSystemLoad;
    void DrawTimers(void);
    void DrawDiscUsage(void);
    bool DrawLoad(void);
    bool DrawDevices(void);
public:
    cDisplayMenuMainView(cTemplateView *tmplView, bool menuInit);
    virtual ~cDisplayMenuMainView();
    void DrawStaticViewElements(void);
    bool DrawDynamicViewElements(void);
};

class cDisplayMenuSchedulesView : public cDisplayMenuView {
private:
    const cChannel *channel;
public:
    cDisplayMenuSchedulesView(cTemplateView *tmplView, eMenuCategory menuCat, bool menuInit);
    virtual ~cDisplayMenuSchedulesView();
    void SetChannel(const cChannel *channel) { if (!this->channel) this->channel = channel; };
    const cChannel *GetChannel(void) { return channel; };
    bool DrawHeader(void);
};

#endif //__DISPLAYMENUVIEW_H
