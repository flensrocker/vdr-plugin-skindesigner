#include <vdr/menu.h>
#include "../services/scraper2vdr.h"
#include "../config.h"
#include "../libcore/helpers.h"
#include "viewhelpers.h"

cViewHelpers::cViewHelpers(void) {
    devicesInit = false;
}

cViewHelpers::~cViewHelpers() {
    if (devicesInit) {
        delete[] lastSignalStrength;
        delete[] lastSignalQuality;
        delete[] recDevices;
    }
}

void cViewHelpers::InitDevices(void) {
    int numDevices = cDevice::NumDevices();
    lastSignalStrength = new int[numDevices];
    lastSignalQuality = new int[numDevices];
    recDevices = new bool[numDevices];
    for (int i=0; i<numDevices; i++) {
        lastSignalStrength[i] = 0;
        lastSignalQuality[i] = 0;
        recDevices[i] = false;
    }
    devicesInit = true;
}

bool cViewHelpers::SetDevices(bool initial, map<string,int> *intTokens, vector<map<string,string> > *devices) {
#ifdef DOPROFILE
    cStopWatch watch("SetDevices");
#endif
    int numDevices = cDevice::NumDevices();
    if (!initial) {
        //check if drawing is necessary
        bool changed = false;
        for (int i = 0; i < numDevices; i++) {
            const cDevice *device = cDevice::GetDevice(i);
            if (!device || !device->NumProvidedSystems()) {
                continue;
            }
            int signalStrength = device->SignalStrength();
#ifdef DOPROFILE
            watch.Report(*cString::sprintf("SignalStrength() device %d", i));
#endif
            int signalQuality = device->SignalQuality();
#ifdef DOPROFILE
            watch.Report(*cString::sprintf("SignalQuality() device %d", i));
#endif

            if ((signalStrength != lastSignalStrength[i]) || (signalQuality != lastSignalQuality[i])) {
                changed = true;
                break;
            }            
        }
        if (!changed) {
#ifdef DOPROFILE
            watch.Stop("SetDevices End");
#endif
            return false;
        }
    }
    //check device which currently displays live tv
    int deviceLiveTV = -1;
    cDevice *primaryDevice = cDevice::PrimaryDevice();
    if (primaryDevice) {
        if (!primaryDevice->Replaying() || primaryDevice->Transferring())
            deviceLiveTV = cDevice::ActualDevice()->DeviceNumber();
        else
            deviceLiveTV = primaryDevice->DeviceNumber();
    }
    //check currently recording devices
    for (cTimer *timer = Timers.First(); timer; timer = Timers.Next(timer)) {
        if (!timer->Recording()) {
            continue;
        }
        if (cRecordControl *RecordControl = cRecordControls::GetRecordControl(timer)) {
            const cDevice *recDevice = RecordControl->Device();
            if (recDevice) {
                recDevices[recDevice->DeviceNumber()] = true;
            }
        }
    }
    int actualNumDevices = 0;
    for (int i = 0; i < numDevices; i++) {
        const cDevice *device = cDevice::GetDevice(i);
        if (!device || !device->NumProvidedSystems()) {
            continue;
        }
        
        map< string, string > deviceVals;
        stringstream strNum;
        strNum << actualNumDevices;
        actualNumDevices++;
        deviceVals.insert(pair< string, string >("devices[num]", strNum.str()));
        deviceVals.insert(pair< string, string >("devices[type]", *(device->DeviceType())));
        cCamSlot *camSlot = device->CamSlot();
        int camNumber = -1;
        if (camSlot) {
            camNumber = camSlot->SlotNumber();
            deviceVals.insert(pair< string, string >("devices[hascam]", "1"));
        } else {
            deviceVals.insert(pair< string, string >("devices[hascam]", "0"));
        }
        int signalStrength = device->SignalStrength();
#ifdef DOPROFILE
        watch.Report(*cString::sprintf("SignalStrength() device %d", i));
#endif
        int signalQuality = device->SignalQuality();
#ifdef DOPROFILE
        watch.Report(*cString::sprintf("SignalQuality() device %d", i));
#endif
        stringstream strCamNumber;
        strCamNumber << camNumber;
        deviceVals.insert(pair< string, string >("devices[cam]", strCamNumber.str()));
        stringstream strStrength;
        strStrength << signalStrength;
        deviceVals.insert(pair< string, string >("devices[signalstrength]", strStrength.str()));
        stringstream strQuality;
        strQuality << signalQuality;
        deviceVals.insert(pair< string, string >("devices[signalquality]", strQuality.str()));

        deviceVals.insert(pair< string, string >("devices[livetv]", i == deviceLiveTV ? "1" : "0"));
        deviceVals.insert(pair< string, string >("devices[recording]", recDevices[i] ? "1" : "0"));

        const cChannel *channel = device->GetCurrentlyTunedTransponder();
        const cSource *source = (channel) ? Sources.Get(channel->Source()) : NULL;
        if (channel && channel->Number() > 0) {
            stringstream strChanNum;
            strChanNum << channel->Number();
            deviceVals.insert(pair< string, string >("devices[channelnumber]", strChanNum.str()));
            deviceVals.insert(pair< string, string >("devices[channelname]", channel->Name()));
            string channelID = *(channel->GetChannelID().ToString());
            deviceVals.insert(pair< string, string >("devices[channelid]", channelID));
            deviceVals.insert(pair< string, string >("devices[channellogoexists]", imgCache->LogoExists(channelID) ? "1" : "0"));
            deviceVals.insert(pair< string, string >("devices[istuned]", "1"));
        } else {
            deviceVals.insert(pair< string, string >("devices[channelnumber]", "0"));
            deviceVals.insert(pair< string, string >("devices[channelname]", ""));
            deviceVals.insert(pair< string, string >("devices[channelid]", ""));
            deviceVals.insert(pair< string, string >("devices[channellogoexists]", "0"));
            deviceVals.insert(pair< string, string >("devices[istuned]", "0"));
        }
    
        deviceVals.insert(pair< string, string >("devices[source]", source ? source->Description() : ""));

        devices->push_back(deviceVals);

        lastSignalStrength[i] = signalStrength;
        lastSignalQuality[i] = signalQuality;
    }

    intTokens->insert(pair<string, int>("numdevices", actualNumDevices));
#ifdef DOPROFILE
    watch.Stop("SetDevices End");
#endif
    return true;
}

bool cViewHelpers::CheckNewMails(void) {
    static cPlugin *pMailbox = cPluginManager::GetPlugin("mailbox");
    if (!pMailbox) {
        return false;
    }
    bool newMail = false;
    if (pMailbox->Service("MailBox-HasNewMail-1.0", &newMail)) {
        return newMail;
    }
    return false;
}

void cViewHelpers::SetScraperTokens(const cEvent *event, const cRecording *recording, map < string, string > &stringTokens, map < string, int > &intTokens, map < string, vector< map< string, string > > > &loopTokens) {
    static cPlugin *pScraper = GetScraperPlugin();
    if (!pScraper || (!event && !recording)) {
        intTokens.insert(pair<string,int>("ismovie", false));
        intTokens.insert(pair<string,int>("isseries", false));
        return;
    }

    ScraperGetEventType getType;
    getType.event = event;
    getType.recording = recording;
    if (!pScraper->Service("GetEventType", &getType)) {
        intTokens.insert(pair<string,int>("ismovie", false));
        intTokens.insert(pair<string,int>("isseries", false));
        return;
    }

    if (getType.type == tMovie) {
        cMovie movie;
        movie.movieId = getType.movieId;
        pScraper->Service("GetMovie", &movie);
        intTokens.insert(pair<string,int>("ismovie", true));
        intTokens.insert(pair<string,int>("isseries", false));

        stringTokens.insert(pair<string,string>("movietitle", movie.title));
        stringTokens.insert(pair<string,string>("movieoriginalTitle", movie.originalTitle));
        stringTokens.insert(pair<string,string>("movietagline", movie.tagline));
        stringTokens.insert(pair<string,string>("movieoverview", movie.overview));
        stringTokens.insert(pair<string,string>("moviegenres", movie.genres));
        stringTokens.insert(pair<string,string>("moviehomepage", movie.homepage));
        stringTokens.insert(pair<string,string>("moviereleasedate", movie.releaseDate));
        stringstream pop;
        pop << movie.popularity;
        stringTokens.insert(pair<string,string>("moviepopularity", pop.str()));
        stringstream vote;
        vote << movie.voteAverage;
        stringTokens.insert(pair<string,string>("movievoteaverage", pop.str()));
        stringTokens.insert(pair<string,string>("posterpath", movie.poster.path));
        stringTokens.insert(pair<string,string>("fanartpath", movie.fanart.path));
        stringTokens.insert(pair<string,string>("collectionposterpath", movie.collectionPoster.path));
        stringTokens.insert(pair<string,string>("collectionfanartpath", movie.collectionFanart.path));

        intTokens.insert(pair<string,int>("movieadult", movie.adult));
        intTokens.insert(pair<string,int>("moviebudget", movie.budget));
        intTokens.insert(pair<string,int>("movierevenue", movie.revenue));
        intTokens.insert(pair<string,int>("movieruntime", movie.runtime));
        intTokens.insert(pair<string,int>("posterwidth", movie.poster.width));
        intTokens.insert(pair<string,int>("posterheight", movie.poster.height));
        intTokens.insert(pair<string,int>("fanartwidth", movie.fanart.width));
        intTokens.insert(pair<string,int>("fanartheight", movie.fanart.height));
        intTokens.insert(pair<string,int>("collectionposterwidth", movie.collectionPoster.width));
        intTokens.insert(pair<string,int>("collectionposterheight", movie.collectionPoster.height));
        intTokens.insert(pair<string,int>("collectionfanartwidth", movie.collectionFanart.width));
        intTokens.insert(pair<string,int>("collectionfanartheight", movie.collectionFanart.height));

        vector< map< string, string > > actors;
        for (vector<cActor>::iterator act = movie.actors.begin(); act != movie.actors.end(); act++) {
            map< string, string > actor;
            actor.insert(pair<string, string>("actors[name]", (*act).name));
            actor.insert(pair<string, string>("actors[role]", (*act).role));
            actor.insert(pair<string, string>("actors[thumb]", (*act).actorThumb.path));
            stringstream actWidth, actHeight;
            actWidth << (*act).actorThumb.width;
            actHeight << (*act).actorThumb.height;
            actor.insert(pair<string, string>("actors[thumbwidth]", actWidth.str()));
            actor.insert(pair<string, string>("actors[thumbheight]", actHeight.str()));
            actors.push_back(actor);
        }
        loopTokens.insert(pair<string, vector< map< string, string > > >("actors", actors));

    } else if (getType.type == tSeries) {
        cSeries series;
        series.seriesId = getType.seriesId;
        series.episodeId = getType.episodeId;
        pScraper->Service("GetSeries", &series);
        intTokens.insert(pair<string,int>("ismovie", false));
        intTokens.insert(pair<string,int>("isseries", true));
        //Series Basics
        stringTokens.insert(pair<string,string>("seriesname", series.name));
        stringTokens.insert(pair<string,string>("seriesoverview", series.overview));
        stringTokens.insert(pair<string,string>("seriesfirstaired", series.firstAired));
        stringTokens.insert(pair<string,string>("seriesnetwork", series.network));
        stringTokens.insert(pair<string,string>("seriesgenre", series.genre));
        stringstream rating;
        rating << series.rating;
        stringTokens.insert(pair<string,string>("seriesrating", rating.str()));
        stringTokens.insert(pair<string,string>("seriesstatus", series.status));
        //Episode Information
        intTokens.insert(pair<string,int>("episodenumber", series.episode.number));
        intTokens.insert(pair<string,int>("episodeseason", series.episode.season));
        stringTokens.insert(pair<string,string>("episodetitle", series.episode.name));
        stringTokens.insert(pair<string,string>("episodefirstaired", series.episode.firstAired));
        stringTokens.insert(pair<string,string>("episodegueststars", series.episode.guestStars));
        stringTokens.insert(pair<string,string>("episodeoverview", series.episode.overview));
        stringstream eprating;
        eprating << series.episode.rating;
        stringTokens.insert(pair<string,string>("episoderating", eprating.str()));
        intTokens.insert(pair<string,int>("episodeimagewidth", series.episode.episodeImage.width));
        intTokens.insert(pair<string,int>("episodeimageheight", series.episode.episodeImage.height));
        stringTokens.insert(pair<string,string>("episodeimagepath", series.episode.episodeImage.path));
        //Seasonposter
        intTokens.insert(pair<string,int>("seasonposterwidth", series.seasonPoster.width));
        intTokens.insert(pair<string,int>("seasonposterheight", series.seasonPoster.height));
        stringTokens.insert(pair<string,string>("seasonposterpath", series.seasonPoster.path));

        //Posters
        int current = 1;
        for(vector<cTvMedia>::iterator poster = series.posters.begin(); poster != series.posters.end(); poster++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesposter" << current << "width";
            labelHeight << "seriesposter" << current << "height";
            labelPath << "seriesposter" << current << "path";

            intTokens.insert(pair<string,int>(labelWidth.str(), (*poster).width));
            intTokens.insert(pair<string,int>(labelHeight.str(), (*poster).height));
            stringTokens.insert(pair<string,string>(labelPath.str(), (*poster).path));
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesposter" << current << "width";
                labelHeight << "seriesposter" << current << "height";
                labelPath << "seriesposter" << current << "path";
                
                intTokens.insert(pair<string,int>(labelWidth.str(), 0));
                intTokens.insert(pair<string,int>(labelHeight.str(), 0));
                stringTokens.insert(pair<string,string>(labelPath.str(), ""));
            }
        }

        //Banners
        current = 1;
        for(vector<cTvMedia>::iterator banner = series.banners.begin(); banner != series.banners.end(); banner++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesbanner" << current << "width";
            labelHeight << "seriesbanner" << current << "height";
            labelPath << "seriesbanner" << current << "path";
            
            intTokens.insert(pair<string,int>(labelWidth.str(), (*banner).width));
            intTokens.insert(pair<string,int>(labelHeight.str(), (*banner).height));
            stringTokens.insert(pair<string,string>(labelPath.str(), (*banner).path));
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesbanner" << current << "width";
                labelHeight << "seriesbanner" << current << "height";
                labelPath << "seriesbanner" << current << "path";
                
                intTokens.insert(pair<string,int>(labelWidth.str(), 0));
                intTokens.insert(pair<string,int>(labelHeight.str(), 0));
                stringTokens.insert(pair<string,string>(labelPath.str(), ""));
            }
        }
        
        //Fanarts
        current = 1;
        for(vector<cTvMedia>::iterator fanart = series.fanarts.begin(); fanart != series.fanarts.end(); fanart++) {
            stringstream labelWidth, labelHeight, labelPath;
            labelWidth << "seriesfanart" << current << "width";
            labelHeight << "seriesfanart" << current << "height";
            labelPath << "seriesfanart" << current << "path";
            
            intTokens.insert(pair<string,int>(labelWidth.str(), (*fanart).width));
            intTokens.insert(pair<string,int>(labelHeight.str(), (*fanart).height));
            stringTokens.insert(pair<string,string>(labelPath.str(), (*fanart).path));
            current++;
        }
        if (current < 3) {
            for (; current < 4; current++) {
                stringstream labelWidth, labelHeight, labelPath;
                labelWidth << "seriesfanart" << current << "width";
                labelHeight << "seriesfanart" << current << "height";
                labelPath << "seriesfanart" << current << "path";
                
                intTokens.insert(pair<string,int>(labelWidth.str(), 0));
                intTokens.insert(pair<string,int>(labelHeight.str(), 0));
                stringTokens.insert(pair<string,string>(labelPath.str(), ""));
            }
        }
        
        //Actors
       vector< map< string, string > > actors;
        for (vector<cActor>::iterator act = series.actors.begin(); act != series.actors.end(); act++) {
            map< string, string > actor;
            actor.insert(pair<string, string>("actors[name]", (*act).name));
            actor.insert(pair<string, string>("actors[role]", (*act).role));
            actor.insert(pair<string, string>("actors[thumb]", (*act).actorThumb.path));
            stringstream actWidth, actHeight;
            actWidth << (*act).actorThumb.width;
            actHeight << (*act).actorThumb.height;
            actor.insert(pair<string, string>("actors[thumbwidth]", actWidth.str()));
            actor.insert(pair<string, string>("actors[thumbheight]", actHeight.str()));
            actors.push_back(actor);
        }
        loopTokens.insert(pair<string, vector< map< string, string > > >("actors", actors));

    } else {
        intTokens.insert(pair<string,int>("ismovie", false));
        intTokens.insert(pair<string,int>("isseries", false));
    }

}
