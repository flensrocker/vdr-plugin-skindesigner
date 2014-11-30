#include "displaymenuitemview.h"
#include "../services/scraper2vdr.h"
#include "../config.h"
#include "../libcore/helpers.h"
#include "../libcore/recfolderinfo.h"
#include <sstream>
#include <algorithm>

/*************************************************************
* cDisplayMenuItemView
*************************************************************/

cDisplayMenuItemView::cDisplayMenuItemView(cTemplateViewList *tmplList, bool current, bool selectable) : cViewListItem(tmplList->GetListElement()) {
    this->tmplList = tmplList;
    this->current = current;
    this->selectable = selectable;
    itemInit = true;
    dirty = true;
    num = 0;
    currentView = NULL;
}

cDisplayMenuItemView::~cDisplayMenuItemView() {
    if (currentView)
        delete currentView;
    CancelSave();
}

void cDisplayMenuItemView::SetCurrent(bool cur) { 
    current = cur; 
    intTokens.erase("current");
    intTokens.insert(pair<string,int>("current", current));
    dirty = true; 
};


void cDisplayMenuItemView::ArrangeContainer(void) {
    if (pos > -1)
        return;
    SetInitFinished();
    pos = num;
    numTotal = tmplList->GetNumericParameter(ptNumElements);
    container.SetX(tmplList->GetNumericParameter(ptX));
    container.SetY(tmplList->GetNumericParameter(ptY));
    container.SetWidth(tmplList->GetNumericParameter(ptWidth));
    container.SetHeight(tmplList->GetNumericParameter(ptHeight));
    align = tmplList->GetNumericParameter(ptAlign);
    listOrientation = tmplList->GetNumericParameter(ptOrientation);
}

void cDisplayMenuItemView::Clear(void) {
    ClearListItem();
    if (currentView) {
        delete currentView;
        currentView = NULL;
    }
}

void cDisplayMenuItemView::PrepareScrolling(void) {
    currentlyScrolling = true;
    ClearListItem();
    DrawListItem(&stringTokens, &intTokens);
}

void cDisplayMenuItemView::EndScrolling(void) {
    const cPoint p(0,0);
    SetDrawPortPoint(scrollingPix, p);
    currentlyScrolling = false;
    ClearListItem();
    DrawListItem(&stringTokens, &intTokens);
}

void cDisplayMenuItemView::Action(void) {
    if (scrolling) {
        DoSleep(scrollDelay);
        PrepareScrolling();
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

void cDisplayMenuItemView::Stop(void) {
    if (Running()) {
        CancelSave();
        EndScrolling();
    }
}


void cDisplayMenuItemView::Debug(void) {
    esyslog("skindesigner: current %d, selectable %d", current, selectable);
    esyslog("skindesigner: pos %d, numTotal %d", pos, numTotal);
    esyslog("skindesigner: container x = %d, y = %d, width = %d, height = %d", container.X(), container.Y(), container.Width(), container.Height());
}

/*************************************************************
* cDisplayMenuItemDefaultView
*************************************************************/

cDisplayMenuItemDefaultView::cDisplayMenuItemDefaultView(cTemplateViewList *tmplList, string *tabTexts, int *tabs, int *tabWidths, bool current, bool selectable) 
                         : cDisplayMenuItemView(tmplList, current, selectable) {
    this->tabTexts = tabTexts;
    this->tabs = tabs;
    this->tabWidths = tabWidths;
    maxTabs = cSkinDisplayMenu::MaxTabs;
}

cDisplayMenuItemDefaultView::~cDisplayMenuItemDefaultView() {
    delete[] tabTexts;
}

void cDisplayMenuItemDefaultView::SetTabTexts(string *tabTexts) {
    delete[] this->tabTexts;
    this->tabTexts = tabTexts; 
}
    
void cDisplayMenuItemDefaultView::SetTokens(void) {
    stringTokens.clear();
    intTokens.clear();
    for (int i=0; i<maxTabs; i++) {
        stringstream name;
        name << "column" << (i+1);
        stringstream nameUsed;
        nameUsed << name.str() << "set";
        stringstream nameX;
        nameX << name.str() << "x";
        stringstream nameWidth;
        nameWidth << name.str() << "width";
        stringTokens.insert(pair<string,string>(name.str(), tabTexts[i]));
        if (i>0)
            intTokens.insert(pair<string,int>(nameUsed.str(), (tabTexts[i].size() > 0) ? 1 : 0 ));
        intTokens.insert(pair<string,int>(nameX.str(), tabs[i]));
        intTokens.insert(pair<string,int>(nameWidth.str(), tabWidths[i]));
    }
    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("separator", !selectable));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
}

void cDisplayMenuItemDefaultView::Prepare(void) {
    ArrangeContainer();
}


void cDisplayMenuItemDefaultView::Render(void) {
    DrawListItem(&stringTokens, &intTokens);
    dirty = false;
}

void cDisplayMenuItemDefaultView::Debug(void) {
    esyslog("skindesigner: Default Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    for (int i=0; i<maxTabs; i++) {
        esyslog("skindesigner: tab %d: text: %s, x: %d", i, tabTexts[i].c_str(), tabs[i]);
    }
}

/*************************************************************
* Private Functions
*************************************************************/


/*************************************************************
* cDisplayMenuItemMainView
*************************************************************/

cDisplayMenuItemMainView::cDisplayMenuItemMainView(cTemplateViewList *tmplList, string itemText, bool current, bool selectable) 
                         : cDisplayMenuItemView(tmplList, current, selectable) {
    text = itemText;
    number = "";
    label = "";
    icon = "";
}

cDisplayMenuItemMainView::~cDisplayMenuItemMainView() {
}

void cDisplayMenuItemMainView::SetTokens(void) {
    if (!itemInit)
        return;
    itemInit = false;
    stringTokens.insert(pair<string,string>("number", number));
    stringTokens.insert(pair<string,string>("label", label));
    stringTokens.insert(pair<string,string>("icon", icon));

    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("separator", !selectable));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
}

void cDisplayMenuItemMainView::Prepare(void) {
    ArrangeContainer();
    SplitMenuText();
    icon = imgCache->GetIconName(label);
}


void cDisplayMenuItemMainView::Render(void) {

    cRect pos = DrawListItem(&stringTokens, &intTokens);

    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentMainView(tmplCurrent, number, label, icon);
            currentView->SetPosMenuItem(pos);
            currentView->Start();
        }
    }

    dirty = false;
}

void cDisplayMenuItemMainView::Debug(void) {
    esyslog("skindesigner: Main Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: item Text: %s", text.c_str());
    esyslog("skindesigner: item Number: %s, Label: %s", number.c_str(), label.c_str());
    esyslog("skindesigner: Icon: %s", icon.c_str());
}

/*************************************************************
* Private Functions
*************************************************************/

void cDisplayMenuItemMainView::SplitMenuText(void) {
    string textPlain = skipspace(text.c_str());
    bool found = false;
    bool doBreak = false;
    size_t i = 0;
    for (; i < textPlain.length(); i++) {
        char s = textPlain.at(i);
        if (i==0) {
            //if text directly starts with nonnumeric, break
            if (!(s >= '0' && s <= '9')) {
                break;
            }
        }
        if (found) {
            if (!(s >= '0' && s <= '9')) {
                doBreak = true;
            }
        }
        if (s >= '0' && s <= '9') {
            found = true;
        }
        if (doBreak)
            break;
        if (i>4)
            break;
    }
    if (found) {
        number = skipspace(textPlain.substr(0,i).c_str());
        label = skipspace(textPlain.substr(i).c_str());
    } else {
        number = "";
        label = textPlain.c_str();
    }
}

/*************************************************************
* cDisplayMenuItemSchedulesView
*************************************************************/

cDisplayMenuItemSchedulesView::cDisplayMenuItemSchedulesView(cTemplateViewList *tmplList, const cEvent *event, 
                                                             const cChannel *channel, eTimerMatch timerMatch, 
                                                             eMenuCategory cat, bool current, bool selectable) 
                                                            : cDisplayMenuItemView(tmplList, current, selectable) {
    this->event = event;
    this->channel = channel;
    this->timerMatch = timerMatch;
    this->cat = cat;
}

cDisplayMenuItemSchedulesView::~cDisplayMenuItemSchedulesView() {
}

void cDisplayMenuItemSchedulesView::SetTokens(void) {
    if (!itemInit)
        return;
    itemInit = false;
    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("separator", !selectable));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
    intTokens.insert(pair<string,int>("whatson", (cat == mcSchedule) ? true: false));
    intTokens.insert(pair<string,int>("whatsonnow", (cat == mcScheduleNow) ? true: false));
    intTokens.insert(pair<string,int>("whatsonnext", (cat == mcScheduleNext) ? true: false));
    if (timerMatch == tmFull) {
        intTokens.insert(pair<string,int>("timerpartitial", false));
        intTokens.insert(pair<string,int>("timerfull", true));
    } else if (timerMatch == tmPartial) {
        intTokens.insert(pair<string,int>("timerpartitial", true));
        intTokens.insert(pair<string,int>("timerfull", false));
    } else {
        intTokens.insert(pair<string,int>("timerpartitial", false));
        intTokens.insert(pair<string,int>("timerfull", false));
    }

    if (event) {
        if (selectable) {
            stringTokens.insert(pair<string,string>("title", event->Title() ? event->Title() : ""));
            stringTokens.insert(pair<string,string>("shorttext", event->ShortText() ? event->ShortText() : ""));
            stringTokens.insert(pair<string,string>("start", *(event->GetTimeString())));
            stringTokens.insert(pair<string,string>("stop", *(event->GetEndTimeString())));
            time_t startTime = event->StartTime();
            stringTokens.insert(pair<string,string>("day", *WeekDayName(startTime)));
            stringTokens.insert(pair<string,string>("date", *ShortDateString(startTime)));
            struct tm * sStartTime = localtime(&startTime);
            intTokens.insert(pair<string, int>("year", sStartTime->tm_year + 1900));
            intTokens.insert(pair<string, int>("daynumeric", sStartTime->tm_mday));
            intTokens.insert(pair<string, int>("month", sStartTime->tm_mon+1));

            bool isRunning = false;
            time_t now = time(NULL);
            if ((now >= event->StartTime()) && (now <= event->EndTime()))
                isRunning = true;
            intTokens.insert(pair<string,int>("running", isRunning));
            if (isRunning) {
                intTokens.insert(pair<string,int>("elapsed", (now - event->StartTime())/60));
            } else {
                intTokens.insert(pair<string,int>("elapsed", 0));
            }
            intTokens.insert(pair<string,int>("duration", event->Duration() / 60));
            intTokens.insert(pair<string,int>("durationhours", event->Duration() / 3600));
            stringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", (event->Duration() / 60)%60)));
            intTokens.insert(pair<string,int>("hasVPS", (bool)event->Vps()));
        } else {
            stringTokens.insert(pair<string,string>("title", event->Title() ? ParseSeparator(event->Title()) : ""));
        }
    }
    if (channel) {
        stringTokens.insert(pair<string,string>("channelname", channel->Name() ? channel->Name() : ""));
        string channelID = *(channel->GetChannelID().ToString());
        stringTokens.insert(pair<string,string>("channelid", channelID));
        intTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));
        if (!event && !selectable) {
            stringTokens.insert(pair<string,string>("title", channel->Name() ? ParseSeparator(channel->Name()) : ""));
        }
    } else {
        stringTokens.insert(pair<string,string>("channelname", ""));
        stringTokens.insert(pair<string,string>("channelid", ""));
        intTokens.insert(pair<string, int>("channellogoexists", 0));
    }
}


void cDisplayMenuItemSchedulesView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemSchedulesView::Render(void) {

    DrawListItem(&stringTokens, &intTokens);
    
    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentSchedulesView(tmplCurrent, event, channel, timerMatch, cat);
            currentView->Start();
        }
    }
    
    dirty = false;
}

void cDisplayMenuItemSchedulesView::Debug(void) {
    esyslog("skindesigner: Schedules Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: Event: %s", event ? event->Title() : "Event is NULL");
}

/*************************************************************
* Private Functions
*************************************************************/

string cDisplayMenuItemSchedulesView::ParseSeparator(string sep) {
    string separator = sep;
    try {
        if (separator.find_first_not_of("-") > 0)
            separator.erase(0, separator.find_first_not_of("-")+1);
        if (separator.find_last_not_of("-") != string::npos)
            separator.erase(separator.find_last_not_of("-")+1);
    } catch (...) {}
    return separator;
}

/*************************************************************
* cDisplayMenuItemChannelsView
*************************************************************/

cDisplayMenuItemChannelsView::cDisplayMenuItemChannelsView(cTemplateViewList *tmplList, const cChannel *channel, 
                                                           bool withProvider, bool current, bool selectable) 
                                                           : cDisplayMenuItemView(tmplList, current, selectable) {
    this->channel = channel;
    this->withProvider = withProvider;
}

cDisplayMenuItemChannelsView::~cDisplayMenuItemChannelsView() {
}

void cDisplayMenuItemChannelsView::SetTokens(void) {
    if (!itemInit)
        return;
    itemInit = false;
    if (!channel)
        return;
    //general channel information
    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("separator", !selectable));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
    intTokens.insert(pair<string,int>("number", channel->Number()));
    intTokens.insert(pair<string,int>("transponder", channel->Transponder()));
    intTokens.insert(pair<string,int>("frequency", channel->Frequency()));

    stringTokens.insert(pair<string,string>("name", channel->Name() ? channel->Name() : ""));
    string channelID = *(channel->GetChannelID().ToString());
    stringTokens.insert(pair<string,string>("channelid", channelID));
    intTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));

    //Channel Source Information
    const cSource *source = Sources.Get(channel->Source());
    if (source) {
        stringTokens.insert(pair<string,string>("source", *cSource::ToString(source->Code())));
        stringTokens.insert(pair<string,string>("sourcedescription", source->Description() ? source->Description() : ""));
        stringTokens.insert(pair<string,string>("source", *cSource::ToString(source->Code())));
        intTokens.insert(pair<string,int>("isAtsc", source->IsAtsc(source->Code())));
        intTokens.insert(pair<string,int>("isCable", source->IsCable(source->Code())));
        intTokens.insert(pair<string,int>("isSat", source->IsSat(source->Code())));
        intTokens.insert(pair<string,int>("isTerr", source->IsTerr(source->Code())));
    }

    //current schedule
    cSchedulesLock schedulesLock;
    const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
    const cSchedule *schedule = NULL;
    schedule = schedules->GetSchedule(channel);
    bool inserted = false;
    if (schedule) {
        const cEvent *presentEvent = schedule->GetPresentEvent();
        if (presentEvent) {
            stringTokens.insert(pair<string,string>("presenteventtitle", presentEvent->Title() ? presentEvent->Title() : ""));
            stringTokens.insert(pair<string,string>("presenteventstart", *presentEvent->GetTimeString()));
            stringTokens.insert(pair<string,string>("presenteventstop", *presentEvent->GetEndTimeString()));
            inserted = true;
        }
    }
    if (!inserted) {
        stringTokens.insert(pair<string,string>("presenteventtitle", ""));
        stringTokens.insert(pair<string,string>("presenteventstart", ""));
        stringTokens.insert(pair<string,string>("presenteventstop", ""));        
    }
}


void cDisplayMenuItemChannelsView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemChannelsView::Render(void) {

    DrawListItem(&stringTokens, &intTokens);
    
    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentChannelView(tmplCurrent, channel);
            currentView->Start();
        }
    }
    
    dirty = false;
}

void cDisplayMenuItemChannelsView::Debug(void) {
    esyslog("skindesigner: Channels Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: Channel: %s", channel ? channel->Name() : "Channel is NULL");
}

/*************************************************************
* Private Functions
*************************************************************/

/*************************************************************
* cDisplayMenuItemTimersView
*************************************************************/

cDisplayMenuItemTimersView::cDisplayMenuItemTimersView(cTemplateViewList *tmplList, const cTimer *timer, bool current, bool selectable) 
                                                           : cDisplayMenuItemView(tmplList, current, selectable) {
    this->timer = timer;
}

cDisplayMenuItemTimersView::~cDisplayMenuItemTimersView() {
}

void cDisplayMenuItemTimersView::SetTokens(void) {
    if (!timer)
        return;
    if (!itemInit) {
        intTokens.erase("flagactive");    
        intTokens.insert(pair<string,int>("flagactive", timer->HasFlags(tfActive)));
        return;
    }
    itemInit = false;
 
    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
    intTokens.insert(pair<string,int>("flagactive", timer->HasFlags(tfActive)));
    intTokens.insert(pair<string,int>("flaginstant", timer->HasFlags(tfInstant)));
    intTokens.insert(pair<string,int>("flagvps", timer->HasFlags(tfVps)));
    intTokens.insert(pair<string,int>("flagrecording", timer->Recording()));
    intTokens.insert(pair<string,int>("flagpending", timer->Pending()));

    
    const char *file = Setup.FoldersInTimerMenu ? NULL : strrchr(timer->File(), FOLDERDELIMCHAR);
    if (file && strcmp(file + 1, TIMERMACRO_TITLE) && strcmp(file + 1, TIMERMACRO_EPISODE))
        file++;
    else
        file = timer->File();
    stringTokens.insert(pair<string,string>("title", file));
    stringTokens.insert(pair<string,string>("timerstart", *cString::sprintf("%02d:%02d", timer->Start() / 100, timer->Start() % 100)));
    stringTokens.insert(pair<string,string>("timerstop", *cString::sprintf("%02d:%02d", timer->Stop() / 100, timer->Stop() % 100)));

    string day = "";
    string dayName = "";
    if (timer->WeekDays())
        day = timer->PrintDay(0, timer->WeekDays(), false);
    else if (timer->Day() - time(NULL) < 28 * SECSINDAY) {
        day = itoa(timer->GetMDay(timer->Day()));
        dayName = WeekDayName(timer->Day());
    } else {
        struct tm tm_r;
        time_t Day = timer->Day();
        localtime_r(&Day, &tm_r);
        char buffer[16];
        strftime(buffer, sizeof(buffer), "%Y%m%d", &tm_r);
        day = buffer;
    }
    
    struct tm tm_r;
    time_t timerDate = timer->Day();
    localtime_r(&timerDate, &tm_r);
    char buffer[4];
    strftime(buffer, sizeof(buffer), "%m", &tm_r);
    int month = atoi(buffer);
    char buffer2[6];
    strftime(buffer2, sizeof(buffer2), "%b", &tm_r);
    string monthName = buffer2;
    char buffer3[6];
    strftime(buffer3, sizeof(buffer3), "%Y", &tm_r);
    int year = atoi(buffer3);

    stringTokens.insert(pair<string,string>("day", day));
    stringTokens.insert(pair<string,string>("dayname", dayName));
    intTokens.insert(pair<string,int>("month", month));
    stringTokens.insert(pair<string,string>("monthname", monthName));
    intTokens.insert(pair<string,int>("year", year));

    const cChannel *channel = timer->Channel();
    if (channel) {
        stringTokens.insert(pair<string,string>("channelname", channel->Name() ? channel->Name() : ""));
        string channelID = *(channel->GetChannelID().ToString());
        stringTokens.insert(pair<string,string>("channelid", channelID));
        intTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));
        intTokens.insert(pair<string,int>("channelnumber", channel->Number()));
    } else {
        stringTokens.insert(pair<string,string>("channelname", ""));
        stringTokens.insert(pair<string,string>("channelid", ""));
        intTokens.insert(pair<string, int>("channellogoexists", 0));
        intTokens.insert(pair<string,int>("channelnumber", 0));        
    }

    const cEvent *event = timer->Event();
    if (event) {
        stringTokens.insert(pair<string,string>("eventtitle", event->Title() ? event->Title() : ""));
        stringTokens.insert(pair<string,string>("eventstart", *event->GetTimeString()));
        stringTokens.insert(pair<string,string>("eventstop", *event->GetEndTimeString()));
    } else {
        stringTokens.insert(pair<string,string>("eventtitle", ""));
        stringTokens.insert(pair<string,string>("eventtitle", ""));
        stringTokens.insert(pair<string,string>("eventstop", ""));
    }
}


void cDisplayMenuItemTimersView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemTimersView::Render(void) {

    DrawListItem(&stringTokens, &intTokens);
    
    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentTimerView(tmplCurrent, timer);
            currentView->Start();
        }
    }
    
    dirty = false;
}

void cDisplayMenuItemTimersView::Debug(void) {
    esyslog("skindesigner: Timers Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: Timer: %s", timer ? *(timer->ToDescr()) : "Timer is NULL");
}

/*************************************************************
* Private Functions
*************************************************************/


/*************************************************************
* cDisplayMenuItemRecordingView
*************************************************************/

cDisplayMenuItemRecordingView::cDisplayMenuItemRecordingView(cTemplateViewList *tmplList, const cRecording *recording, int level, int total, int newRecs, bool current, bool selectable) 
                                                           : cDisplayMenuItemView(tmplList, current, selectable) {
    this->recording = recording;
    this->level = level;
    this->total = total;
    this->newRecs = newRecs;
}

cDisplayMenuItemRecordingView::~cDisplayMenuItemRecordingView() {
}

void cDisplayMenuItemRecordingView::SetTokens(void) {
    if (!itemInit) return;
    itemInit = false;
    if (!recording) return;

    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("nummenuitem", num+1));
    bool isFolder = (total > 0) ? true : false;
    intTokens.insert(pair<string,int>("folder", isFolder));

    string name = recording->Name() ? recording->Name() : "";
    string buffer = "";
    stringstream folderName;
    try {
        vector<string> tokens;
        istringstream f(name.c_str());
        string s;
        int i=0;
        while (getline(f, s, FOLDERDELIMCHAR)) {
            tokens.push_back(s);
            if (isFolder && i <= level) {
                if (i > 0)
                    folderName << FOLDERDELIMCHAR;
                folderName << s;
                i++;
            }
        }
        buffer = tokens.at(level);
        if (!isFolder && recording->IsEdited()) {
            buffer = buffer.substr(1);
        }
    } catch (...) {
        buffer = name.c_str();
    }

    const cRecording *usedRecording = recording;

    if (isFolder) {
        cRecordingsFolderInfo::cFolderInfo *folderInfo = recFolderInfo.Get(folderName.str().c_str());
        if (folderInfo) {
            cRecording *newestRec = Recordings.GetByName(*folderInfo->LatestFileName);
            if (newestRec) {
                usedRecording = newestRec;
            }
            delete folderInfo;
        }
    }

    stringTokens.insert(pair<string,string>("name", buffer.c_str()));
    intTokens.insert(pair<string,int>("new", usedRecording->IsNew()));
    intTokens.insert(pair<string,int>("newrecordingsfolder", newRecs));
    intTokens.insert(pair<string,int>("numrecordingsfolder", total));
    intTokens.insert(pair<string,int>("cutted", usedRecording->IsEdited()));

    const cEvent *event = NULL;
    const cRecordingInfo *info = usedRecording->Info();
    if (!info) return;
    event = info->GetEvent();
    if (!event) return;


    string recDate = *(event->GetDateString());
    string recTime = *(event->GetTimeString());
    if (recDate.find("1970") != string::npos) {
        time_t start = usedRecording->Start();
        recDate = *DateString(start);
        recTime = *TimeString(start);
    }

    time_t startTime = event->StartTime();
    struct tm * sStartTime = localtime(&startTime);
    intTokens.insert(pair<string, int>("year", sStartTime->tm_year + 1900));
    intTokens.insert(pair<string, int>("daynumeric", sStartTime->tm_mday));
    intTokens.insert(pair<string, int>("month", sStartTime->tm_mon+1));

    int duration = event->Duration() / 60;
    int recDuration = usedRecording->LengthInSeconds();
    recDuration = (recDuration>0)?(recDuration / 60):0;
    stringTokens.insert(pair<string,string>("date", recDate.c_str()));
    stringTokens.insert(pair<string,string>("time", recTime.c_str()));
    intTokens.insert(pair<string,int>("duration", recDuration));
    intTokens.insert(pair<string,int>("durationhours", recDuration / 60));
    stringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", recDuration%60)));
    intTokens.insert(pair<string,int>("durationevent", duration));
    intTokens.insert(pair<string,int>("durationeventhours", duration / 60));
    stringTokens.insert(pair<string,string>("durationeventminutes", *cString::sprintf("%.2d", duration%60)));

    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper || !usedRecording) {
        intTokens.insert(pair<string,int>("hasposterthumbnail", false));
        intTokens.insert(pair<string,int>("thumbnailbwidth", -1));
        intTokens.insert(pair<string,int>("thumbnailheight", -1));
        stringTokens.insert(pair<string,string>("thumbnailpath", ""));
        return;
    }

    ScraperGetPosterThumb call;
    call.event = NULL;
    call.recording = usedRecording;
    if (pScraper->Service("GetPosterThumb", &call)) {
        intTokens.insert(pair<string,int>("hasposterthumbnail", FileExists(call.poster.path)));
        intTokens.insert(pair<string,int>("thumbnailbwidth", call.poster.width));
        intTokens.insert(pair<string,int>("thumbnailheight", call.poster.height));
        stringTokens.insert(pair<string,string>("thumbnailpath", call.poster.path));
    } else {
        intTokens.insert(pair<string,int>("hasposterthumbnail", false));
        intTokens.insert(pair<string,int>("thumbnailbwidth", -1));
        intTokens.insert(pair<string,int>("thumbnailheight", -1));
        stringTokens.insert(pair<string,string>("thumbnailpath", ""));
    }
}


void cDisplayMenuItemRecordingView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemRecordingView::Render(void) {
    
    DrawListItem(&stringTokens, &intTokens);
    
    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentRecordingView(tmplCurrent, recording, level, total, newRecs);
            currentView->Start();
        }
    }
    
    dirty = false;
}

void cDisplayMenuItemRecordingView::Debug(void) {
    esyslog("skindesigner: Recording Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: Recording: %s", recording ? recording->Title() : "Recording is NULL");
}

/*************************************************************
* cDisplayMenuItemPluginView
*************************************************************/

cDisplayMenuItemPluginView::cDisplayMenuItemPluginView(cTemplateViewList *tmplList, map<string,string> *plugStringTokens, map<string,int> *plugIntTokens, 
                                                       map<string,vector<map<string,string> > > *pluginLoopTokens, int index, bool current, bool selectable) 
                                                       : cDisplayMenuItemView(tmplList, current, selectable) {

    for (map<string,string>::iterator it = plugStringTokens->begin(); it != plugStringTokens->end(); it++) {
        stringTokens.insert(pair<string,string>(it->first, it->second));
    }

    for (map<string,int>::iterator it = plugIntTokens->begin(); it != plugIntTokens->end(); it++) {
        intTokens.insert(pair<string,int>(it->first, it->second));
    }

    for(map<string,vector<map<string,string> > >::iterator it = pluginLoopTokens->begin(); it != pluginLoopTokens->end(); it++) {
        loopTokens.insert(pair<string,vector<map<string,string> > >(it->first, it->second));
    }
}

cDisplayMenuItemPluginView::~cDisplayMenuItemPluginView() {
}

void cDisplayMenuItemPluginView::SetTokens(void) {
    if (!itemInit) return;
    itemInit = false;
    intTokens.insert(pair<string,int>("current", current));
}


void cDisplayMenuItemPluginView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemPluginView::Render(void) {
    
    DrawListItem(&stringTokens, &intTokens);
    
    if (current) {
        cTemplateViewElement *tmplCurrent = tmplList->GetListElementCurrent();
        if (tmplCurrent) {
            currentView = new cDisplayMenuItemCurrentPluginView(tmplCurrent, stringTokens, intTokens, loopTokens);
            currentView->Start();
        }
    }
    
    dirty = false;
}

void cDisplayMenuItemPluginView::Debug(void) {
    esyslog("skindesigner: Plugin Menu Item ---------------");
    cDisplayMenuItemView::Debug();
}

/*************************************************************
* cDisplayMenuItemTrackView
*************************************************************/

cDisplayMenuItemTrackView::cDisplayMenuItemTrackView(cTemplateViewList *tmplList, const char *title, bool current, bool selectable, int numTracks) 
                                                           : cDisplayMenuItemView(tmplList, current, selectable) {
    this->title = title;
    this->numTracks = numTracks;
}

cDisplayMenuItemTrackView::~cDisplayMenuItemTrackView() {
}

void cDisplayMenuItemTrackView::SetTokens(void) {
    if (!itemInit) return;
    itemInit = false;
    if (!title) return;

    stringTokens.insert(pair<string,string>("title", title));
}


void cDisplayMenuItemTrackView::Prepare(void) {
    ArrangeContainer();
}

void cDisplayMenuItemTrackView::Render(void) {
    
    DrawListItem(&stringTokens, &intTokens);
    
    dirty = false;
}

void cDisplayMenuItemTrackView::Debug(void) {
    esyslog("skindesigner: Tracks Menu Item ---------------");
    cDisplayMenuItemView::Debug();
    esyslog("skindesigner: Title: %s", title);
}

