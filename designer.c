#include "designer.h"
#include "libcore/helpers.h"

cSkinDesigner::cSkinDesigner(string skin) : cSkin(skin.c_str(), &::Theme) {
    init = true;
    this->skin = skin;

    backupSkin = NULL;
    useBackupSkin = false;
    
    globals = NULL;
    channelTemplate = NULL;
    menuTemplate = NULL;
    messageTemplate = NULL;
    replayTemplate = NULL;
    volumeTemplate = NULL;
    audiotracksTemplate = NULL;

    currentMenu = NULL;

    dsyslog("skindesigner: skin %s started", skin.c_str());
}

cSkinDesigner::~cSkinDesigner(void) {
    if (globals)
        delete globals;
    DeleteTemplates();
    if (backupSkin)
        delete backupSkin;
}

const char *cSkinDesigner::Description(void) {
  return skin.c_str();
}

cSkinDisplayChannel *cSkinDesigner::DisplayChannel(bool WithInfo) {
    currentMenu = NULL;
    cSkinDisplayChannel *displayChannel = NULL;
    if (!useBackupSkin) {
        Init();
        displayChannel = new cSDDisplayChannel(channelTemplate, WithInfo);
    } else {
        displayChannel = backupSkin->DisplayChannel(WithInfo);
    }
    return displayChannel;
}

cSkinDisplayMenu *cSkinDesigner::DisplayMenu(void) {
    if (!useBackupSkin) {
        cSDDisplayMenu *displayMenu = NULL;
        Init();
        firstDisplay = false;
        displayMenu = new cSDDisplayMenu(menuTemplate);
        currentMenu = displayMenu;
        return displayMenu;
    } else {
        cSkinDisplayMenu *displayMenu = backupSkin->DisplayMenu();
        currentMenu = NULL;
        return displayMenu;
    }
}

cSkinDisplayReplay *cSkinDesigner::DisplayReplay(bool ModeOnly) {
    currentMenu = NULL;
    cSkinDisplayReplay *displayReplay = NULL;
    if (!useBackupSkin) {
        Init();
        displayReplay = new cSDDisplayReplay(replayTemplate, ModeOnly);
    } else {
        displayReplay = backupSkin->DisplayReplay(ModeOnly);
    }
    return displayReplay;
}

cSkinDisplayVolume *cSkinDesigner::DisplayVolume(void) {
    currentMenu = NULL;
    cSkinDisplayVolume *displayVolume = NULL;
    if (!useBackupSkin) {
        Init();
        displayVolume = new cSDDisplayVolume(volumeTemplate);
    } else {
        displayVolume = backupSkin->DisplayVolume();
    }
    return displayVolume;
}

cSkinDisplayTracks *cSkinDesigner::DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks) {
    currentMenu = NULL;
    cSkinDisplayTracks *displayTracks = NULL;
    if (!useBackupSkin) {
        Init();
        displayTracks = new cSDDisplayTracks(audiotracksTemplate, Title, NumTracks, Tracks);
    } else {
        displayTracks = backupSkin->DisplayTracks(Title, NumTracks, Tracks);
    }
    return displayTracks;
}

cSkinDisplayMessage *cSkinDesigner::DisplayMessage(void) {
    currentMenu = NULL;
    cSkinDisplayMessage *displayMessage = NULL;
    if (!useBackupSkin) {
        Init();
        displayMessage = new cSDDisplayMessage(messageTemplate);
    } else {
        displayMessage = backupSkin->DisplayMessage();
    }
    return displayMessage;
}

void cSkinDesigner::Reload(void) {
    dsyslog("skindesigner: forcing full reload of templates");
    if (cOsd::IsOpen()) {
        esyslog("skindesigner: OSD is open, close first!");
        return;
    }
    DeleteTemplates();
    cStopWatch watch;
    bool ok = LoadTemplates();
    if (!ok) {
        esyslog("skindesigner: error during loading of templates - using LCARS as backup");
        if (!backupSkin)
            backupSkin = new cSkinLCARS();
        useBackupSkin = true;
    } else {
        CacheTemplates();
        useBackupSkin = false;        
        watch.Stop("templates reloaded and cache created");
    }
}

void cSkinDesigner::ListAvailableFonts(void) {
    fontManager->ListAvailableFonts();
}

bool cSkinDesigner::SetCustomToken(string option) {
    splitstring s(option.c_str());
    vector<string> flds = s.split('=', 0);

    if (flds.size() != 2)
        return false;

    string key = trim(flds[0]);
    string val = trim(flds[1]);

    if (!globals)
        return true;

    map<string, string>::iterator hit = globals->customTokens.find(key);
    if (hit != globals->customTokens.end()) {
        globals->customTokens.erase(key);
    }
    globals->customTokens.insert(pair<string,string>(key, val));

    return true;
}

void cSkinDesigner::ListCustomTokens(void) {
    if (!globals)
        return;

    for (map<string, string>::iterator it = globals->customTokens.begin(); it != globals->customTokens.end(); it++) {
        dsyslog("skindesigner: custom token \"%s\" = \"%s\"", (it->first).c_str(), (it->second).c_str());
    }
}

/*********************************************************************************
* PRIVATE FUNCTIONS
*********************************************************************************/    
void cSkinDesigner::Init(void) {
    if (init || config.OsdSizeChanged() || config.SkinChanged() || config.OsdLanguageChanged()) {
        
        if (init) {
            config.SetSkin();
            config.SetOSDSize();
            config.SetOSDFonts();
        }
        dsyslog("skindesigner: initializing skin %s", skin.c_str());
        
        config.CheckDecimalPoint();
        
        if (fontManager)
            delete fontManager;
        fontManager = new cFontManager();
        if (imgCache)
            delete imgCache;
        imgCache = new cImageCache();
        imgCache->SetPathes();

        cStopWatch watch;
        bool ok = LoadTemplates();
        if (!ok) {
            esyslog("skindesigner: error during loading of templates - using LCARS as backup");
            backupSkin = new cSkinLCARS();
            useBackupSkin = true;
        } else {
            CacheTemplates();
            watch.Stop("templates loaded and cache created");
        }
        init = false;
    } else if (config.OsdFontsChanged()) {
        dsyslog("skindesigner: reloading fonts");
        if (fontManager)
            delete fontManager;
        fontManager = new cFontManager();
        cStopWatch watch;
        bool ok = LoadTemplates();
        if (!ok) {
            esyslog("skindesigner: error during loading of templates - using LCARS as backup");
            backupSkin = new cSkinLCARS();
            useBackupSkin = true;
        } else {
            CacheTemplates();
            watch.Stop("templates loaded and cache created");
        }
    }
}

void cSkinDesigner::DeleteTemplates(void) {
    if (channelTemplate) {
        delete channelTemplate;
        channelTemplate = NULL;
    }
    if (menuTemplate) {
        delete menuTemplate;
        menuTemplate = NULL;
    }
    if (messageTemplate) {
        delete messageTemplate;
        messageTemplate = NULL;
    }
    if (replayTemplate) {
        delete replayTemplate;
        replayTemplate = NULL;
    }
    if (volumeTemplate) {
        delete volumeTemplate;
        volumeTemplate = NULL;
    }
    if (audiotracksTemplate) {
        delete audiotracksTemplate;
        audiotracksTemplate = NULL;
    }
}

bool cSkinDesigner::LoadTemplates(void) {
    if (globals)
        delete globals;
    globals = new cGlobals();
    bool ok = globals->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error parsing globals, aborting");
        return false;
    }

    DeleteTemplates();

    channelTemplate = new cTemplate(vtDisplayChannel);
    channelTemplate->SetGlobals(globals);
    ok = channelTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displaychannel template, aborting");
        DeleteTemplates();
        return false;
    }
    channelTemplate->Translate();

    menuTemplate = new cTemplate(vtDisplayMenu);
    menuTemplate->SetGlobals(globals);
    ok = menuTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displaymenu template, aborting");
        DeleteTemplates();
        return false;
    }
    menuTemplate->Translate();

    messageTemplate = new cTemplate(vtDisplayMessage);
    messageTemplate->SetGlobals(globals);
    ok = messageTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displaymessage template, aborting");
        DeleteTemplates();
        return false;
    }
    messageTemplate->Translate();

    replayTemplate = new cTemplate(vtDisplayReplay);
    replayTemplate->SetGlobals(globals);
    ok = replayTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displayreplay template, aborting");
        DeleteTemplates();
        return false;
    }
    replayTemplate->Translate();

    volumeTemplate = new cTemplate(vtDisplayVolume);
    volumeTemplate->SetGlobals(globals);
    ok = volumeTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displayvolume template, aborting");
        DeleteTemplates();
        return false;
    }
    volumeTemplate->Translate();

    audiotracksTemplate = new cTemplate(vtDisplayAudioTracks);
    audiotracksTemplate->SetGlobals(globals);
    ok = audiotracksTemplate->ReadFromXML();
    if (!ok) {
        esyslog("skindesigner: error reading displayaudiotracks template, aborting");
        DeleteTemplates();
        return false;
    }
    audiotracksTemplate->Translate();

    dsyslog("skindesigner: templates successfully validated and parsed");        
    return true;
}

void cSkinDesigner::CacheTemplates(void) {
    channelTemplate->PreCache();
    menuTemplate->PreCache();
    messageTemplate->PreCache();
    replayTemplate->PreCache();
    volumeTemplate->PreCache();
    audiotracksTemplate->PreCache();
    dsyslog("skindesigner: templates cached");
    fontManager->CacheFonts(channelTemplate);
    fontManager->CacheFonts(menuTemplate);
    fontManager->CacheFonts(messageTemplate);
    fontManager->CacheFonts(replayTemplate);
    fontManager->CacheFonts(volumeTemplate);
    fontManager->CacheFonts(audiotracksTemplate);
    dsyslog("skindesigner: fonts cached");
    dsyslog("skindesigner: caching images...");
    imgCache->Clear();
    imgCache->SetPathes();
    channelTemplate->CacheImages();
    menuTemplate->CacheImages();
    messageTemplate->CacheImages();
    replayTemplate->CacheImages();
    volumeTemplate->CacheImages();
    audiotracksTemplate->CacheImages();
    imgCache->Debug(false);
}

