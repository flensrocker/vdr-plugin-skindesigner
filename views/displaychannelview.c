#define __STL_CONFIG_H
#include <vdr/menu.h>
#include "../services/scraper2vdr.h"
#include "displaychannelview.h"
#include "../libcore/timers.h"
#include "../libcore/helpers.h"


cDisplayChannelView::cDisplayChannelView(cTemplateView *tmplView) : cView(tmplView) {
    lastDate = "";
    lastScreenWidth = 0;
    lastScreenHeight = 0;
    lastSignalDisplay = 0;
    lastSignalStrength = 0;
    lastSignalQuality = 0;
    lastNumAudioTracks = 0;
    lastAudioChannel = -1;
    lastTracDesc = "";
    lastTrackLang = "";
    InitDevices();
    DeleteOsdOnExit();
    SetFadeTime(tmplView->GetNumericParameter(ptFadeTime));
}

cDisplayChannelView::~cDisplayChannelView() {
    CancelSave();
    FadeOut();
}

bool cDisplayChannelView::createOsd(void) {
    cRect osdSize = tmplView->GetOsdSize();
    bool ok = CreateOsd(cOsd::OsdLeft() + osdSize.X(),
                        cOsd::OsdTop() + osdSize.Y(),
                        osdSize.Width(),
                        osdSize.Height());
    return ok;
}

void cDisplayChannelView::DrawBackground(void) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    DrawViewElement(veBackground, &stringTokens, &intTokens);
}

void cDisplayChannelView::DrawChannel(cString &number, cString &name, cString &id, bool switching) {
    map < string, string > stringTokens;
    map < string, int > intTokens;

    stringTokens.insert(pair<string,string>("channelname", *name));
    stringTokens.insert(pair<string,string>("channelnumber", *number));
    stringTokens.insert(pair<string,string>("channelid", *id));

    intTokens.insert(pair<string, int>("switching", switching));
    bool logoExisis = imgCache->LogoExists(*id);
    intTokens.insert(pair<string, int>("channellogoexists", logoExisis));

    DrawViewElement(veChannelInfo, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearChannel(void) {
    ClearViewElement(veChannelInfo);
}

void cDisplayChannelView::DrawDate(void) {
    if (!ViewElementImplemented(veDateTime)) {
        return;
    }
    cString curDate = DayDateTime();
    if (strcmp(curDate, lastDate)) {
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

        lastDate = curDate;
    }
}

void cDisplayChannelView::DrawProgressBar(cString &start, cString &stop, int Current, int Total) {
    if (!ViewElementImplemented(veProgressBar)) {
        return;
    }

    map < string, string > stringTokens;
    map < string, int > intTokens;

    stringTokens.insert(pair<string, string>("start", *start));
    stringTokens.insert(pair<string, string>("stop", *stop));

    intTokens.insert(pair<string, int>("duration", Total));
    intTokens.insert(pair<string, int>("elapsed", Current));
    intTokens.insert(pair<string, int>("remaining", Total - Current));

    ClearProgressBar();
    DrawViewElement(veProgressBar, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearProgressBar(void) {
    ClearViewElement(veProgressBar);
}

void cDisplayChannelView::DrawProgressBarBack(void) {
    DrawViewElement(veProgressBarBack);
}

void cDisplayChannelView::ClearProgressBarBack(void) {
    ClearViewElement(veProgressBarBack);
}

void cDisplayChannelView::DrawEPGInfo(const cEvent *present, const cEvent *next, bool presentRecording, bool nextRecording) {   
    map < string, string > stringTokens;
    map < string, int > intTokens;

    if (present) {
        stringTokens.insert(pair<string,string>("currenttitle", (present->Title())?present->Title():""));
        stringTokens.insert(pair<string,string>("currentsubtitle", (present->ShortText())?present->ShortText():""));
        stringTokens.insert(pair<string,string>("currentstart", *present->GetTimeString()));
        stringTokens.insert(pair<string,string>("currentstop", *present->GetEndTimeString()));
        
        intTokens.insert(pair<string,int>("currentduration", present->Duration() / 60));
        intTokens.insert(pair<string,int>("currentdurationhours", present->Duration() / 3600));
        stringTokens.insert(pair<string,string>("currentdurationminutes", *cString::sprintf("%.2d", (present->Duration() / 60)%60)));
        intTokens.insert(pair<string,int>("currentelapsed", (int)round((time(NULL) - present->StartTime())/60)));
        intTokens.insert(pair<string,int>("currentremaining", (int)round((present->EndTime() - time(NULL))/60)));
        intTokens.insert(pair<string,int>("currentrecording", presentRecording ? 1 : 0));
        intTokens.insert(pair<string,int>("hasVPS", (bool)present->Vps()));
    } else {
        stringTokens.insert(pair<string,string>("currenttitle", ""));
        stringTokens.insert(pair<string,string>("currentsubtitle", ""));
        stringTokens.insert(pair<string,string>("currentstart", ""));
        stringTokens.insert(pair<string,string>("currentstop", ""));
        
        intTokens.insert(pair<string,int>("currentduration", 0));
        intTokens.insert(pair<string,int>("currentdurationhours", 0));
        stringTokens.insert(pair<string,string>("currentdurationminutes", ""));
        intTokens.insert(pair<string,int>("currentelapsed", 0));
        intTokens.insert(pair<string,int>("currentremaining", 0));
        intTokens.insert(pair<string,int>("currentrecording", 0));
        intTokens.insert(pair<string,int>("hasVPS", 0));
    }

    if (next) {
        stringTokens.insert(pair<string,string>("nexttitle", (next->Title())?next->Title():""));
        stringTokens.insert(pair<string,string>("nextsubtitle", (next->ShortText())?next->ShortText():""));
        stringTokens.insert(pair<string,string>("nextstart", *next->GetTimeString()));
        stringTokens.insert(pair<string,string>("nextstop", *next->GetEndTimeString()));

        intTokens.insert(pair<string,int>("nextduration", next->Duration() / 60));
        intTokens.insert(pair<string,int>("nextdurationhours", next->Duration() / 3600));
        stringTokens.insert(pair<string,string>("nextdurationminutes", *cString::sprintf("%.2d", (next->Duration() / 60)%60)));
        intTokens.insert(pair<string,int>("nextrecording", nextRecording ? 1 : 0));
    } else {
        stringTokens.insert(pair<string,string>("nexttitle", ""));
        stringTokens.insert(pair<string,string>("nextsubtitle", ""));
        stringTokens.insert(pair<string,string>("nextstart", ""));
        stringTokens.insert(pair<string,string>("nextstop", ""));

        intTokens.insert(pair<string,int>("nextduration", 0));
        intTokens.insert(pair<string,int>("nextdurationhours", 0));
        stringTokens.insert(pair<string,string>("nextdurationminutes", ""));
        intTokens.insert(pair<string,int>("nextrecording", 0));
    }
    DrawViewElement(veEpgInfo, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearEPGInfo(void) {
    ClearViewElement(veEpgInfo);
}

void cDisplayChannelView::DrawStatusIcons(const cChannel *Channel) {
    if (!ViewElementImplemented(veStatusInfo)) {
        return;
    }

    map < string, int > intTokens;
    map < string, string > stringTokens;

    bool isRadio = !Channel->Vpid() && Channel->Apid(0);
    bool hasVT = Channel->Vpid() && Channel->Tpid();
    bool isStereo = Channel->Apid(0);
    bool isDolby = Channel->Dpid(0);
    bool isEncrypted = Channel->Ca();
    bool isRecording = cRecordControls::Active();
    cGlobalSortedTimers SortedTimers;// local and remote timers
    for (int i = 0; i < SortedTimers.Size() && !isRecording; i++)
        if (const cTimer *Timer = SortedTimers[i])
            if (Timer->Recording()) 
                isRecording = true;

    intTokens.insert(pair<string,int>("isRadio", isRadio));
    intTokens.insert(pair<string,int>("hasVT", hasVT));
    intTokens.insert(pair<string,int>("isStereo", isStereo));
    intTokens.insert(pair<string,int>("isDolby", isDolby));
    intTokens.insert(pair<string,int>("isEncrypted", isEncrypted));
    intTokens.insert(pair<string,int>("isRecording", isRecording));

    DrawViewElement(veStatusInfo, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearStatusIcons(void) {
    ClearViewElement(veStatusInfo);
}

void cDisplayChannelView::DrawAudioInfo(void) {
    if (!ViewElementImplemented(veAudioInfo)) {
        return;
    }

    int numAudioTracks = 0;
    int audioChannel = -1;
    string trackDescription = "";
    string trackLanguage = "";

    cDevice *device = cDevice::PrimaryDevice();
    if (device) {
        numAudioTracks = device->NumAudioTracks();
        audioChannel = device->GetAudioChannel();
        if (numAudioTracks > 0) {
            const tTrackId *track = device->GetTrack(device->GetCurrentAudioTrack());
            if (track) {
                trackDescription = track->description ? track->description : "";
                trackLanguage = track->language ? track->language : "";
            }
        }
    }
    if (lastNumAudioTracks != numAudioTracks || lastAudioChannel != audioChannel || lastTracDesc.compare(trackDescription) || lastTrackLang.compare(trackLanguage)) {
        lastNumAudioTracks = numAudioTracks;
        lastAudioChannel = audioChannel;
        lastTracDesc = trackDescription;
        lastTrackLang = trackLanguage;

        map < string, int > intTokens;
        map < string, string > stringTokens;
        intTokens.insert(pair<string,int>("numaudiotracks", numAudioTracks));
        intTokens.insert(pair<string,int>("audiochannel", audioChannel));
        stringTokens.insert(pair<string,string>("trackdesc", trackDescription));
        stringTokens.insert(pair<string,string>("tracklang", trackLanguage));
        
        ClearAudioInfo();
        DrawViewElement(veAudioInfo, &stringTokens, &intTokens);
    }
}

void cDisplayChannelView::ClearAudioInfo(void) {
    ClearViewElement(veAudioInfo);
}

void cDisplayChannelView::DrawScreenResolution(void) {
    if (!ViewElementImplemented(veScreenResolution)) {
        return;
    }

    int screenWidth = 0;
    int screenHeight = 0;
    double aspect = 0;

    cDevice::PrimaryDevice()->GetVideoSize(screenWidth, screenHeight, aspect);

    if ((lastScreenWidth == screenWidth) && (lastScreenHeight == screenHeight))
        return;

    if ((screenWidth == 0) && (screenHeight == 0))
        return;

    lastScreenWidth = screenWidth;
    lastScreenHeight = screenHeight;

    bool isHD = false;
    string resName = GetScreenResolutionString(screenWidth, screenHeight, &isHD);

    bool isWideScreen = false;
    string aspectName = GetScreenAspectString(aspect, &isWideScreen);

    map < string, string > stringTokens;
    map < string, int > intTokens;

    intTokens.insert(pair<string,int>("screenwidth", screenWidth));
    intTokens.insert(pair<string,int>("screenheight", screenHeight));
    intTokens.insert(pair<string,int>("isHD", isHD));
    intTokens.insert(pair<string,int>("isWideScreen", isWideScreen));
    stringTokens.insert(pair<string,string>("resolution", resName));
    stringTokens.insert(pair<string,string>("aspect", aspectName));

    ClearViewElement(veScreenResolution);
    DrawViewElement(veScreenResolution, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearScreenResolution(void) {
    ClearViewElement(veScreenResolution);
    lastScreenWidth = 0;
    lastScreenHeight = 0;
}

void cDisplayChannelView::DrawScraperContent(const cEvent *event) {
    if (!event)
        return;

    if (!ViewElementImplemented(veScraperContent)) {
        return;
    }

    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper) {
        return;
    }

    ScraperGetPosterBanner call;
    call.event = event;
    if (pScraper->Service("GetPosterBanner", &call)) {
        int mediaWidth = 0;
        int mediaHeight = 0;
        std::string mediaPath = "";
        bool isBanner = false;

        if ((call.type == tSeries) && call.banner.path.size() > 0) {
            mediaWidth = call.banner.width;
            mediaHeight = call.banner.height;
            mediaPath = call.banner.path;
            isBanner = true;
        } else if (call.type == tMovie && call.poster.path.size() > 0 && call.poster.height > 0) {
            mediaWidth = call.poster.width;
            mediaHeight = call.poster.height;
            mediaPath = call.poster.path;
        } else
            return;

        map < string, int > intTokens;
        map < string, string > stringTokens;
        intTokens.insert(pair<string,int>("mediawidth", mediaWidth));
        intTokens.insert(pair<string,int>("mediaheight", mediaHeight));
        intTokens.insert(pair<string,int>("isbanner", isBanner));
        stringTokens.insert(pair<string,string>("mediapath", mediaPath));
        ClearViewElement(veScraperContent);
        DrawViewElement(veScraperContent, &stringTokens, &intTokens);
    }
}

void cDisplayChannelView::ClearScraperContent(void) {
    ClearViewElement(veScraperContent);
}

void cDisplayChannelView::DrawSignal(void) {
    if (!ViewElementImplemented(veSignalQuality)) {
        return;
    }
    time_t Now = time(NULL);
    if (Now != lastSignalDisplay) {
        int SignalStrength = cDevice::ActualDevice()->SignalStrength();
        int SignalQuality = cDevice::ActualDevice()->SignalQuality();
        if (SignalStrength < 0) SignalStrength = 0;
        if (SignalQuality < 0) SignalQuality = 0;
        if ((SignalStrength == 0)&&(SignalQuality==0))
            return;
        if ((lastSignalStrength != SignalStrength) || (lastSignalQuality != SignalQuality)) {
            map < string, int > intTokens;
            map < string, string > stringTokens;
            intTokens.insert(pair<string,int>("signalstrength", SignalStrength));
            intTokens.insert(pair<string,int>("signalquality", SignalQuality));
            ClearViewElement(veSignalQuality);
            DrawViewElement(veSignalQuality, &stringTokens, &intTokens);
        }
        lastSignalStrength = SignalStrength;
        lastSignalQuality = SignalQuality;
        lastSignalDisplay = Now;
    }
}

void cDisplayChannelView::DrawSignalBackground(void) {
    if (!ViewElementImplemented(veSignalQualityBack)) {
        return;
    }
    DrawViewElement(veSignalQualityBack);
}


void cDisplayChannelView::ClearSignal(void) {
    ClearViewElement(veSignalQuality);
}

void cDisplayChannelView::ClearSignalBackground(void) {
    ClearViewElement(veSignalQualityBack);
}

void cDisplayChannelView::DrawDevices(bool initial) {
    if (!ViewElementImplemented(veDevices)) {
        return;
    }
    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > deviceLoopTokens;
    vector< map< string, string > > devices;

    bool changed = SetDevices(initial, &intTokens, &devices);
    if (!changed)
        return;

    deviceLoopTokens.insert(pair< string, vector< map< string, string > > >("devices", devices));
    
    ClearViewElement(veDevices);
    DrawViewElement(veDevices, &stringTokens, &intTokens, &deviceLoopTokens);
}

void cDisplayChannelView::ClearDevices(void) {
    ClearViewElement(veDevices);
}

void cDisplayChannelView::DrawChannelGroups(const cChannel *Channel, cString ChannelName) {
    if (!ViewElementImplemented(veChannelGroup)) {
        return;
    }

    bool separatorExists = imgCache->SeparatorLogoExists(*ChannelName);
    string separatorPath = separatorExists ? *ChannelName : "";

    std::string prevChannelSep = GetChannelSep(Channel, true);
    std::string nextChannelSep = GetChannelSep(Channel, false);
    bool prevAvailable = (prevChannelSep.size() > 0)?true:false;
    bool nextAvailable = (nextChannelSep.size() > 0)?true:false;

    map < string, string > stringTokens;
    map < string, int > intTokens;

    intTokens.insert(pair<string,int>("sepexists", separatorExists));
    intTokens.insert(pair<string,int>("prevAvailable", prevAvailable));
    intTokens.insert(pair<string,int>("nextAvailable", nextAvailable));
    stringTokens.insert(pair<string,string>("group", *ChannelName));
    stringTokens.insert(pair<string,string>("nextgroup", nextChannelSep));
    stringTokens.insert(pair<string,string>("prevgroup", prevChannelSep));
    stringTokens.insert(pair<string,string>("seppath", separatorPath));

    ClearViewElement(veChannelGroup);
    DrawViewElement(veChannelGroup, &stringTokens, &intTokens);
}

void cDisplayChannelView::ClearChannelGroups(void) {
    ClearViewElement(veChannelGroup);
}

string cDisplayChannelView::GetChannelSep(const cChannel *channel, bool prev) {
    std::string sepName = "";
    const cChannel *sep = prev ? Channels.Prev(channel) :
                                 Channels.Next(channel);
    for (; sep; (prev)?(sep = Channels.Prev(sep)):(sep = Channels.Next(sep))) {
        if (sep->GroupSep()) {
            sepName = sep->Name();
            break;
        }
    }
    return sepName;
}

void cDisplayChannelView::DisplayMessage(eMessageType Type, const char *Text) {
    if (!Text) {
        ClearViewElement(veMessage);
    }
    
    map < string, string > stringTokens;
    map < string, int > intTokens;
    
    stringTokens.insert(pair<string,string>("text", Text));

    intTokens.insert(pair<string,int>("status",  (Type == mtStatus)  ? true : false));
    intTokens.insert(pair<string,int>("info",    (Type == mtInfo)    ? true : false));
    intTokens.insert(pair<string,int>("warning", (Type == mtWarning) ? true : false));
    intTokens.insert(pair<string,int>("error",   (Type == mtError)   ? true : false));
    
    ClearViewElement(veMessage);
    DrawViewElement(veMessage, &stringTokens, &intTokens);
}

void cDisplayChannelView::DrawCustomTokens(void) {
    if (!ViewElementImplemented(veCustomTokens)) {
        return;
    }
    if (!tmplView)
        return;
    map < string, string > stringTokens = tmplView->GetCustomTokens();
    map < string, int > intTokens;
    DrawViewElement(veCustomTokens, &stringTokens, &intTokens);
}

void cDisplayChannelView::Action(void) {
    SetInitFinished();
    FadeIn();
    DoFlush();
    cView::Action();
}
