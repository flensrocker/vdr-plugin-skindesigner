#include "displaymenudetailview.h"
#include "../libcore/helpers.h"
#include "../services/scraper2vdr.h"
#include "../services/epgsearch.h"

cDisplayMenuDetailView::cDisplayMenuDetailView(cTemplateView *tmplDetailView) : cView(tmplDetailView) {
    event = NULL;
    recording = NULL;
    text = NULL;
    detailViewInit = true;
    isPluginTextView = false;
    currentTmplTab = NULL;
    tabView = NULL;
}

cDisplayMenuDetailView::~cDisplayMenuDetailView() {
    CancelSave();
    if (tabView)
        delete tabView;
}

void cDisplayMenuDetailView::SetPluginTokens(map<string,string> *plugStringTokens, map<string,int> *plugIntTokens, map<string,vector<map<string,string> > > *plugLoopTokens) {
    for (map<string,string>::iterator it = plugStringTokens->begin(); it != plugStringTokens->end(); it++) {
        stringTokens.insert(pair<string,string>(it->first, it->second));
    }

    for (map<string,int>::iterator it = plugIntTokens->begin(); it != plugIntTokens->end(); it++) {
        intTokens.insert(pair<string,int>(it->first, it->second));
    }

    for(map<string,vector<map<string,string> > >::iterator it = plugLoopTokens->begin(); it != plugLoopTokens->end(); it++) {
        loopTokens.insert(pair<string,vector<map<string,string> > >(it->first, it->second));
    }
    isPluginTextView = true;
}

void cDisplayMenuDetailView::Clear(void) {
    ClearViewElement(veDetailHeader);
    ClearViewElement(veScrollbar);
    ClearViewElement(veTabLabels);
}

void cDisplayMenuDetailView::Render(void) {
    if (detailViewInit) {
        DrawHeader();
        DoFlush();
        SetTokens();
        InitTabs();
        currentTmplTab = *atIt;
        detailViewInit = false;
    }
    if (!tabView) {
        tabView = new cDisplayMenuTabView(currentTmplTab);
        tabView->SetTokens(&intTokens, &stringTokens, &loopTokens);
        tabView->CreateTab();
        tabView->Start();
    }
    DrawScrollbar();
    DrawTabLabels();
}

void cDisplayMenuDetailView::KeyLeft(void) {
    if (activeTabs.size() > 1) {
        currentTmplTab = GetPrevTab();
        delete tabView;
        tabView = NULL;
        Render();
        DoFlush();
    } else {
        bool scrolled = tabView->KeyLeft();
        if (scrolled) {
            DrawScrollbar();
            DoFlush();
        }
    }
}

void cDisplayMenuDetailView::KeyRight(void) {
    if (activeTabs.size() > 1) {
        currentTmplTab = GetNextTab();
        delete tabView;
        tabView = NULL;
        Render();
        DoFlush();
    } else {
        bool scrolled = tabView->KeyRight();
        if (scrolled) {
            DrawScrollbar();
            DoFlush();
        }
    }
}

void cDisplayMenuDetailView::KeyUp(void) {
    if (!tabView)
        return;
    bool scrolled = tabView->KeyUp();
    if (scrolled) {
        DrawScrollbar();
        DoFlush();
    }
}

void cDisplayMenuDetailView::KeyDown(void) {
    if (!tabView)
        return;
    bool scrolled = tabView->KeyDown();
    if (scrolled) {
        DrawScrollbar();
        DoFlush();
    }
}


void cDisplayMenuDetailView::SetTokens(void) {
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

        string channelID = *(event->ChannelID().ToString());
        stringTokens.insert(pair<string,string>("channelid", channelID));
        intTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));

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
        if (event->Vps())
            stringTokens.insert(pair<string,string>("vps", *event->GetVpsString()));
        else
            stringTokens.insert(pair<string,string>("vps", ""));

        vector< map< string, string > > reruns;
        bool hasReruns = LoadReruns(&reruns);
        loopTokens.insert(pair<string, vector< map< string, string > > >("reruns", reruns));

        intTokens.insert(pair<string,int>("hasreruns", hasReruns));

        SetScraperTokens(event, recording, stringTokens, intTokens, loopTokens);
        SetEpgPictures(event->EventID());

    } else if (recording) {
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
        LoadRecordingInformation();
        SetScraperTokens(event, recording, stringTokens, intTokens, loopTokens);
        SetRecordingImages(recording->FileName());
    } else if (text) {
        stringTokens.insert(pair<string,string>("text", text));
    } else {
        intTokens.insert(pair<string,int>("running", false));
        intTokens.insert(pair<string,int>("hasreruns", false));
    }
}

void cDisplayMenuDetailView::InitTabs(void) {
    tmplView->InitViewTabIterator();
    cTemplateViewTab *tmplTab = NULL;
    while(tmplTab = tmplView->GetNextViewTab()) {
        tmplTab->ParseDynamicParameters(&intTokens, true);
        tmplTab->ClearDynamicFunctionParameters();
        tmplTab->ParseDynamicFunctionParameters(&stringTokens, &intTokens);
        if (tmplTab->DoExecute()) {
            activeTabs.push_back(tmplTab);
        }
    }
    atIt = activeTabs.begin();
}

bool cDisplayMenuDetailView::LoadReruns(vector< map< string, string > > *reruns) {
    if (!event)
        return false;
    
    cPlugin *epgSearchPlugin = cPluginManager::GetPlugin("epgsearch");
    if (!epgSearchPlugin)
        return false;

    if (isempty(event->Title()))
        return false;

    int maxNumReruns = config.rerunAmount;
    int rerunDistance = config.rerunDistance * 3600;
    int rerunNaxChannel = config.rerunMaxChannel;

    Epgsearch_searchresults_v1_0 data;
    string strQuery = (event->Title()) ? event->Title() : "";
    data.query = (char *)strQuery.c_str();
    data.mode = 0;
    data.channelNr = 0;
    data.useTitle = true;
    data.useSubTitle = true;
    data.useDescription = false;

    bool foundRerun = false;
    if (epgSearchPlugin->Service("Epgsearch-searchresults-v1.0", &data)) {
        cList<Epgsearch_searchresults_v1_0::cServiceSearchResult>* list = data.pResultList;
        if (list && (list->Count() > 1)) {
            foundRerun = true;
            int i = 0;
            for (Epgsearch_searchresults_v1_0::cServiceSearchResult *r = list->First(); r && i < maxNumReruns; r = list->Next(r)) {
                time_t eventStart = event->StartTime();
                time_t rerunStart = r->event->StartTime();
                cChannel *channel = Channels.GetByChannelID(r->event->ChannelID(), true, true);
                //check for identical event
                if ((event->ChannelID() == r->event->ChannelID()) && (eventStart == rerunStart))
                    continue;
                //check for timely distance
                if (rerunDistance > 0) {
                    if (rerunStart - eventStart < rerunDistance) {
                        continue;
                    }
                }
                //check for maxchannel
                if (rerunNaxChannel > 0) {
                    if (channel && channel->Number() > rerunNaxChannel) {
                        continue;
                    }
                }
                i++;
                map< string, string > rerun;
                rerun.insert(pair<string, string>("reruns[title]", r->event->Title() ? r->event->Title() : ""));
                rerun.insert(pair<string, string>("reruns[shorttext]", r->event->ShortText() ? r->event->ShortText() : ""));
                rerun.insert(pair<string, string>("reruns[start]", *(r->event->GetTimeString())));
                rerun.insert(pair<string, string>("reruns[start]", *(r->event->GetTimeString())));
                rerun.insert(pair<string, string>("reruns[stop]", *(r->event->GetEndTimeString())));
                rerun.insert(pair<string, string>("reruns[date]", *ShortDateString(r->event->StartTime())));
                rerun.insert(pair<string, string>("reruns[day]", *WeekDayName(r->event->StartTime())));
                string channelID = *(r->event->ChannelID().ToString());
                rerun.insert(pair<string, string>("reruns[channelid]", channelID));
                bool logoExists = imgCache->LogoExists(channelID);
                rerun.insert(pair<string, string>("reruns[channellogoexists]", logoExists ? "1" : "0"));

                if (channel) {
                    stringstream channelNumber;
                    channelNumber << channel->Number();
                    rerun.insert(pair<string, string>("reruns[channelname]", channel->ShortName(true)));
                    rerun.insert(pair<string, string>("reruns[channelnumber]", channelNumber.str()));
                } else {
                    rerun.insert(pair<string, string>("reruns[channelname]", ""));
                    rerun.insert(pair<string, string>("reruns[channelnumber]", ""));                    
                }
                reruns->push_back(rerun);
            }
            delete list;
        }
    }
    return foundRerun;
}

void cDisplayMenuDetailView::LoadRecordingInformation(void) {
    const cRecordingInfo *Info = recording->Info();
    if (!Info)
        return;
    unsigned long long nRecSize = -1;
    unsigned long long nFileSize[1000];
    nFileSize[0] = 0;
    int i = 0;
    struct stat filebuf;
    cString filename;
    int rc = 0;
    do {
        if (recording->IsPesRecording())
            filename = cString::sprintf("%s/%03d.vdr", recording->FileName(), ++i);
        else
            filename = cString::sprintf("%s/%05d.ts", recording->FileName(), ++i);
        rc = stat(filename, &filebuf);
        if (rc == 0)
            nFileSize[i] = nFileSize[i-1] + filebuf.st_size;
        else
            if (ENOENT != errno) {
                nRecSize = -1;
            }
    } while (i <= 999 && !rc);
    nRecSize = nFileSize[i-1];

    cMarks marks;
    bool fHasMarks = marks.Load(recording->FileName(), recording->FramesPerSecond(), recording->IsPesRecording()) && marks.Count();
    cIndexFile *index = new cIndexFile(recording->FileName(), false, recording->IsPesRecording());

    int nCutLength = 0;
    long nCutInFrame = 0;
    unsigned long long nRecSizeCut = nRecSize < 0 ? -1 : 0;
    unsigned long long nCutInOffset = 0;

    if (fHasMarks && index) {
        uint16_t FileNumber;
        off_t FileOffset;

        bool fCutIn = true;
        cMark *mark = marks.First();
        while (mark) {
            int pos = mark->Position();
            index->Get(pos, &FileNumber, &FileOffset); //TODO: will disc spin up?
            if (fCutIn) {
                nCutInFrame = pos;
                fCutIn = false;
                if (nRecSize >= 0)
                    nCutInOffset = nFileSize[FileNumber-1] + FileOffset;
            } else {
                nCutLength += pos - nCutInFrame;
                fCutIn = true;
                if (nRecSize >= 0)
                    nRecSizeCut += nFileSize[FileNumber-1] + FileOffset - nCutInOffset;
            }
            cMark *nextmark = marks.Next(mark);
            mark = nextmark;
        }
        if (!fCutIn) {
            nCutLength += index->Last() - nCutInFrame;
            index->Get(index->Last() - 1, &FileNumber, &FileOffset);
            if (nRecSize >= 0)
                nRecSizeCut += nFileSize[FileNumber-1] + FileOffset - nCutInOffset;
        }
    }

    if (nRecSize < 0) {
        if ((nRecSize = ReadSizeVdr(recording->FileName())) < 0) {
            nRecSize = DirSizeMB(recording->FileName());
        }
    }
    if (nRecSize >= 0) {
        cString strRecSize = "";
        cString strRecSizeCut = "";

        if (fHasMarks) {
            if (nRecSize > MEGABYTE(1023)) {
                strRecSize = cString::sprintf("%.2f GB", (float)nRecSize / MEGABYTE(1024));
                strRecSizeCut = cString::sprintf("%.2f GB", (float)nRecSizeCut / MEGABYTE(1024));
            } else {
                strRecSize = cString::sprintf("%lld MB", nRecSize / MEGABYTE(1));
                strRecSizeCut = cString::sprintf("%lld MB", nRecSizeCut / MEGABYTE(1));
            }
        } else {
            if (nRecSize > MEGABYTE(1023)) {
                strRecSize = cString::sprintf("%.2f GB", (float)nRecSize / MEGABYTE(1024));
                strRecSizeCut = strRecSize;
            } else {
                strRecSize = cString::sprintf("%lld MB", nRecSize / MEGABYTE(1));
                strRecSizeCut = strRecSize;
            }
        }
        stringTokens.insert(pair<string,string>("recordingsize", *strRecSize));
        stringTokens.insert(pair<string,string>("recordingsizecutted", *strRecSizeCut));
    } else {
        stringTokens.insert(pair<string,string>("recordingsize", ""));        
        stringTokens.insert(pair<string,string>("recordingsizecutted", ""));        
    }

    cChannel *channel = Channels.GetByChannelID(Info->ChannelID());
    if (channel) {
        stringTokens.insert(pair<string,string>("recchannelname", channel->Name()));
        intTokens.insert(pair<string,int>("recchannelnumber", channel->Number()));
    }

    if (index) {
        int nLastIndex = index->Last();
        if (nLastIndex) {
            string strLength = *IndexToHMSF(nLastIndex, false, recording->FramesPerSecond());
            string strLengthCutted = "";
            if (fHasMarks) {
                strLengthCutted = *IndexToHMSF(nCutLength, false, recording->FramesPerSecond());
            } else {
                strLengthCutted = strLength;
            }
            string strBitrate = *cString::sprintf("%.2f MBit/s", (float)nRecSize / nLastIndex * recording->FramesPerSecond() * 8 / MEGABYTE(1));
            stringTokens.insert(pair<string,string>("recordinglength", strLength));
            stringTokens.insert(pair<string,string>("recordinglengthcutted", strLengthCutted));
            stringTokens.insert(pair<string,string>("recordingbitrate", strBitrate));
        }
        delete index;
    }
    
    string recFormat = recording->IsPesRecording() ? "PES" : "TS";
    stringTokens.insert(pair<string,string>("recordingformat", recFormat));
    
    bool searchTimerFound = false;
    if (Info) {
        const char *aux = NULL;
        aux = Info->Aux();
        if (aux) {
            string strAux = aux;
            string auxEpgsearch = StripXmlTag(strAux, "epgsearch");
            if (!auxEpgsearch.empty()) {
                string searchTimer = StripXmlTag(auxEpgsearch, "searchtimer");
                if (!searchTimer.empty()) {
                    stringTokens.insert(pair<string,string>("searchtimer", searchTimer));
                    searchTimerFound = true;
                }
            }
        }
    }
    if (!searchTimerFound)
        stringTokens.insert(pair<string,string>("searchtimer", "n.a."));
}

string cDisplayMenuDetailView::StripXmlTag(string &Line, const char *Tag) {
        // set the search strings
        stringstream strStart, strStop;
        strStart << "<" << Tag << ">";
        strStop << "</" << Tag << ">";
        // find the strings
        string::size_type locStart = Line.find(strStart.str());
        string::size_type locStop = Line.find(strStop.str());
        if (locStart == string::npos || locStop == string::npos)
                return "";
        // extract relevant text
        int pos = locStart + strStart.str().size();
        int len = locStop - pos;
        return len < 0 ? "" : Line.substr(pos, len);
}


int cDisplayMenuDetailView::ReadSizeVdr(const char *strPath) {
    int dirSize = -1;
    char buffer[20];
    char *strFilename = NULL;
    if (-1 != asprintf(&strFilename, "%s/size.vdr", strPath)) {
        struct stat st;
        if (stat(strFilename, &st) == 0) {
                int fd = open(strFilename, O_RDONLY);
            if (fd >= 0) {
                if (safe_read(fd, &buffer, sizeof(buffer)) >= 0) {
                    dirSize = atoi(buffer);
                }
                close(fd);
            }
        }
        free(strFilename);
    }
    return dirSize;
}

void cDisplayMenuDetailView::SetEpgPictures(int eventId) {
    for (int i=0; i<3; i++) {
        stringstream picName;
        picName << eventId << "_" << i;
        bool epgPicAvailable = FileExists(*config.epgImagePath, picName.str(), "jpg");
        stringstream available;
        stringstream path;
        available << "epgpic" << i+1 << "avaialble";
        path << "epgpic" << i+1 << "path";
        if (epgPicAvailable) {
            intTokens.insert(pair<string,int>(available.str(), true));
            stringTokens.insert(pair<string,string>(path.str(), *cString::sprintf("%s%s.jpg", *config.epgImagePath, picName.str().c_str())));
        } else {
            intTokens.insert(pair<string,int>(available.str(), false));
            stringTokens.insert(pair<string,string>(path.str(), ""));            
        }
    }
}

void cDisplayMenuDetailView::SetRecordingImages(const char *recPath) {
    if (!recPath) {
        intTokens.insert(pair<string,int>("recimg1avaialble", false));
        intTokens.insert(pair<string,int>("recimg2avaialble", false));
        intTokens.insert(pair<string,int>("recimg3avaialble", false));
        stringTokens.insert(pair<string,string>("recimg1path", ""));
        stringTokens.insert(pair<string,string>("recimg2path", ""));
        stringTokens.insert(pair<string,string>("recimg3path", ""));
        return;
    }

    string path = recPath;
    DIR *dirHandle;
    struct dirent *dirEntry;
    dirHandle = opendir(recPath);
    if (!dirHandle) {
        intTokens.insert(pair<string,int>("recimg1avaialble", false));
        intTokens.insert(pair<string,int>("recimg2avaialble", false));
        intTokens.insert(pair<string,int>("recimg3avaialble", false));
        stringTokens.insert(pair<string,string>("recimg1path", ""));
        stringTokens.insert(pair<string,string>("recimg2path", ""));
        stringTokens.insert(pair<string,string>("recimg3path", ""));
        return;        
    }

    int picsFound = 0;
    while ( 0 != (dirEntry = readdir(dirHandle))) {
        if (endswith(dirEntry->d_name, "jpg")) {
            string fileName = dirEntry->d_name;
            stringstream available;
            available << "recimg" << picsFound+1  << "avaialble";
            stringstream path;
            path << "recimg" << picsFound+1 << "path";
            intTokens.insert(pair<string,int>(available.str(), true));
            stringTokens.insert(pair<string,string>(path.str(), *cString::sprintf("%s/%s", recPath, fileName.c_str())));
            picsFound++;
        }
        if (picsFound == 3) {
            break;
        }
    }
    for (int i=picsFound; i<3; i++) {
        stringstream available;
        available << "recimg" << i+1  << "avaialble";
        stringstream path;
        path << "recimg" << i+1 << "path";
        intTokens.insert(pair<string,int>(available.str(), false));
        stringTokens.insert(pair<string,string>(path.str(), ""));
    }
    closedir(dirHandle);
}

void cDisplayMenuDetailView::DrawHeader(void) {
    map < string, string > headerStringTokens;
    map < string, int > headerIntTokens;

    if (event || recording) {
        static cPlugin *pScraper = GetScraperPlugin();
        if (!pScraper) {
            headerIntTokens.insert(pair<string,int>("ismovie", false));
            headerIntTokens.insert(pair<string,int>("isseries", false));
            headerIntTokens.insert(pair<string,int>("posteravailable", false));
            headerIntTokens.insert(pair<string,int>("banneravailable", false));
        } else {
            ScraperGetEventType getType;
            getType.event = event;
            getType.recording = recording;
            if (!pScraper->Service("GetEventType", &getType)) {
                headerIntTokens.insert(pair<string,int>("ismovie", false));
                headerIntTokens.insert(pair<string,int>("isseries", false));
                headerIntTokens.insert(pair<string,int>("posteravailable", false));
                headerIntTokens.insert(pair<string,int>("banneravailable", false));
            } else {
                if (getType.type == tMovie) {
                    cMovie movie;
                    movie.movieId = getType.movieId;
                    pScraper->Service("GetMovie", &movie);
                    headerIntTokens.insert(pair<string,int>("ismovie", true));
                    headerIntTokens.insert(pair<string,int>("isseries", false));
                    headerIntTokens.insert(pair<string,int>("posteravailable", true));
                    headerIntTokens.insert(pair<string,int>("banneravailable", false));
                    headerStringTokens.insert(pair<string,string>("posterpath", movie.poster.path));
                    headerIntTokens.insert(pair<string,int>("posterwidth", movie.poster.width));
                    headerIntTokens.insert(pair<string,int>("posterheight", movie.poster.height));
                } else if (getType.type == tSeries) {
                    cSeries series;
                    series.seriesId = getType.seriesId;
                    series.episodeId = getType.episodeId;
                    pScraper->Service("GetSeries", &series);
                    headerIntTokens.insert(pair<string,int>("ismovie", false));
                    headerIntTokens.insert(pair<string,int>("isseries", true));
                    vector<cTvMedia>::iterator poster = series.posters.begin();
                    if (poster != series.posters.end()) {
                        headerIntTokens.insert(pair<string,int>("posterwidth", (*poster).width));
                        headerIntTokens.insert(pair<string,int>("posterheight", (*poster).height));
                        headerStringTokens.insert(pair<string,string>("posterpath", (*poster).path));
                        headerIntTokens.insert(pair<string,int>("posteravailable", true));
                    } else {
                        headerIntTokens.insert(pair<string,int>("posterwidth", 0));
                        headerIntTokens.insert(pair<string,int>("posterheight", 0));
                        headerStringTokens.insert(pair<string,string>("posterpath", ""));                    
                        headerIntTokens.insert(pair<string,int>("posteravailable", false));
                    }
                    vector<cTvMedia>::iterator banner = series.banners.begin();
                    if (banner != series.banners.end()) {
                        headerIntTokens.insert(pair<string,int>("bannerwidth", (*banner).width));
                        headerIntTokens.insert(pair<string,int>("bannerheight", (*banner).height));
                        headerStringTokens.insert(pair<string,string>("bannerpath", (*banner).path));
                        headerIntTokens.insert(pair<string,int>("banneravailable", true));
                    } else {
                        headerIntTokens.insert(pair<string,int>("bannerwidth", 0));
                        headerIntTokens.insert(pair<string,int>("bannerheight", 0));
                        headerStringTokens.insert(pair<string,string>("bannerpath", ""));                    
                        headerIntTokens.insert(pair<string,int>("banneravailable", false));
                    }
                } else {
                    headerIntTokens.insert(pair<string,int>("ismovie", false));
                    headerIntTokens.insert(pair<string,int>("isseries", false));        
                    headerIntTokens.insert(pair<string,int>("posteravailable", false));
                    headerIntTokens.insert(pair<string,int>("banneravailable", false));
                }
            }
        }
    }

    if (event) {
        headerStringTokens.insert(pair<string,string>("title", event->Title() ? event->Title() : ""));
        headerStringTokens.insert(pair<string,string>("shorttext", event->ShortText() ? event->ShortText() : ""));
        headerStringTokens.insert(pair<string,string>("start", *(event->GetTimeString())));
        headerStringTokens.insert(pair<string,string>("stop", *(event->GetEndTimeString())));
        
        time_t startTime = event->StartTime();
        headerStringTokens.insert(pair<string,string>("day", *WeekDayName(startTime)));
        headerStringTokens.insert(pair<string,string>("date", *ShortDateString(startTime)));
        struct tm * sStartTime = localtime(&startTime);
        headerIntTokens.insert(pair<string, int>("year", sStartTime->tm_year + 1900));
        headerIntTokens.insert(pair<string, int>("daynumeric", sStartTime->tm_mday));
        headerIntTokens.insert(pair<string, int>("month", sStartTime->tm_mon+1));

        const cChannel *channel = Channels.GetByChannelID(event->ChannelID());
        if (channel) {
            headerStringTokens.insert(pair<string,string>("channelname", channel->Name() ? channel->Name() : ""));
            headerIntTokens.insert(pair<string, int>("channelnumber", channel->Number()));
        } else {
            headerStringTokens.insert(pair<string,string>("channelname", ""));            
            headerIntTokens.insert(pair<string, int>("channelnumber", 0));
        }
        string channelID = *(channel->GetChannelID().ToString());
        headerStringTokens.insert(pair<string,string>("channelid", channelID));
        headerIntTokens.insert(pair<string, int>("channellogoexists", imgCache->LogoExists(channelID)));

        bool isRunning = false;
        time_t now = time(NULL);
        if ((now >= event->StartTime()) && (now <= event->EndTime()))
            isRunning = true;
        headerIntTokens.insert(pair<string,int>("running", isRunning));
        if (isRunning) {
            headerIntTokens.insert(pair<string,int>("elapsed", (now - event->StartTime())/60));
        } else {
            headerIntTokens.insert(pair<string,int>("elapsed", 0));
        }
        headerIntTokens.insert(pair<string,int>("duration", event->Duration() / 60));
        headerIntTokens.insert(pair<string,int>("durationhours", event->Duration() / 3600));
        headerStringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", (event->Duration() / 60)%60)));
        if (event->Vps())
            headerStringTokens.insert(pair<string,string>("vps", *event->GetVpsString()));
        else
            headerStringTokens.insert(pair<string,string>("vps", ""));

        stringstream epgImageName;
        epgImageName << event->EventID();
        bool epgPicAvailable = FileExists(*config.epgImagePath, epgImageName.str(), "jpg");
        if (epgPicAvailable) {
            headerIntTokens.insert(pair<string,int>("epgpicavailable", true));
            headerStringTokens.insert(pair<string,string>("epgpicpath", *cString::sprintf("%s%s.jpg", *config.epgImagePath, epgImageName.str().c_str())));
        } else {
            epgImageName << "_0";
            epgPicAvailable = FileExists(*config.epgImagePath, epgImageName.str(), "jpg");
            if (epgPicAvailable) {
                headerIntTokens.insert(pair<string,int>("epgpicavailable", true));
                headerStringTokens.insert(pair<string,string>("epgpicpath", *cString::sprintf("%s%s.jpg", *config.epgImagePath, epgImageName.str().c_str())));
            } else {
                headerIntTokens.insert(pair<string,int>("epgpicavailable", false));
                headerStringTokens.insert(pair<string,string>("epgpicpath", ""));                
            }
        }

        DrawViewElement(veDetailHeader, &headerStringTokens, &headerIntTokens);
        return;
    } else if (recording) {
        string name = recording->Name() ? recording->Name() : "";
        headerStringTokens.insert(pair<string,string>("name", name));

        const cRecordingInfo *info = recording->Info();
        if (info) {
            headerStringTokens.insert(pair<string,string>("shorttext", info->ShortText() ? info->ShortText() : ""));
            const cEvent *event = info->GetEvent();
            if (event) {
                string recDate = *(event->GetDateString());
                string recTime = *(event->GetTimeString());
                if (recDate.find("1970") != string::npos) {
                    time_t start = recording->Start();
                    recDate = *DateString(start);
                    recTime = *TimeString(start);
                }
                int duration = event->Duration() / 60;
                int recDuration = recording->LengthInSeconds();
                recDuration = (recDuration>0)?(recDuration / 60):0;
                headerStringTokens.insert(pair<string,string>("date", recDate.c_str()));
                headerStringTokens.insert(pair<string,string>("time", recTime.c_str()));
                time_t startTime = event->StartTime();
                struct tm * sStartTime = localtime(&startTime);
                headerIntTokens.insert(pair<string, int>("year", sStartTime->tm_year + 1900));
                headerIntTokens.insert(pair<string, int>("daynumeric", sStartTime->tm_mday));
                headerIntTokens.insert(pair<string, int>("month", sStartTime->tm_mon+1));
                headerIntTokens.insert(pair<string,int>("duration", recDuration));
                headerIntTokens.insert(pair<string,int>("durationhours", recDuration / 60));
                headerStringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", recDuration%60)));
                headerIntTokens.insert(pair<string,int>("durationevent", duration));
                headerIntTokens.insert(pair<string,int>("durationeventhours", duration / 60));
                headerStringTokens.insert(pair<string,string>("durationeventminutes", *cString::sprintf("%.2d", duration%60)));
            }
        } else {
            headerStringTokens.insert(pair<string,string>("shorttext", ""));
            int recDuration = recording->LengthInSeconds();
            recDuration = (recDuration>0)?(recDuration / 60):0;
            headerStringTokens.insert(pair<string,string>("date", ""));
            headerStringTokens.insert(pair<string,string>("time", ""));
            headerIntTokens.insert(pair<string,int>("duration", recDuration));
            headerIntTokens.insert(pair<string,int>("durationhours", recDuration / 60));
            headerStringTokens.insert(pair<string,string>("durationminutes", *cString::sprintf("%.2d", recDuration%60)));
            headerIntTokens.insert(pair<string,int>("durationevent", 0));
            headerIntTokens.insert(pair<string,int>("durationeventhours", 0));
            headerStringTokens.insert(pair<string,string>("durationeventminutes", ""));
        }

        string recImage = "";
        string path = recording->FileName() ? recording->FileName() : "";
        string extension =  ".jpg";
        if (FirstFileInFolder(path, extension, recImage)) {
            headerIntTokens.insert(pair<string,int>("recimgavailable", true));
            headerStringTokens.insert(pair<string,string>("recimgpath", *cString::sprintf("%s/%s", path.c_str(), recImage.c_str())));            
        } else {
            headerIntTokens.insert(pair<string,int>("recimgavailable", false));
            headerStringTokens.insert(pair<string,string>("recimgpath", ""));            
        }
        DrawViewElement(veDetailHeader, &headerStringTokens, &headerIntTokens);
        return;
    }

    if (isPluginTextView) {
        DrawViewElement(veDetailHeader, &stringTokens, &intTokens);
    }
}

void cDisplayMenuDetailView::DrawScrollbar(void) {
    map < string, string > scrollbarStringTokens;
    map < string, int > scrollbarIntTokens;

    int barTop = 0;
    int barHeight = 0;
    tabView->GetScrollbarPosition(barTop, barHeight);
    
    scrollbarIntTokens.insert(pair<string,int>("height", barHeight));
    scrollbarIntTokens.insert(pair<string,int>("offset", barTop));
    ClearViewElement(veScrollbar);
    DrawViewElement(veScrollbar, &scrollbarStringTokens, &scrollbarIntTokens);
}

void cDisplayMenuDetailView::DrawTabLabels(void) {
    if (!ViewElementImplemented(veTabLabels)) {
        return;
    }
    map < string, string > labelStringTokens;
    map < string, int > labelIntTokens;
    map < string, vector< map< string, string > > > labelLoopTokens;

    vector< map< string, string > > tabLabels;
    for (list<cTemplateViewTab*>::iterator it = activeTabs.begin(); it != activeTabs.end(); it++) {
        cTemplateViewTab *tab = *it;
        map< string, string > tabLabel;
        tabLabel.insert(pair< string, string >("tabs[title]", tab->GetName()));
        if (tab == currentTmplTab) {
            tabLabel.insert(pair< string, string >("tabs[current]", "1"));
        } else {
            tabLabel.insert(pair< string, string >("tabs[current]", "0"));            
        }
        tabLabels.push_back(tabLabel);
    }
    labelLoopTokens.insert(pair< string, vector< map< string, string > > >("tabs", tabLabels));

    ClearViewElement(veTabLabels);
    DrawViewElement(veTabLabels, &labelStringTokens, &labelIntTokens, &labelLoopTokens);
}

cTemplateViewTab *cDisplayMenuDetailView::GetPrevTab(void) {
    if (atIt == activeTabs.begin()) {
        atIt = activeTabs.end();
    }
    atIt--;
    return *atIt;
}

cTemplateViewTab *cDisplayMenuDetailView::GetNextTab(void) {
    atIt++;
    if (atIt == activeTabs.end()) {
        atIt = activeTabs.begin();
    }
    return *atIt;
}
