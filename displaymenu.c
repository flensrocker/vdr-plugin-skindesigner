#include "displaymenu.h"
#include "libcore/helpers.h"

cSDDisplayMenu::cSDDisplayMenu(cTemplate *menuTemplate) {
    textAreaFont = NULL; 
    doOutput = true;
    state = vsInit;
    pluginMenu = -1;
    pluginName = "";
    pluginMenuType = mtUnknown;
    if (!menuTemplate) {
        doOutput = false;
        esyslog("skindesigner: displayMenu no valid template - aborting");
        return;
    } 
    rootView = new cDisplayMenuRootView(menuTemplate->GetRootView());
    if (!rootView->createOsd()) {
        doOutput = false;
        return;
    }
}

cSDDisplayMenu::~cSDDisplayMenu() {
    if (!doOutput)
        return;
    delete rootView;
    if (textAreaFont)
        delete textAreaFont;
}

void cSDDisplayMenu::Scroll(bool Up, bool Page) {
    if (!doOutput)
        return;
    rootView->KeyInput(Up, Page);
}

int cSDDisplayMenu::MaxItems(void) {
    if (!doOutput)
        return 0;
    int maxItems = rootView->GetMaxItems();
    return maxItems;
}

void cSDDisplayMenu::Clear(void) {
    if (!doOutput)
        return;
    rootView->Clear();
}

void cSDDisplayMenu::SetMenuCategory(eMenuCategory MenuCat) {
    if (!doOutput)
        return;
    rootView->SetMenu(MenuCat, (state == vsInit) ? true : false);
    cSkinDisplayMenu::SetMenuCategory(MenuCat);
    if (state != vsInit)
        state = vsMenuInit;
}

void cSDDisplayMenu::SetPluginMenu(string name, int menu, int type, bool init) {
    pluginName = name;
    pluginMenu = menu;
    pluginMenuType = (ePluginMenuType)type;
    rootView->SetPluginMenu(pluginName, pluginMenu, pluginMenuType);
    if (!init) {
        rootView->SetMenu(mcPlugin, false);        
    }
}

void cSDDisplayMenu::SetTitle(const char *Title) {
    if (!doOutput)
        return;
    rootView->SetTitle(Title);
}

void cSDDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
    if (!doOutput)
        return;
    rootView->SetButtonTexts(Red, Green, Yellow, Blue);
    if (state != vsInit && MenuCategory() != mcMain)
        state = vsMenuInit;
}

void cSDDisplayMenu::SetMessage(eMessageType Type, const char *Text) {
    if (!doOutput)
        return;
    rootView->SetMessage(Type, Text);
    rootView->DoFlush();
}

bool cSDDisplayMenu::SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch) {
    if (!doOutput)
        return true;
    if (!rootView->SubViewAvailable())
        return false;
    if (config.blockFlush)
        rootView->LockFlush();
    const cChannel *channel = Channel;
    if (!channel) {
        channel = rootView->GetChannel();
    } 
    if (!channel && Event) {
        channel = Channels.GetByChannelID(Event->ChannelID());
    }
    rootView->SetChannel(channel);
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list)
        return false;
    list->AddSchedulesMenuItem(Index, Event, channel, TimerMatch, MenuCategory(), Current, Selectable);
    if (state == vsIdle)
        state = vsMenuUpdate;
    return true;
}

bool cSDDisplayMenu::SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable) {
    if (!doOutput)
        return true;
    if (!rootView->SubViewAvailable())
        return false;
    if (config.blockFlush)
        rootView->LockFlush();
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list)
        return false;
    list->AddTimersMenuItem(Index, Timer, Current, Selectable);
    if (state == vsIdle)
        state = vsMenuUpdate;
    return true;
}

bool cSDDisplayMenu::SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable, bool WithProvider) {
    if (!doOutput)
        return true;
    if (!rootView->SubViewAvailable())
        return false;
    if (config.blockFlush)
        rootView->LockFlush();
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list)
        return false;
    list->AddChannelsMenuItem(Index, Channel, WithProvider, Current, Selectable);
    if (state == vsIdle)
        state = vsMenuUpdate;
    return true;
}

bool cSDDisplayMenu::SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New) {
    if (!doOutput)
        return true;
    if (!rootView->SubViewAvailable())
        return false;
    if (config.blockFlush)
        rootView->LockFlush();
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list)
        return false;
    list->AddRecordingMenuItem(Index, Recording, Level, Total, New, Current, Selectable);
    if (state == vsIdle)
        state = vsMenuUpdate;
    return true;
}

bool cSDDisplayMenu::SetItemPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens, int Index, bool Current, bool Selectable) {
    if (!doOutput)
        return true;
    if (!rootView->SubViewAvailable())
        return false;
    if (config.blockFlush)
        rootView->LockFlush();
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list)
        return false;
    list->AddPluginMenuItem(stringTokens, intTokens, loopTokens, Index, Current, Selectable);
    if (state == vsIdle)
        state = vsMenuUpdate;
    return true;
}

void cSDDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable) {
    if (!doOutput)
        return;
    cDisplayMenuListView *list = rootView->GetListView();
    if (!list) {
        return;
    }
    if (config.blockFlush)
        rootView->LockFlush();
    eMenuCategory cat = MenuCategory();
    if (cat == mcMain && rootView->SubViewAvailable()) {
        list->AddMainMenuItem(Index, Text, Current, Selectable);
    } else if (cat == mcSetup && rootView->SubViewAvailable()) {
        list->AddSetupMenuItem(Index, Text, Current, Selectable);        
    } else {
        rootView->CorrectDefaultMenu();
        string *tabTexts = new string[MaxTabs];
        for (int i=0; i<MaxTabs; i++) {
            const char *s = GetTabbedText(Text, i);
            if (s) {
                tabTexts[i] = s;
            } else {
                tabTexts[i] = "";
            }
        }
        list->AddDefaultMenuItem(Index, tabTexts, Current, Selectable);
        SetEditableWidth( rootView->GetListViewWidth() / 2);
    }
    if (state == vsIdle)
        state = vsMenuUpdate;
}

void cSDDisplayMenu::SetTabs(int Tab1, int Tab2, int Tab3, int Tab4, int Tab5) {
    if (!doOutput)
        return;
    rootView->SetTabs(Tab1, Tab2, Tab3, Tab4, Tab5);
}

int cSDDisplayMenu::GetTextAreaWidth(void) const {
    int areaWidth = rootView->GetTextAreaWidth();
    return areaWidth;
}

const cFont *cSDDisplayMenu::GetTextAreaFont(bool FixedFont) const {
    if (textAreaFont)
        return textAreaFont;
    textAreaFont = rootView->GetTextAreaFont();
    return textAreaFont;
}

void cSDDisplayMenu::SetScrollbar(int Total, int Offset) {
    if (!doOutput)
        return;
    rootView->RenderMenuScrollBar(Total, Offset);
}

void cSDDisplayMenu::SetEvent(const cEvent *Event) {
    if (!doOutput)
        return;
    rootView->SetDetailedViewEvent(Event);
    state = vsMenuDetail;
}

void cSDDisplayMenu::SetRecording(const cRecording *Recording) {
    if (!doOutput)
        return;
    rootView->SetDetailedViewRecording(Recording);
    state = vsMenuDetail;
}

void cSDDisplayMenu::SetText(const char *Text, bool FixedFont) {
    if (!doOutput)
        return;
    rootView->SetDetailedViewText(Text);
    state = vsMenuDetail;
}

bool cSDDisplayMenu::SetPluginText(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens) {
    if (!doOutput)
        return true;
    bool tmplOk = rootView->SetDetailedViewPlugin(stringTokens, intTokens, loopTokens);
    state = vsMenuDetail;
    return tmplOk;
}

void cSDDisplayMenu::Flush(void) {
    if (!doOutput)
        return;
    bool doFlush = false;
    if (state == vsInit) {
        rootView->Start();
        rootView->RenderMenuItems();
        doFlush = true;
    } else if (state == vsMenuInit) {
        rootView->Render();
        rootView->RenderMenuItems();
        doFlush = true;
    } else if (state == vsMenuUpdate) {
        rootView->RenderMenuItems();
        doFlush = true;
    } else if (state == vsMenuDetail) {
        rootView->OpenFlush();
        rootView->Render();
        rootView->DoFlush();
        rootView->RenderDetailView();
        rootView->DoFlush();
    } else {
        if (rootView->RenderDynamicElements()) {
            doFlush = true;
        }
    }
    if (doFlush) {
        if (config.blockFlush)
            rootView->OpenFlush();
        rootView->DoFlush();
    }
    state = vsIdle;
}
