#define __STL_CONFIG_H
#include <vdr/menu.h>
#include <vdr/videodir.h>
#include "displaymenurootview.h"
#include "../config.h"
#include "../libcore/helpers.h"

cDisplayMenuRootView::cDisplayMenuRootView(cTemplateView *rootView) : cView(rootView) {
    cat = mcUndefined;
    viewType = svUndefined;
    subView = NULL;
    subViewAvailable = false;
    pluginName = "";
    pluginMenu = -1;
    pluginMenuType = mtUnknown;
    pluginMenuChanged = false;
    view = NULL;
    listView = NULL;
    detailView = NULL;
    buttonTexts[0] = "";
    buttonTexts[1] = "";
    buttonTexts[2] = "";
    buttonTexts[3] = "";
    defaultBackgroundDrawn = false;
    defaultHeaderDrawn = false;
    defaultButtonsDrawn = false;
    defaultDateTimeDrawn = false;
    defaultMessageDrawn = false;
    DeleteOsdOnExit();
    SetFadeTime(tmplView->GetNumericParameter(ptFadeTime));
}

cDisplayMenuRootView::~cDisplayMenuRootView() {
    if (view)
        delete view;
    if (listView)
        delete listView;
    if (detailView)
        delete detailView;
}

/*******************************************************************
* Public Functions
*******************************************************************/

bool cDisplayMenuRootView::createOsd(void) {
    cRect osdSize = tmplView->GetOsdSize();
    bool ok = CreateOsd(cOsd::OsdLeft() + osdSize.X(),
                        cOsd::OsdTop() + osdSize.Y(),
                        osdSize.Width(),
                        osdSize.Height());
    return ok;
}

/* Categories:
-1 mcUndefined,
0  mcUnknown,
1  mcMain,
2  mcSchedule,
3  mcScheduleNow,
4  mcScheduleNext,
5  mcChannel,
6  mcChannelEdit,
7  mcTimer,
8  mcTimerEdit,
9  mcRecording,
10 mcRecordingInfo,
11 mcRecordingEdit,
12 mcPlugin,
13 mcPluginSetup,
14 mcSetup,
15 mcSetupOsd,
16 mcSetupEpg,
17 mcSetupDvb,
18 mcSetupLnb,
19 mcSetupCam,
20 mcSetupRecord,
21 mcSetupReplay,
22 mcSetupMisc,
23 mcSetupPlugins,
24 mcCommand,
25 mcEvent,
26 mcText,
27 mcFolder,
28 mcCam
*/

void cDisplayMenuRootView::SetMenu(eMenuCategory menuCat, bool menuInit) {
    eSubView newViewType = svUndefined;
    cat = menuCat;
    bool isListView = true;
    if (menuCat != mcPlugin) {
        pluginName = "";
        pluginMenu = -1;
        pluginMenuType = mtUnknown;
        pluginMenuChanged = false;
    }
    switch (menuCat) {
        case mcMain:
            newViewType = svMenuMain;
            break;
       case mcSetup:
            newViewType = svMenuSetup;
            break;
        case mcSchedule:
        case mcScheduleNow:
        case mcScheduleNext:
            newViewType = svMenuSchedules;
            if (view)
                view->SetMenuCat(menuCat);
            break;
        case mcChannel:
            newViewType = svMenuChannels;
            break;
        case mcTimer:
            newViewType = svMenuTimers;
            break;
        case mcRecording:
            newViewType = svMenuRecordings;
            break;
        case mcEvent:
            newViewType = svMenuDetailedEpg;
            isListView = false;
            break;
        case mcRecordingInfo:
            newViewType = svMenuDetailedRecording;
            isListView = false;
            break;
        case mcText:
            newViewType = svMenuDetailedText;
            isListView = false;
            break;
        case mcPlugin:
            newViewType = svMenuPlugin;
            isListView = ( pluginMenuType == mtList ) ? true : false;
            break;
        default:
            newViewType = svMenuDefault;
            break;
    }
    if (newViewType != viewType || pluginMenuChanged) {
        if (newViewType == svMenuPlugin) {
            subView = tmplView->GetPluginView(pluginName, pluginMenu);
            if (!subView) {
                subViewAvailable = false;
                if (isListView)
                    subView = tmplView->GetSubView(svMenuDefault);
                else
                    subView = tmplView->GetSubView(svMenuDetailedText);
            } else {
                subViewAvailable = true;
            }
        } else {
            subView = tmplView->GetSubView(newViewType);
            if (!subView) {
                subViewAvailable = false;
                subView = tmplView->GetSubView(svMenuDefault);
            } else {
                subViewAvailable = true;
            }
        }
        //Cleanup
        if (view) {
            delete view;
            view = NULL;
        }
        if (listView) {
            delete listView;
            listView = NULL;
        }
        if (detailView) {
            delete detailView;
            detailView = NULL;
        }
        //Create new View
        switch (newViewType) {
            case svMenuMain:
                view = new cDisplayMenuMainView(subView, menuInit);
                break;
            case svMenuSchedules:
                if (subViewAvailable)
                    view = new cDisplayMenuSchedulesView(subView, menuCat, menuInit);
                else
                    view = new cDisplayMenuView(subView, menuInit);
                break;
            default:
                view = new cDisplayMenuView(subView, menuInit);
        }
        view->SetMenuCat(cat);
        view->SetButtonTexts(buttonTexts);
        //Cleanup root view
        ClearRootView();
        if (isListView) {
            //Create menu item list
            cTemplateViewList *tmplMenuItems = subView->GetViewList(vlMenuItem);
            if (!tmplMenuItems)
                return;
            listView = new cDisplayMenuListView(tmplMenuItems);
        } else {
            //Create detailed view
            detailView = new cDisplayMenuDetailView(subView);
        }
        viewType = newViewType;
    }
}

void cDisplayMenuRootView::CorrectDefaultMenu(void) {
    if (viewType > svMenuDefault) {
        SetMenu(mcUnknown, true);
    }
}

void cDisplayMenuRootView::SetPluginMenu(string name, int menu, int type) { 
    if (pluginName.compare(name) || menu != pluginMenu || type != pluginMenuType)
        pluginMenuChanged = true;
    else
        pluginMenuChanged = false;

    pluginName = name; 
    pluginMenu = menu; 
    pluginMenuType = (ePluginMenuType)type;
}

void cDisplayMenuRootView::SetTitle(const char *title) {
    menuTitle = title;
    if (view)
        view->SetTitle(title);
}

void cDisplayMenuRootView::SetButtonTexts(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
    if (Red)
        buttonTexts[0] = Red;
    else
        buttonTexts[0] = "";
    if (Green)
        buttonTexts[1] = Green;
    else
        buttonTexts[1] = "";
    if (Yellow)
        buttonTexts[2] = Yellow;
    else
        buttonTexts[2] = "";
    if (Blue)
        buttonTexts[3] = Blue;
    else
        buttonTexts[3] = "";
    if (view)
        view->SetButtonTexts(buttonTexts);
}

void cDisplayMenuRootView::SetTabs(int tab1, int tab2, int tab3, int tab4, int tab5) {
    if (listView) {
        listView->SetTabs(tab1, tab2, tab3, tab4, tab5);
    }  
}

void cDisplayMenuRootView::SetMessage(eMessageType type, const char *text) {
    if (!view)
        return;
    if (!view->DrawMessage(type, text)) {
        defaultMessageDrawn = true;
        DrawMessage(type, text);
    } else {
        defaultMessageDrawn = false;
    }
}

void cDisplayMenuRootView::SetDetailedViewEvent(const cEvent *event) {
    if (!detailView) {
        if (viewType != svMenuDetailedEpg) {
            SetMenu(mcEvent, true);
        } else {
            detailView = new cDisplayMenuDetailView(subView);
        }
    }
    detailView->SetEvent(event);
}

void cDisplayMenuRootView::SetDetailedViewRecording(const cRecording *recording) {
    if (!detailView) {
        if (viewType != svMenuDetailedRecording) {
            SetMenu(mcRecordingInfo, true);
        } else {
            detailView = new cDisplayMenuDetailView(subView);
        }
    }
    detailView->SetRecording(recording);
}

void cDisplayMenuRootView::SetDetailedViewText(const char *text) {
    if (!detailView) {
        if (viewType != svMenuDetailedText) {
            SetMenu(mcText, true);
        } else {
            detailView = new cDisplayMenuDetailView(subView);
        }
    }
    detailView->SetText(text);
}

bool cDisplayMenuRootView::SetDetailedViewPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens) {
    if (!detailView) {
        SetMenu(mcPlugin, true);
        if (!subViewAvailable)
            return false;
    }
    detailView->SetPluginTokens(stringTokens, intTokens, loopTokens);
    return true;
}

void cDisplayMenuRootView::KeyInput(bool up, bool page) {
    if (!detailView)
        return;

    if (up && page) {
        detailView->KeyLeft();
    } else if (!up && page) {
        detailView->KeyRight();
    } else if (up && !page) {
        detailView->KeyUp();
    } else if (!up && !page) {
        detailView->KeyDown();
    }
}

void cDisplayMenuRootView::Clear(void) {
    if (listView) {
        listView->Clear();
    }
    if (detailView) {
        delete detailView;
        detailView = NULL;
    }
}

void cDisplayMenuRootView::ClearRootView(void) {
    if (defaultBackgroundDrawn && view->BackgroundImplemented())
        ClearViewElement(veBackground);
    if (defaultHeaderDrawn)
        ClearViewElement(veHeader);
    if (defaultButtonsDrawn)
        ClearViewElement(veButtons);
    if (defaultDateTimeDrawn)
        ClearViewElement(veDateTime);
    if (defaultMessageDrawn)
        ClearViewElement(veMessage);
}

int cDisplayMenuRootView::GetMaxItems(void) {
    if (listView) {
        return listView->GetMaxItems();
    }
    //wrong menucat
    SetMenu(mcUnknown, true);
    return listView->GetMaxItems();
}

int cDisplayMenuRootView::GetListViewWidth(void) {
    if (listView) {
        return listView->GetListWidth();
    }
    return 0;    
}

int cDisplayMenuRootView::GetTextAreaWidth(void) {
    if (!tmplView)
        return 1900;
    cTemplateView *tmplSubView = tmplView->GetSubView(svMenuDefault);
    if (!tmplSubView)
        return 1900;
    int areaWidth = tmplSubView->GetNumericParameter(ptWidth);
    if (areaWidth > 0)
        return areaWidth;
    return 1900;
}

cFont *cDisplayMenuRootView::GetTextAreaFont(void) {
    if (!tmplView)
        return NULL;
    cTemplateView *tmplSubViewDefault = tmplView->GetSubView(svMenuDefault);
    if (!tmplSubViewDefault)
        return NULL;

    cTemplateViewList *tmplViewList = tmplSubViewDefault->GetViewList(vlMenuItem);
    if (!tmplViewList)
        return NULL;

    return tmplViewList->GetTextAreaFont();
}


void cDisplayMenuRootView::Render(void) {
    if (!view)
        return;
    if (!view->DrawBackground()) {
        if (!defaultBackgroundDrawn) {
            defaultBackgroundDrawn = true;
            DrawBackground();
        }
    } else {
        defaultBackgroundDrawn = false;
    }

    if (!view->DrawHeader()) {
        defaultHeaderDrawn = true;
        DrawHeader();
    } else {
        defaultHeaderDrawn = false;
    }

    if (!view->DrawColorButtons()) {
        defaultButtonsDrawn = true;
        DrawColorButtons();
    } else {
        defaultButtonsDrawn = false;
    }

    if (!view->DrawDateTime()) {
        defaultDateTimeDrawn = true;
        DrawDateTime();
    } else {
        defaultDateTimeDrawn = false;
    }

    view->DrawStaticViewElements();
    view->DrawDynamicViewElements();
}

void cDisplayMenuRootView::RenderMenuItems(void) {
    if (listView)
        listView->Render();
}

void cDisplayMenuRootView::RenderDetailView(void) {
    if (detailView)
        detailView->Render();
}

void cDisplayMenuRootView::RenderMenuScrollBar(int Total, int Offset) {
    if (!listView)
        return;
    view->DrawScrollbar(listView->GetMaxItems(), Total, Offset);
}

bool cDisplayMenuRootView::RenderDynamicElements(void) {
    return view->DrawDynamicViewElements();
}

/*******************************************************************
* Private Functions
*******************************************************************/

void cDisplayMenuRootView::DrawBackground(void) {
    DrawViewElement(veBackground);
}
void cDisplayMenuRootView::DrawHeader(void) {
    if (!ViewElementImplemented(veHeader)) {
        return;
    }

    map < string, string > stringTokens;
    map < string, int > intTokens;

    stringTokens.insert(pair<string,string>("title", menuTitle));
    stringTokens.insert(pair<string,string>("vdrversion", VDRVERSION));

    //check for standard menu entries
    bool hasIcon = false;

    string icon = imgCache->GetIconName(menuTitle, cat);
    if (imgCache->MenuIconExists(icon))
        hasIcon = true;

    stringTokens.insert(pair<string,string>("icon", icon));
    intTokens.insert(pair<string,int>("hasicon", hasIcon));

    //Disc Usage
    string vdrUsageString = *cVideoDiskUsage::String();
    int discUsage = cVideoDiskUsage::UsedPercent();
    bool discAlert = (discUsage > 95) ? true : false;
    string freeTime = *cString::sprintf("%02d:%02d", cVideoDiskUsage::FreeMinutes() / 60, cVideoDiskUsage::FreeMinutes() % 60);
    int freeGB = cVideoDiskUsage::FreeMB() / 1024;

    intTokens.insert(pair<string, int>("usedpercent", discUsage));
    intTokens.insert(pair<string, int>("freepercent", 100-discUsage));
    intTokens.insert(pair<string, int>("discalert", discAlert));
    intTokens.insert(pair<string, int>("freegb", freeGB));
    stringTokens.insert(pair<string,string>("freetime", freeTime));
    stringTokens.insert(pair<string,string>("vdrusagestring", vdrUsageString));

    ClearViewElement(veHeader);
    DrawViewElement(veHeader, &stringTokens, &intTokens);
}

void cDisplayMenuRootView::DrawDateTime(void) {
    if (!ViewElementImplemented(veDateTime)) {
        return;
    }
    
    map < string, string > stringTokens;
    map < string, int > intTokens;
    
    time_t t = time(0);   // get time now
    struct tm * now = localtime(&t);
    
    intTokens.insert(pair<string, int>("year", now->tm_year + 1900));
    intTokens.insert(pair<string, int>("day", now->tm_mday));

    char monthname[20];
    char monthshort[10];
    strftime(monthshort, sizeof(monthshort), "%b", now);
    strftime(monthname, sizeof(monthname), "%B", now);

    stringTokens.insert(pair<string,string>("monthname", monthname));
    stringTokens.insert(pair<string,string>("monthnameshort", monthshort));
    stringTokens.insert(pair<string,string>("month", *cString::sprintf("%02d", now->tm_mon + 1)));
    stringTokens.insert(pair<string,string>("dayleadingzero", *cString::sprintf("%02d", now->tm_mday)));
    stringTokens.insert(pair<string,string>("dayname", *WeekDayNameFull(now->tm_wday)));
    stringTokens.insert(pair<string,string>("daynameshort", *WeekDayName(now->tm_wday)));
    stringTokens.insert(pair<string,string>("time", *TimeString(t)));

    ClearViewElement(veDateTime);
    DrawViewElement(veDateTime, &stringTokens, &intTokens);

}


void cDisplayMenuRootView::DrawColorButtons(void) {
    if (!ViewElementImplemented(veButtons)) {
        return;
    }

    map < string, string > stringTokens;
    map < string, int > intTokens;

    stringTokens.insert(pair<string,string>("red", buttonTexts[0]));
    stringTokens.insert(pair<string,string>("green", buttonTexts[1]));
    stringTokens.insert(pair<string,string>("yellow", buttonTexts[2]));
    stringTokens.insert(pair<string,string>("blue", buttonTexts[3]));

    int colorKeys[4] = { Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3 };

    for (int button = 1; button < 5; button++) {
        string red = *cString::sprintf("red%d", button);
        string green = *cString::sprintf("green%d", button);
        string yellow = *cString::sprintf("yellow%d", button);
        string blue = *cString::sprintf("blue%d", button);
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button-1]) {
            case 0:
                isRed = true;
                break;
            case 1:
                isGreen = true;
                break;
            case 2:
                isYellow = true;
                break;
            case 3:
                isBlue = true;
                break;
            default:
                break;
        }
        intTokens.insert(pair<string, int>(red, isRed));
        intTokens.insert(pair<string, int>(green, isGreen));
        intTokens.insert(pair<string, int>(yellow, isYellow));
        intTokens.insert(pair<string, int>(blue, isBlue));
    }

    ClearViewElement(veButtons);
    DrawViewElement(veButtons, &stringTokens, &intTokens);
}

void cDisplayMenuRootView::DrawMessage(eMessageType type, const char *text) {
    if (!text) {
        ClearViewElement(veMessage);
        return;
    }

    map < string, string > stringTokens;
    map < string, int > intTokens;

    intTokens.insert(pair<string, int>("status",  (type == mtStatus)  ? true : false));
    intTokens.insert(pair<string, int>("info",    (type == mtInfo)    ? true : false));
    intTokens.insert(pair<string, int>("warning", (type == mtWarning) ? true : false));
    intTokens.insert(pair<string, int>("error",   (type == mtError)   ? true : false));
    stringTokens.insert(pair<string,string>("text", text));

    ClearViewElement(veMessage);
    DrawViewElement(veMessage, &stringTokens, &intTokens);
}

void cDisplayMenuRootView::Action(void) {
    SetInitFinished();
    Render();
    view->Start();
    FadeIn();
    DoFlush();
}
