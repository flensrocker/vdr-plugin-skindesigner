#define __STL_CONFIG_H
#include <vdr/menu.h>
#include "../services/scraper2vdr.h"
#include "displayreplayview.h"
#include "../libcore/helpers.h"

cDisplayReplayView::cDisplayReplayView(cTemplateView *tmplView) : cView(tmplView) {
    lastDate = "";
    DeleteOsdOnExit();
    SetFadeTime(tmplView->GetNumericParameter(ptFadeTime));
}

cDisplayReplayView::~cDisplayReplayView() {
    CancelSave();
    FadeOut();
}

bool cDisplayReplayView::createOsd(void) {
    cRect osdSize = tmplView->GetOsdSize();
    bool ok = CreateOsd(cOsd::OsdLeft() + osdSize.X(),
                        cOsd::OsdTop() + osdSize.Y(),
                        osdSize.Width(),
                        osdSize.Height());
    return ok;
}

void cDisplayReplayView::DrawBackground(bool modeOnly) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    if (modeOnly)
        DrawViewElement(veBackgroundModeOnly, &stringTokens, &intTokens);
    else
        DrawViewElement(veBackground, &stringTokens, &intTokens);
}

void cDisplayReplayView::DrawDate(bool modeOnly) {
    if (modeOnly)
        return;
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

void cDisplayReplayView::DrawTitle(const cRecording *recording) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    
    const char *recName = NULL;
    const cRecordingInfo *recInfo = recording->Info();
    if (recInfo) {
        recName = recInfo->Title();
    }
    if (!recName)
        recName = recording->Name();
    string recShortText = recInfo->ShortText() ? recInfo->ShortText() : "";
    string recDate = *ShortDateString(recording->Start());
    string recTime = *TimeString(recording->Start());

    stringTokens.insert(pair<string,string>("rectitle", recName ? recName : ""));
    stringTokens.insert(pair<string,string>("recsubtitle", recShortText));
    stringTokens.insert(pair<string,string>("recdate", recDate));
    stringTokens.insert(pair<string,string>("rectime", recTime));

    DrawViewElement(veRecTitle, &stringTokens, &intTokens);
}

void cDisplayReplayView::DrawRecordingInformation(const cRecording *recording) {
    map < string, string > stringTokens;
    map < string, int > intTokens;

    int screenWidth = 0;
    int screenHeight = 0;
    double aspect = 0;
    cDevice::PrimaryDevice()->GetVideoSize(screenWidth, screenHeight, aspect);
    bool isHD = false;
    string resName = GetScreenResolutionString(screenWidth, screenHeight, &isHD);
    bool isWideScreen = false;
    string aspectName = GetScreenAspectString(aspect, &isWideScreen);

    intTokens.insert(pair<string,int>("screenwidth", screenWidth));
    intTokens.insert(pair<string,int>("screenheight", screenHeight));
    intTokens.insert(pair<string,int>("isHD", isHD));
    intTokens.insert(pair<string,int>("isWideScreen", isWideScreen));
    stringTokens.insert(pair<string,string>("resolution", resName));
    stringTokens.insert(pair<string,string>("aspect", aspectName));

    ClearViewElement(veRecInfo);
    DrawViewElement(veRecInfo, &stringTokens, &intTokens);
}

void cDisplayReplayView::DrawScraperContent(const cRecording *recording) {
    if (!recording)
        return;

    if (!ViewElementImplemented(veScraperContent)) {
        return;
    }

    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper) {
        return;
    }

    ScraperGetPosterBannerV2 call;
    call.event = NULL;
    call.recording = recording;
    if (pScraper->Service("GetPosterBannerV2", &call)) {
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

void cDisplayReplayView::DrawCurrent(const char *current) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    stringTokens.insert(pair<string,string>("reccurrent", current));

    ClearViewElement(veRecCurrent);
    DrawViewElement(veRecCurrent, &stringTokens, &intTokens);
}

void cDisplayReplayView::DrawTotal(const char *total) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    stringTokens.insert(pair<string,string>("rectotal", total));

    ClearViewElement(veRecTotal);
    DrawViewElement(veRecTotal, &stringTokens, &intTokens);
}

void cDisplayReplayView::DrawProgressBar(int current, int total) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    intTokens.insert(pair<string,int>("current", current));
    intTokens.insert(pair<string,int>("total", total));
    stringTokens.insert(pair<string,string>("dummy", ""));
    ClearViewElement(veRecProgressBar);
    DrawViewElement(veRecProgressBar, &stringTokens, &intTokens);    
}

void cDisplayReplayView::DrawMarks(const cMarks *marks, int total) {
    if (!marks)
        return;

    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > loopTokens;
    vector< map< string, string > > markTokens;
    stringstream tot;
    tot << total;

    bool isStartMark = true;
    for (const cMark *m = marks->First(); m; m = marks->Next(m)) {
        map< string, string > markVals;
        stringstream pos;
        pos << m->Position();
        markVals.insert(pair< string, string >("marks[position]", pos.str()));
        markVals.insert(pair< string, string >("marks[total]", tot.str()));
        markVals.insert(pair< string, string >("marks[startmark]", isStartMark ? "1" : "0"));
        const cMark *m2 = marks->Next(m);
        if (m2) { 
            stringstream posNext;
            posNext << m2->Position();
            markVals.insert(pair< string, string >("marks[endposition]", posNext.str()));
        } else {
            markVals.insert(pair< string, string >("marks[endposition]", tot.str()));            
        }
        isStartMark = !isStartMark;
        markTokens.push_back(markVals);
    }
    loopTokens.insert(pair< string, vector< map< string, string > > >("marks", markTokens));

    ClearViewElement(veCuttingMarks);
    DrawViewElement(veCuttingMarks, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayReplayView::DrawControlIcons(bool play, bool forward, int speed, bool modeOnly) {
    map < string, string > stringTokens;
    map < string, int > intTokens;

    bool isPlay = false;
    bool isPause = false;
    bool isFF = false;
    bool isFF1x = false;
    bool isFF2x = false;
    bool isFF3x = false;
    bool isRew = false;
    bool isRew1x = false;
    bool isRew2x = false;
    bool isRew3x = false;

    if (speed == -1) {
        if (play) {
            isPlay = true;
        } else {
            isPause = true;
        }
    } else if (forward) {
        if (!play) {
            isPause = true;
        }
        if (speed == 1) {
            isFF1x = true;
        } else if (speed == 2) {
            isFF2x = true;
        } else if (speed == 3) {
            isFF3x = true;
        } else {
            isFF = true;
        }
    } else {
        if (!play) {
            isPause = true;
        }
        if (speed == 1) {
            isRew1x = true;
        } else if (speed == 2) {
            isRew2x = true;
        } else if (speed == 3) {
            isRew3x = true;
        } else {
            isRew = true;
        }
    }
    intTokens.insert(pair<string,int>("play", isPlay));
    intTokens.insert(pair<string,int>("pause", isPause));
    intTokens.insert(pair<string,int>("forward", isFF));
    intTokens.insert(pair<string,int>("forward1x", isFF1x));
    intTokens.insert(pair<string,int>("forward2x", isFF2x));
    intTokens.insert(pair<string,int>("forward3x", isFF3x));
    intTokens.insert(pair<string,int>("rewind", isRew));
    intTokens.insert(pair<string,int>("rewind1x", isRew1x));
    intTokens.insert(pair<string,int>("rewind2x", isRew2x));
    intTokens.insert(pair<string,int>("rewind3x", isRew3x));

    if (modeOnly) {
        ClearViewElement(veControlIconsModeOnly);
        DrawViewElement(veControlIconsModeOnly, &stringTokens, &intTokens);
    } else {
        ClearViewElement(veControlIcons);
        DrawViewElement(veControlIcons, &stringTokens, &intTokens);
    }
}

void cDisplayReplayView::DrawJump(const char *jump) {
    if (!jump) {
        ClearViewElement(veRecJump);
        return;
    }
    
    map < string, string > stringTokens;
    map < string, int > intTokens;
    stringTokens.insert(pair<string,string>("jump", jump));

    ClearViewElement(veRecJump);
    DrawViewElement(veRecJump, &stringTokens, &intTokens);        
}

void cDisplayReplayView::DrawMessage(eMessageType type, const char *text) {
    if (!text) {
        ClearViewElement(veMessage);
    }
    
    map < string, string > stringTokens;
    map < string, int > intTokens;
    
    stringTokens.insert(pair<string,string>("text", text));

    intTokens.insert(pair<string,int>("status",  (type == mtStatus)  ? true : false));
    intTokens.insert(pair<string,int>("info",    (type == mtInfo)    ? true : false));
    intTokens.insert(pair<string,int>("warning", (type == mtWarning) ? true : false));
    intTokens.insert(pair<string,int>("error",   (type == mtError)   ? true : false));
    
    ClearViewElement(veMessage);
    DrawViewElement(veMessage, &stringTokens, &intTokens);
}


void cDisplayReplayView::Action(void) {
    SetInitFinished();
    FadeIn();
    DoFlush();
    cView::Action();
}

string cDisplayReplayView::GetScreenResolutionString(int width, int height, bool *isHD) {
    string name = "";
    switch (width) {
        case 1920:
        case 1440:
            name = "hd1080i";
            *isHD = true;
            break;
        case 1280:
            if (height == 720)
                name = "hd720p";
            else
                name = "hd1080i";
            *isHD = true;
            break;
        case 720:
            name = "sd576i";
            break;
        default:
            name = "sd576i";
            break;
    }
    return name;
}

string cDisplayReplayView::GetScreenAspectString(double aspect, bool *isWideScreen) {
    string name = "";
    *isWideScreen = false;
    if (aspect == 4.0/3.0) {
        name = "4:3";
        *isWideScreen = false;
    } else if (aspect == 16.0/9.0) {
        name = "16:9";
        *isWideScreen = true;
    } else if (aspect == 2.21) {
        name = "21:9";
        *isWideScreen = true;
    }
    return name;
}