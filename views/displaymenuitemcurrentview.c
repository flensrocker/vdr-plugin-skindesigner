#include "../services/scraper2vdr.h"
#include "../libcore/helpers.h"
#include "../libcore/recfolderinfo.h"
#include "../libcore/extrecinfo.h"
#include "displaymenuitemcurrentview.h"


cDisplayMenuItemCurrentView::cDisplayMenuItemCurrentView(cTemplateViewElement *tmplCurrent) : cView(tmplCurrent) {
    delay = tmplItem->GetNumericParameter(ptDelay);
    SetFadeTime(tmplItem->GetNumericParameter(ptFadeTime));
}

cDisplayMenuItemCurrentView::~cDisplayMenuItemCurrentView() {
    CancelSave();
}

void cDisplayMenuItemCurrentView::SetPosMenuItem(cRect &pos) {
    posMenuItem.SetX(pos.X()); 
    posMenuItem.SetY(pos.Y());
    posMenuItem.SetWidth(pos.Width());
    posMenuItem.SetHeight(pos.Height());
}

void cDisplayMenuItemCurrentView::SetTokensPosMenuItem(void) {
    intTokens.insert(pair<string,int>("menuitemx", posMenuItem.X()));
    intTokens.insert(pair<string,int>("menuitemy", posMenuItem.Y()));
    intTokens.insert(pair<string,int>("menuitemwidth", posMenuItem.Width()));
    intTokens.insert(pair<string,int>("menuitemheight", posMenuItem.Height()));
}

void cDisplayMenuItemCurrentView::SetScraperPoster(const cEvent *event, const cRecording *recording) {
    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper || (!event && !recording)) {
        intTokens.insert(pair<string,int>("hasposter", false));
        intTokens.insert(pair<string,int>("posterwidth", -1));
        intTokens.insert(pair<string,int>("posterheight", -1));
        stringTokens.insert(pair<string,string>("posterpath", ""));
        return;
    }

    ScraperGetPoster call;
    call.event = event;
    call.recording = recording;
    if (pScraper->Service("GetPoster", &call)) {
        intTokens.insert(pair<string,int>("hasposter", FileExists(call.poster.path)));
        intTokens.insert(pair<string,int>("posterwidth", call.poster.width));
        intTokens.insert(pair<string,int>("posterheight", call.poster.height));
        stringTokens.insert(pair<string,string>("posterpath", call.poster.path));
    } else {
        intTokens.insert(pair<string,int>("hasposter", false));
        intTokens.insert(pair<string,int>("posterwidth", -1));
        intTokens.insert(pair<string,int>("posterheight", -1));
        stringTokens.insert(pair<string,string>("posterpath", ""));
    }
}

/*************************************************************
* cDisplayMenuItemCurrentMainView
*************************************************************/

cDisplayMenuItemCurrentMainView::cDisplayMenuItemCurrentMainView(cTemplateViewElement *tmplCurrent, string number, string label, string icon) 
                         : cDisplayMenuItemCurrentView(tmplCurrent) {
    this->number = number;
    this->label = label;
    this->icon = icon;
}

cDisplayMenuItemCurrentMainView::~cDisplayMenuItemCurrentMainView() {
}

void cDisplayMenuItemCurrentMainView::Prepare(void) {
}


void cDisplayMenuItemCurrentMainView::Render(void) {
    stringTokens.insert(pair<string,string>("number", number));
    stringTokens.insert(pair<string,string>("label", label));
    stringTokens.insert(pair<string,string>("icon", icon));
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens);
}

void cDisplayMenuItemCurrentMainView::Clear(void) {
    
}

void cDisplayMenuItemCurrentMainView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

/*************************************************************
* cDisplayMenuItemCurrentSchedulesView
*************************************************************/

cDisplayMenuItemCurrentSchedulesView::cDisplayMenuItemCurrentSchedulesView(cTemplateViewElement *tmplCurrent, const cEvent *event, const cChannel *channel, eTimerMatch timerMatch, eMenuCategory cat) 
                         : cDisplayMenuItemCurrentView(tmplCurrent) {
    this->event = event;
    this->channel = channel;
    this->timerMatch = timerMatch;
    this->cat = cat;
}

cDisplayMenuItemCurrentSchedulesView::~cDisplayMenuItemCurrentSchedulesView() {
}

void cDisplayMenuItemCurrentSchedulesView::Prepare(void) {
}


void cDisplayMenuItemCurrentSchedulesView::Render(void) {
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
        stringTokens.insert(pair<string,string>("title", event->Title() ? event->Title() : ""));
        stringTokens.insert(pair<string,string>("shorttext", event->ShortText() ? event->ShortText() : ""));
        stringTokens.insert(pair<string,string>("description", event->Description() ? event->Description() : ""));
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
        SetScraperPoster(event);
    }
    if (channel) {
        stringTokens.insert(pair<string,string>("channelname", channel->Name() ? channel->Name() : ""));
        string channelID = *(channel->GetChannelID().ToString());
        stringTokens.insert(pair<string,string>("channelid", channelID));
        intTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));
    }

    vector< map<string,string> > schedulesTokens;
    if (cat == mcScheduleNow || cat == mcScheduleNext) {
        ReadSchedules(&schedulesTokens);
    }
    map < string, vector< map< string, string > > > loopTokens;
    loopTokens.insert(pair<string, vector< map< string, string > > >("schedule", schedulesTokens));
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayMenuItemCurrentSchedulesView::Clear(void) {
    
}

void cDisplayMenuItemCurrentSchedulesView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

void cDisplayMenuItemCurrentSchedulesView::ReadSchedules(vector< map<string,string> > *schedulesTokens) {
    if (!event)
        return;
    const cSchedule *schedule = event->Schedule();
    const cEvent *curEvent = event;
    int num = 0;
    for (; curEvent; curEvent = schedule->Events()->Next(curEvent)) {
        if (num == 0) {
            num++;
            continue;
        }
        map<string,string> element;
        element.insert(pair<string,string>("schedule[start]", *(curEvent->GetTimeString())));
        element.insert(pair<string,string>("schedule[stop]", *(curEvent->GetEndTimeString())));
        element.insert(pair<string,string>("schedule[title]", curEvent->Title() ? curEvent->Title() : ""));
        element.insert(pair<string,string>("schedule[shorttext]", curEvent->ShortText() ? curEvent->ShortText() : ""));
        schedulesTokens->push_back(element);
        num++;
        if (num > 10)
            break;
    }
}

/*************************************************************
* cDisplayMenuItemCurrentChannelView
*************************************************************/

cDisplayMenuItemCurrentChannelView::cDisplayMenuItemCurrentChannelView(cTemplateViewElement *tmplCurrent, const cChannel *channel) 
                         : cDisplayMenuItemCurrentView(tmplCurrent) {
    this->channel = channel;
}

cDisplayMenuItemCurrentChannelView::~cDisplayMenuItemCurrentChannelView() {
}

void cDisplayMenuItemCurrentChannelView::Prepare(void) {
}


void cDisplayMenuItemCurrentChannelView::Render(void) {
    if (!channel)
        return;
    //general channel information
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
    if (schedule) {
        const cEvent *presentEvent = schedule->GetPresentEvent();
        if (presentEvent) {
            stringTokens.insert(pair<string,string>("presenteventtitle", presentEvent->Title() ? presentEvent->Title() : ""));
            stringTokens.insert(pair<string,string>("presenteventshorttext", presentEvent->ShortText() ? presentEvent->ShortText() : ""));
            stringTokens.insert(pair<string,string>("presenteventdescription", presentEvent->Description() ? presentEvent->Description() : ""));
            stringTokens.insert(pair<string,string>("presenteventstart", *presentEvent->GetTimeString()));
            stringTokens.insert(pair<string,string>("presenteventstop", *presentEvent->GetEndTimeString()));
            stringTokens.insert(pair<string,string>("presenteventday", *WeekDayName(presentEvent->StartTime())));
            stringTokens.insert(pair<string,string>("presenteventdate", *ShortDateString(presentEvent->StartTime())));
            intTokens.insert(pair<string,int>("presenteventelapsed", (time(0) - presentEvent->StartTime())/60));
            intTokens.insert(pair<string,int>("presenteventduration", presentEvent->Duration() / 60));
            intTokens.insert(pair<string,int>("presenteventdurationhours", presentEvent->Duration() / 3600));
            stringTokens.insert(pair<string,string>("presenteventdurationminutes", *cString::sprintf("%.2d", (presentEvent->Duration() / 60)%60)));
            SetScraperPoster(presentEvent);
        } else {
            stringTokens.insert(pair<string,string>("presenteventtitle", ""));
            stringTokens.insert(pair<string,string>("presenteventshorttext", ""));
            stringTokens.insert(pair<string,string>("presenteventdescription", ""));
            stringTokens.insert(pair<string,string>("presenteventstart", ""));
            stringTokens.insert(pair<string,string>("presenteventstop", ""));
            stringTokens.insert(pair<string,string>("presenteventday", ""));
            stringTokens.insert(pair<string,string>("presenteventdate", ""));
            intTokens.insert(pair<string,int>("presenteventelapsed", 0));
            intTokens.insert(pair<string,int>("presenteventduration", 0));
            intTokens.insert(pair<string,int>("presenteventdurationhours", 0));
            stringTokens.insert(pair<string,string>("presenteventdurationminutes", ""));      
        }
        const cList<cEvent> *events = schedule->Events();
        bool inserted = false;
        if (events && presentEvent) {
            const cEvent *nextEvent = events->Next(presentEvent);
            if (nextEvent) {
                stringTokens.insert(pair<string,string>("nexteventtitle", nextEvent->Title() ? nextEvent->Title() : ""));
                stringTokens.insert(pair<string,string>("nexteventshorttext", nextEvent->ShortText() ? nextEvent->ShortText() : ""));
                stringTokens.insert(pair<string,string>("nexteventdescription", nextEvent->Description() ? nextEvent->Description() : ""));
                stringTokens.insert(pair<string,string>("nexteventstart", *nextEvent->GetTimeString()));
                stringTokens.insert(pair<string,string>("nexteventstop", *nextEvent->GetEndTimeString()));
                stringTokens.insert(pair<string,string>("nexteventday", *WeekDayName(nextEvent->StartTime())));
                stringTokens.insert(pair<string,string>("nexteventdate", *ShortDateString(nextEvent->StartTime())));
                intTokens.insert(pair<string,int>("nexteventduration", nextEvent->Duration() / 60));
                intTokens.insert(pair<string,int>("nexteventdurationhours", nextEvent->Duration() / 3600));
                stringTokens.insert(pair<string,string>("nexteventdurationminutes", *cString::sprintf("%.2d", (nextEvent->Duration() / 60)%60)));
                inserted = true;
            }
        }
        if (!inserted) {
            stringTokens.insert(pair<string,string>("nexteventtitle", ""));
            stringTokens.insert(pair<string,string>("nexteventshorttext", ""));
            stringTokens.insert(pair<string,string>("nexteventdescription", ""));
            stringTokens.insert(pair<string,string>("nexteventstart", ""));
            stringTokens.insert(pair<string,string>("nexteventstop", ""));
            stringTokens.insert(pair<string,string>("nexteventday", ""));
            stringTokens.insert(pair<string,string>("nexteventdate", ""));
            intTokens.insert(pair<string,int>("nexteventduration", 0));
            intTokens.insert(pair<string,int>("nexteventdurationhours", 0));
            stringTokens.insert(pair<string,string>("nexteventdurationminutes", ""));
        }
    }
    
    vector< map<string,string> > schedulesTokens;
    ReadSchedules(&schedulesTokens);
    
    map < string, vector< map< string, string > > > loopTokens;
    loopTokens.insert(pair<string, vector< map< string, string > > >("schedule", schedulesTokens));
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayMenuItemCurrentChannelView::Clear(void) {
    
}

void cDisplayMenuItemCurrentChannelView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

void cDisplayMenuItemCurrentChannelView::ReadSchedules(vector< map<string,string> > *schedulesTokens) {
    cSchedulesLock schedulesLock;
    const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
    const cSchedule *schedule = NULL;
    schedule = schedules->GetSchedule(channel);
    if (!schedule) {
        return;
    } 
    const cEvent *curEvent = schedule->GetPresentEvent();
    int num = 0;
    for (; curEvent; curEvent = schedule->Events()->Next(curEvent)) {
        if (num < 2) {
            num++;
            continue;
        }
        map<string,string> element;
        element.insert(pair<string,string>("schedule[start]", *(curEvent->GetTimeString())));
        element.insert(pair<string,string>("schedule[stop]", *(curEvent->GetEndTimeString())));
        element.insert(pair<string,string>("schedule[title]", curEvent->Title() ? curEvent->Title() : ""));
        element.insert(pair<string,string>("schedule[shorttext]", curEvent->ShortText() ? curEvent->ShortText() : ""));
        schedulesTokens->push_back(element);
        num++;
        if (num > 11)
            break;
    }
}

/*************************************************************
* cDisplayMenuItemCurrentTimerView
*************************************************************/

cDisplayMenuItemCurrentTimerView::cDisplayMenuItemCurrentTimerView(cTemplateViewElement *tmplCurrent, const cTimer *timer)
                         : cDisplayMenuItemCurrentView(tmplCurrent) {
    this->timer = timer;
}

cDisplayMenuItemCurrentTimerView::~cDisplayMenuItemCurrentTimerView() {
}

void cDisplayMenuItemCurrentTimerView::Prepare(void) {
}


void cDisplayMenuItemCurrentTimerView::Render(void) {
    if (!timer)
        return;
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
        stringTokens.insert(pair<string,string>("eventshorttext", event->ShortText() ? event->ShortText() : ""));
        stringTokens.insert(pair<string,string>("eventdescription", event->Description() ? event->Description() : ""));
        SetScraperPoster(event);
    } else {
        stringTokens.insert(pair<string,string>("eventtitle", ""));
        stringTokens.insert(pair<string,string>("eventstart", ""));
        stringTokens.insert(pair<string,string>("eventstop", ""));
        stringTokens.insert(pair<string,string>("eventshorttext", ""));
        stringTokens.insert(pair<string,string>("eventdescription", ""));
    }
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens);
}

void cDisplayMenuItemCurrentTimerView::Clear(void) {
    
}

void cDisplayMenuItemCurrentTimerView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

/*************************************************************
* cDisplayMenuItemCurrentRecordingView
*************************************************************/

cDisplayMenuItemCurrentRecordingView::cDisplayMenuItemCurrentRecordingView(cTemplateViewElement *tmplCurrent, const cRecording *recording, int level, int total, int newRecs)
                         : cDisplayMenuItemCurrentView(tmplCurrent) {
    this->recording = recording;
    this->level = level;
    this->total = total;
    this->newRecs = newRecs;
}

cDisplayMenuItemCurrentRecordingView::~cDisplayMenuItemCurrentRecordingView() {
}

void cDisplayMenuItemCurrentRecordingView::Prepare(void) {
}


void cDisplayMenuItemCurrentRecordingView::Render(void) {
    if (!recording)
        return;
    map < string, vector< map< string, string > > > loopTokens;

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

    SetScraperPoster(NULL, usedRecording);


    const cRecordingInfo *info = usedRecording->Info();
    if (!info) return;

    bool extRecinfoAvailable = false;    
    if (info->Aux()) {
        cExtRecInfo extRecInfo(info->Aux());
        if (extRecInfo.Parse()) {
            extRecinfoAvailable = true;
            intTokens.insert(pair<string,int>("screenwidth", extRecInfo.resWidth));
            intTokens.insert(pair<string,int>("screenheight", extRecInfo.resHeight));
            intTokens.insert(pair<string,int>("isHD", extRecInfo.isHD));
            intTokens.insert(pair<string,int>("isWideScreen", extRecInfo.isWideScreen));
            intTokens.insert(pair<string,int>("isDolby", extRecInfo.isDolby));
            stringTokens.insert(pair<string,string>("resolution", extRecInfo.resString));
            stringTokens.insert(pair<string,string>("aspect", extRecInfo.aspectratio));
            stringTokens.insert(pair<string,string>("codec", extRecInfo.codec));
            stringTokens.insert(pair<string,string>("format", extRecInfo.format));
            stringTokens.insert(pair<string,string>("framerate", extRecInfo.framerate));
            stringTokens.insert(pair<string,string>("interlace", extRecInfo.interlace));
            intTokens.insert(pair<string,int>("numtracks", extRecInfo.tracks.size()));
            vector< map<string,string> > trackTokens;
            int  trackNumber = 1;
            for (vector<tAudioTrack>::iterator track = extRecInfo.tracks.begin(); track != extRecInfo.tracks.end(); track++) {
                map<string,string> element;
                stringstream trackNum;
                trackNum << trackNumber++;
                element.insert(pair<string,string>("track[num]", trackNum.str()));
                element.insert(pair<string,string>("track[codec]", (*track).codec));
                element.insert(pair<string,string>("track[bitrate]", (*track).bitrate));
                element.insert(pair<string,string>("track[language]", (*track).language));
                trackTokens.push_back(element);
            }            
            loopTokens.insert(pair<string, vector< map< string, string > > >("track", trackTokens));
        }
    }
    intTokens.insert(pair<string,int>("extrecinfoavailable", extRecinfoAvailable));

    stringTokens.insert(pair<string,string>("shorttext", info->ShortText() ? info->ShortText() : ""));
    stringTokens.insert(pair<string,string>("description", info->Description() ? info->Description() : ""));
    
    const cEvent *event = info->GetEvent();
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
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayMenuItemCurrentRecordingView::Clear(void) {
    
}

void cDisplayMenuItemCurrentRecordingView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

/*************************************************************
* cDisplayMenuItemCurrentPluginView
*************************************************************/

cDisplayMenuItemCurrentPluginView::cDisplayMenuItemCurrentPluginView(cTemplateViewElement *tmplCurrent, map <string,string> &plugStringTokens, 
                                                                     map <string,int> &plugIntTokens, map<string,vector<map<string,string> > > &pluginLoopTokens)
                         : cDisplayMenuItemCurrentView(tmplCurrent) {

    stringTokens = plugStringTokens;
    intTokens = plugIntTokens;
    loopTokens = pluginLoopTokens;
}

cDisplayMenuItemCurrentPluginView::~cDisplayMenuItemCurrentPluginView() {
}

void cDisplayMenuItemCurrentPluginView::Prepare(void) {
}


void cDisplayMenuItemCurrentPluginView::Render(void) {
    SetTokensPosMenuItem();
    DrawViewElement(veMenuCurrentItemDetail, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayMenuItemCurrentPluginView::Clear(void) {
    
}

void cDisplayMenuItemCurrentPluginView::Action(void) {
    SetInitFinished();
    DoSleep(delay);
    Render();
    FadeIn();
    DoFlush();
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}
