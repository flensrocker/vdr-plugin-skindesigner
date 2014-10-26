#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include "imagecache.h"
#include "../config.h"
#include "helpers.h"

using namespace Magick;

cMutex cImageCache::mutex;

string cImageCache::items[16] = { "Schedule", "Channels", "Timers", "Recordings", "Setup", "Commands",
                   "OSD", "EPG", "DVB", "LNB", "CAM", "Recording", "Replay", "Miscellaneous", "Plugins", "Restart"};

cImageCache::cImageCache() : cImageMagickWrapper() {
    tempStaticLogo = NULL;
}

cImageCache::~cImageCache() {
    Clear();
    if (tempStaticLogo) {
        delete tempStaticLogo;
        tempStaticLogo = NULL;
    }
}

void cImageCache::SetPathes(void) {
    string logoPathSkin = *cString::sprintf("%s%s/themes/%s/logos/", *config.skinPath, Setup.OSDSkin, Setup.OSDTheme);
    if (FolderExists(logoPathSkin)) {
        logoPath = logoPathSkin;
    } else {
        logoPath = *config.logoPath;
    }
    iconPath = *cString::sprintf("%s%s/themes/%s/", *config.skinPath, Setup.OSDSkin, Setup.OSDTheme);
    skinPartsPath = *cString::sprintf("%s%s/themes/%s/skinparts/", *config.skinPath, Setup.OSDSkin, Setup.OSDTheme);

    dsyslog("skindesigner: using channel logo path %s", logoPath.c_str());
    dsyslog("skindesigner: using icon path %s", iconPath.c_str());
    dsyslog("skindesigner: using skinparts path %s", skinPartsPath.c_str());
}

void cImageCache::CacheLogo(int width, int height) {
    if (config.numLogosPerSizeInitial == 0)
        return;
    if (width == 0 || height == 0)
        return;
    
    int channelsCached = 0;     
    
    for (const cChannel *channel = Channels.First(); channel; channel = Channels.Next(channel)) {
        if (channelsCached >= config.numLogosPerSizeInitial)
            break;
        if (channel->GroupSep()) {
            continue;
        }
        bool success = LoadLogo(channel);
        if (success) {
            channelsCached++;
            cImage *image = CreateImage(width, height);
            stringstream logoName;
            logoName << *channel->GetChannelID().ToString() << "_" << width << "x" << height;
            std::map<std::string, cImage*>::iterator hit = channelLogoCache.find(logoName.str());
            if (hit != channelLogoCache.end()) {
                delete image;
                return;
            }
            channelLogoCache.insert(pair<string, cImage*>(logoName.str(), image));
        }
    }
}

cImage *cImageCache::GetLogo(string channelID, int width, int height) {
    cMutexLock MutexLock(&mutex);

    stringstream logoName;
    logoName << channelID << "_" << width << "x" << height;

    std::map<std::string, cImage*>::iterator hit = channelLogoCache.find(logoName.str());

    if (hit != channelLogoCache.end()) {
        return (cImage*)hit->second;
    } else {
        tChannelID chanID = tChannelID::FromString(channelID.c_str());
        const cChannel *channel = Channels.GetByChannelID(chanID);
        if (!channel)
            return NULL;
        bool success = LoadLogo(channel);
        if (success) {
            if (config.limitLogoCache && (channelLogoCache.size() >= config.numLogosMax)) {
                //logo cache is full, don't cache anymore
                if (tempStaticLogo) {
                    delete tempStaticLogo;
                    tempStaticLogo = NULL;
                }
                tempStaticLogo = CreateImage(width, height);
                return tempStaticLogo;
            } else {
                //add requested logo to cache
                cImage *image = CreateImage(width, height);
                channelLogoCache.insert(pair<string, cImage*>(logoName.str(), image));
                hit = channelLogoCache.find(logoName.str());
                if (hit != channelLogoCache.end()) {
                    return (cImage*)hit->second;
                }
            }
        }
    }
    return NULL;
}

cImage *cImageCache::GetSeparatorLogo(string name, int width, int height) {
    cMutexLock MutexLock(&mutex);

    stringstream logoName;
    logoName << name << "_" << width << "x" << height;

    std::map<std::string, cImage*>::iterator hit = channelLogoCache.find(logoName.str());

    if (hit != channelLogoCache.end()) {
        return (cImage*)hit->second;
    } else {
        bool success = LoadSeparatorLogo(name);
        if (success) {
            //add requested logo to cache
            cImage *image = CreateImage(width, height);
            channelLogoCache.insert(pair<string, cImage*>(logoName.str(), image));
            hit = channelLogoCache.find(logoName.str());
            if (hit != channelLogoCache.end()) {
                return (cImage*)hit->second;
            }
        }
    }
    return NULL;
}

bool cImageCache::LogoExists(string channelID) {
    tChannelID chanID = tChannelID::FromString(channelID.c_str());
    const cChannel *channel = Channels.GetByChannelID(chanID);
    if (!channel)
        return false;
    string logoLower = StrToLowerCase(channel->Name());
    string logoExt = *config.logoExtension;
    bool logoExists = FileExists(logoPath.c_str(), logoLower, logoExt);
    if (logoExists) {
        return true;
    }
    logoExists = FileExists(logoPath.c_str(), channelID, logoExt);
    if (logoExists) {
        return true;
    }
    return false;    
}

bool cImageCache::SeparatorLogoExists(string name) {
    string separatorPath = *cString::sprintf("%sseparatorlogos/", logoPath.c_str());
    string nameLower = StrToLowerCase(name.c_str());
    string logoExt = *config.logoExtension;
    bool logoExists = FileExists(separatorPath, nameLower, logoExt);
    if (logoExists) {
        return true;
    }
    return false;    
}

void cImageCache::CacheIcon(eImageType type, string name, int width, int height) {
    if (width < 1 || width > 1920 || height < 1 || height > 1080)
        return;
    bool success = LoadIcon(type, name);
    if (!success) 
        return;
    stringstream iconName;
    iconName << name << "_" << width << "x" << height;
    cImage *image = CreateImage(width, height, true);
    iconCache.insert(pair<string, cImage*>(iconName.str(), image));
}

cImage *cImageCache::GetIcon(eImageType type, string name, int width, int height) {
    if (width < 1 || width > 1920 || height < 1 || height > 1080)
        return NULL;
    cMutexLock MutexLock(&mutex);
    stringstream iconName;
    iconName << name << "_" << width << "x" << height;
    map<string, cImage*>::iterator hit = iconCache.find(iconName.str());
    if (hit != iconCache.end()) {
        return (cImage*)hit->second;
    } else {
        bool success = LoadIcon(type, name);
        if (!success) 
            return NULL;
        cImage *image = CreateImage(width, height, true);
        iconCache.insert(pair<string, cImage*>(iconName.str(), image));
        hit = iconCache.find(iconName.str());
        if (hit != iconCache.end()) {
            return (cImage*)hit->second;
        }
    }
    return NULL;    
}

string cImageCache::GetIconName(string label, eMenuCategory cat) {
    //if cat is set, use standard menu entries
    switch (cat) {
        case mcSchedule:
        case mcScheduleNow:
        case mcScheduleNext:
        case mcEvent:
            return "standardicons/Schedule";
        case mcChannel:
        case mcChannelEdit:
            return "standardicons/Channels";
        case mcTimer:
        case mcTimerEdit:
            return "standardicons/Timers";
        case mcRecording:
        case mcRecordingInfo:
        case mcSetupRecord:
        case mcSetupReplay:
            return "standardicons/Recordings";
        case mcPlugin:
        case mcPluginSetup:
        case mcSetupPlugins:
            return "standardicons/Plugins";
        case mcSetup:
            return "standardicons/Setup";
        case mcSetupOsd:
            return "standardicons/OSD";
        case mcSetupEpg:
            return "standardicons/EPG";
        case mcSetupDvb:
            return "standardicons/DVB";
        case mcSetupLnb:
            return "standardicons/LNB";
        case mcSetupCam:
            return "standardicons/CAM";
        case mcSetupMisc:
            return "standardicons/Miscellaneous";    
        case mcCommand:
            return "standardicons/Commands";
    }
    //check for standard menu entries
    for (int i=0; i<16; i++) {
        string s = trVDR(items[i].c_str());
        if (s == label) {
            return *cString::sprintf("standardicons/%s", items[i].c_str());
        }
    }
    //check for special main menu entries "stop recording", "stop replay"
    string stopRecording = skipspace(trVDR(" Stop recording "));
    string stopReplay = skipspace(trVDR(" Stop replaying"));
    try {
        if (label.substr(0, stopRecording.size()) == stopRecording) {
            return "standardicons/StopRecording";
        }
        if (label.substr(0, stopReplay.size()) == stopReplay) {
            return "standardicons/StopReplay";
        }
    } catch (...) {}
    //check for Plugins
    for (int i = 0; ; i++) {
        cPlugin *p = cPluginManager::GetPlugin(i);
        if (p) {
            const char *mainMenuEntry = p->MainMenuEntry();
            if (mainMenuEntry) {
                string plugMainEntry = mainMenuEntry;
                try {
                    if (label.substr(0, plugMainEntry.size()) == plugMainEntry) {
                        return *cString::sprintf("pluginicons/%s", p->Name());
                    }
                } catch (...) {}
            }
        } else
            break;
    }
    return *cString::sprintf("customicons/%s", label.c_str());
}

void cImageCache::CacheSkinpart(string name, int width, int height) {
    if (width < 1 || width > 1920 || height < 1 || height > 1080)
        return;
    bool success = LoadSkinpart(name);
    if (!success) 
        return;
    stringstream iconName;
    iconName << name << "_" << width << "x" << height;
    cImage *image = CreateImage(width, height, false);
    skinPartsCache.insert(pair<string, cImage*>(iconName.str(), image));
}

cImage *cImageCache::GetSkinpart(string name, int width, int height) {
    if (width < 1 || width > 1920 || height < 1 || height > 1080)
        return NULL;
    cMutexLock MutexLock(&mutex);
    stringstream iconName;
    iconName << name << "_" << width << "x" << height;
    map<string, cImage*>::iterator hit = skinPartsCache.find(iconName.str());
    if (hit != skinPartsCache.end()) {
        return (cImage*)hit->second;
    } else {
        bool success = LoadSkinpart(name);
        if (!success) 
            return NULL;
        cImage *image = CreateImage(width, height, false);
        skinPartsCache.insert(pair<string, cImage*>(iconName.str(), image));
        hit = skinPartsCache.find(iconName.str());
        if (hit != skinPartsCache.end()) {
            return (cImage*)hit->second;
        }
    }
    return NULL;    
}

bool cImageCache::LoadIcon(eImageType type, string name) {
    bool success = false;
    cString subdir("");
    if (type == itMenuIcon)
        subdir = "menuicons";
    else if (type == itIcon)
        subdir = "icons";
    cString subIconPath = cString::sprintf("%s%s/", iconPath.c_str(), *subdir);
    success = LoadImage(name, *subIconPath, "png");
    if (success) {
        return true;
    }
    return false;
}

bool cImageCache::LoadLogo(const cChannel *channel) {
    if (!channel)
        return false;
    string channelID = StrToLowerCase(*(channel->GetChannelID().ToString()));
    string logoLower = StrToLowerCase(channel->Name());
    bool success = false;
    success = LoadImage(channelID.c_str(), logoPath.c_str(), *config.logoExtension);
    if (success)
        return true;
    success = LoadImage(logoLower.c_str(), logoPath.c_str(), *config.logoExtension);
    if (success)
        return true;
    return false;
}

bool cImageCache::LoadSeparatorLogo(string name) {
    cString separatorPath = cString::sprintf("%sseparatorlogos/", logoPath.c_str());
    string nameLower = StrToLowerCase(name.c_str());
    bool success = false;
    success = LoadImage(nameLower.c_str(), *separatorPath, *config.logoExtension);
    if (success)
        return true;
    return false;    
}

bool cImageCache::LoadSkinpart(string name) {
    bool success = false;
    success = LoadImage(name, skinPartsPath.c_str(), "png");
    if (success) {
        return true;
    }
    return false;
}

void cImageCache::Clear(void) {
    for(map<string, cImage*>::const_iterator it = iconCache.begin(); it != iconCache.end(); it++) {
        cImage *img = (cImage*)it->second;
        delete img;
    }
    iconCache.clear();

    for(map<string, cImage*>::const_iterator it = channelLogoCache.begin(); it != channelLogoCache.end(); it++) {
        cImage *img = (cImage*)it->second;
        delete img;
    }
    channelLogoCache.clear();

    for(map<std::string, cImage*>::const_iterator it = skinPartsCache.begin(); it != skinPartsCache.end(); it++) {
        cImage *img = (cImage*)it->second;
        delete img;
    }
    skinPartsCache.clear();
}

void cImageCache::Debug(bool full) {
    int sizeIconCache = 0;
    int numIcons = 0;
    GetIconCacheSize(numIcons, sizeIconCache);
    dsyslog("skindesigner: cached %d icons - size %d byte", numIcons, sizeIconCache);
    if (full) {
        for(std::map<std::string, cImage*>::const_iterator it = iconCache.begin(); it != iconCache.end(); it++) {
            string name = it->first;
            dsyslog("skindesigner: cached icon %s", name.c_str());
        }
    }

    int sizeLogoCache = 0;
    int numLogos = 0;
    GetLogoCacheSize(numLogos, sizeLogoCache);
    dsyslog("skindesigner: cached %d logos - size %d byte", numLogos, sizeLogoCache);
    if (full) {
        for(std::map<std::string, cImage*>::const_iterator it = channelLogoCache.begin(); it != channelLogoCache.end(); it++) {
            string name = it->first;
            dsyslog("skindesigner: cached logo %s", name.c_str());
        }
    }

    int sizeSkinpartCache = 0;
    int numSkinparts = 0;
    GetSkinpartsCacheSize(numSkinparts, sizeSkinpartCache);
    dsyslog("skindesigner: cached %d skinparts - size %d byte", numSkinparts, sizeSkinpartCache);
    if (full) {
        for(std::map<std::string, cImage*>::const_iterator it = skinPartsCache.begin(); it != skinPartsCache.end(); it++) {
            string name = it->first;
            dsyslog("skindesigner: cached skinpart %s", name.c_str());
        }
    }
}

void cImageCache::GetIconCacheSize(int &num, int &size) {
    num = iconCache.size();
    for (map<string, cImage*>::iterator icon = iconCache.begin(); icon != iconCache.end(); icon++) {
        cImage* img = icon->second;
        size += img->Width() * img->Height() * sizeof(tColor);
    }
}

void cImageCache::GetLogoCacheSize(int &num, int &size) {
    num = channelLogoCache.size();
    for (map<string, cImage*>::iterator logo = channelLogoCache.begin(); logo != channelLogoCache.end(); logo++) {
        cImage* img = logo->second;
        size += img->Width() * img->Height() * sizeof(tColor);
    }
}

void cImageCache::GetSkinpartsCacheSize(int &num, int &size) {
    num = skinPartsCache.size();
    for (map<string, cImage*>::iterator skinpart = skinPartsCache.begin(); skinpart != skinPartsCache.end(); skinpart++) {
        cImage* img = skinpart->second;
        size += img->Width() * img->Height() * sizeof(tColor);
    }    
}
