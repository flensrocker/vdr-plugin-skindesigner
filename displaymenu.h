#ifndef __DISPLAYMENU_H
#define __DISPLAYMENU_H

#include <libskindesigner/services.h>

#include "libtemplate/template.h"
#include "views/displaymenurootview.h"

enum eViewState {
    vsInit,
    vsMenuInit,
    vsMenuUpdate,
    vsMenuDetail,
    vsIdle
};

class cSDDisplayMenu : public libskindesigner::ISDDisplayMenu {
private:
    cDisplayMenuRootView *rootView;
    eViewState state;
    bool doOutput;
    string pluginName;
    int pluginMenu;
    ePluginMenuType pluginMenuType;
    mutable cFont *textAreaFont;
protected:
    int Tab(int n);
public:
    cSDDisplayMenu(cTemplate *menuTemplate);
    virtual ~cSDDisplayMenu();
    virtual void Scroll(bool Up, bool Page);
    virtual int MaxItems(void);
    virtual void Clear(void);
    virtual void SetMenuCategory(eMenuCategory MenuCat);
    virtual void SetPluginMenu(string name, int menu, int type, bool init);
    virtual void SetTitle(const char *Title);
    virtual void SetButtons(const char *Red, const char *Green = NULL, const char *Yellow = NULL, const char *Blue = NULL);
    virtual void SetMessage(eMessageType Type, const char *Text);
    virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);
    virtual bool SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch);
    virtual bool SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable);
    virtual bool SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable, bool WithProvider);
    virtual bool SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New);
    virtual bool SetItemPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens, int Index, bool Current, bool Selectable);
    virtual void SetScrollbar(int Total, int Offset);
    virtual void SetEvent(const cEvent *Event);
    virtual void SetRecording(const cRecording *Recording);
    virtual void SetText(const char *Text, bool FixedFont);
    virtual bool SetPluginText(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens);
    virtual void Flush(void);
    virtual void SetTabs(int Tab1, int Tab2 = 0, int Tab3 = 0, int Tab4 = 0, int Tab5 = 0);
    virtual int GetTextAreaWidth(void) const;
    virtual const cFont *GetTextAreaFont(bool FixedFont) const;
};

#endif //__DISPLAYMENU_H
