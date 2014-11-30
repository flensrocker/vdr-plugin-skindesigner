#define __STL_CONFIG_H
#include <vdr/player.h>
#include "displayreplayonpauseview.h"

cDisplayReplayOnPauseView::cDisplayReplayOnPauseView(cTemplateViewElement *tmplViewElement) : cView(tmplViewElement) {
    tmplViewElement->SetPixOffset(0);
    delay = tmplViewElement->GetNumericParameter(ptDelay) * 1000;
    SetFadeTime(tmplViewElement->GetNumericParameter(ptFadeTime));
}

cDisplayReplayOnPauseView::~cDisplayReplayOnPauseView() {
    CancelSave();
    FadeOut();
}

void cDisplayReplayOnPauseView::Render(void) {
    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > loopTokens;
    const cRecording *recording = NULL;
    cControl *control = cControl::Control();
    if (control) {
        recording = control->GetRecording();
    }
    if (recording) {
        string name = recording->Name() ? recording->Name() : "";
        stringTokens.insert(pair<string,string>("name", name));
        const cRecordingInfo *info = recording->Info();
        if (info) {
            stringTokens.insert(pair<string,string>("shorttext", info->ShortText() ? info->ShortText() : ""));
            stringTokens.insert(pair<string,string>("description", info->Description() ? info->Description() : ""));
            const cEvent *event = info->GetEvent();
            if (event) {
                string recDate = *(event->GetDateString());
                string recTime = *(event->GetTimeString());
                if (recDate.find("1970") != string::npos) {
                    time_t start = recording->Start();
                    recDate = *DateString(start);
                    recTime = *TimeString(start);
                }
                stringTokens.insert(pair<string,string>("date", recDate.c_str()));
                stringTokens.insert(pair<string,string>("time", recTime.c_str()));
                time_t startTime = event->StartTime();
                struct tm * sStartTime = localtime(&startTime);
                intTokens.insert(pair<string, int>("year", sStartTime->tm_year + 1900));
                intTokens.insert(pair<string, int>("daynumeric", sStartTime->tm_mday));
                intTokens.insert(pair<string, int>("month", sStartTime->tm_mon+1));
                int duration = event->Duration() / 60;
                int recDuration = recording->LengthInSeconds();
                recDuration = (recDuration>0)?(recDuration / 60):0;
                intTokens.insert(pair<string,int>("duration", recDuration));
                intTokens.insert(pair<string,int>("durationhours", recDuration / 60));
                stringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", recDuration%60)));
                intTokens.insert(pair<string,int>("durationevent", duration));
                intTokens.insert(pair<string,int>("durationeventhours", duration / 60));
                stringTokens.insert(pair<string,string>("durationeventminutes", *cString::sprintf("%.2d", duration%60)));
            }
        } else {
            stringTokens.insert(pair<string,string>("shorttext", ""));
            stringTokens.insert(pair<string,string>("description", ""));            
            int recDuration = recording->LengthInSeconds();
            recDuration = (recDuration>0)?(recDuration / 60):0;
            stringTokens.insert(pair<string,string>("date", ""));
            stringTokens.insert(pair<string,string>("time", ""));
            intTokens.insert(pair<string,int>("duration", recDuration));
            intTokens.insert(pair<string,int>("durationhours", recDuration / 60));
            stringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", recDuration%60)));
            intTokens.insert(pair<string,int>("durationevent", 0));
            intTokens.insert(pair<string,int>("durationeventhours", 0));
            stringTokens.insert(pair<string,string>("durationeventminutes", ""));
        }
    } else {
        stringTokens.insert(pair<string,string>("name", ""));
        stringTokens.insert(pair<string,string>("shorttext", ""));
        stringTokens.insert(pair<string,string>("description", ""));
        stringTokens.insert(pair<string,string>("date", ""));
        stringTokens.insert(pair<string,string>("time", ""));
        stringTokens.insert(pair<string,string>("durationminutes", ""));
        stringTokens.insert(pair<string,string>("durationeventminutes", ""));
    }
    SetScraperTokens(NULL, recording, stringTokens, intTokens, loopTokens);
    DrawViewElement(veOnPause, &stringTokens, &intTokens, &loopTokens);
}

void cDisplayReplayOnPauseView::Action(void) {
    DoSleep(delay);
    if (!Running())
        return;
    Render();
    SetInitFinished();
    FadeIn();
    DoFlush();
}
