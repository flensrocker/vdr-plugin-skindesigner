/*
 * skindesigner.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#include <getopt.h>
#include <vdr/plugin.h>

#include <libskindesigner/services.h>

#define DEFINE_CONFIG 1
#include "config.h"
#include "designer.h"
#include "setup.h"

#if defined(APIVERSNUM) && APIVERSNUM < 20000 
#error "VDR-2.0.0 API version or greater is required!"
#endif


static const char *VERSION        = "0.0.5";
static const char *DESCRIPTION    = "SkinDesigner";
static const char *MAINMENUENTRY  = "Skin Designer";

class cPluginSkinDesigner : public cPlugin {
private:
    vector<cSkinDesigner*> skins;
public:
    cPluginSkinDesigner(void);
    virtual ~cPluginSkinDesigner();
    virtual const char *Version(void) { return VERSION; }
    virtual const char *Description(void) { return DESCRIPTION; }
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int argc, char *argv[]);
    virtual bool Initialize(void);
    virtual bool Start(void);
    virtual void Stop(void);
    virtual void Housekeeping(void);
    virtual void MainThreadHook(void);
    virtual cString Active(void);
    virtual time_t WakeupTime(void);
    virtual const char *MainMenuEntry(void) {return NULL;}
    virtual cOsdObject *MainMenuAction(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *Name, const char *Value);
    virtual bool Service(const char *Id, void *Data = NULL);
    virtual const char **SVDRPHelpPages(void);
    virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
};

cPluginSkinDesigner::cPluginSkinDesigner(void) {
}

cPluginSkinDesigner::~cPluginSkinDesigner() {
}

const char *cPluginSkinDesigner::CommandLineHelp(void) {
  return
         "  -s <SKINPATH>, --skinpath=<SKINPATH> Set directory where xml skins are stored\n"
         "  -l <LOGOPATH>, --logopath=<LOGOPATH> Set directory where a common logo set for all skins is stored\n"
         "  -e <EPGIMAGESPATH>, --epgimages=<IMAGESPATH> Set directory where epgimages are stored\n";
}

bool cPluginSkinDesigner::ProcessArgs(int argc, char *argv[]) {
  // Implement command line argument processing here if applicable.
    static const struct option long_options[] = {
        { "epgimages", required_argument, NULL, 'e' },
        { "logopath", required_argument, NULL, 'l' },
        { "skinpath", required_argument, NULL, 's' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = getopt_long(argc, argv, "e:s:l:", long_options, NULL)) != -1) {
        switch (c) {
            case 'e':
                config.SetEpgImagePath(cString(optarg));
                break;
            case 'l':
                config.SetLogoPath(cString(optarg));
                break;
            case 's':
                config.SetSkinPath(cString(optarg));
                break;
            default:
                return false;
        }
    }
    return true;
}

bool cPluginSkinDesigner::Initialize(void) {
    return true;
}

bool cPluginSkinDesigner::Start(void) {
    cXmlParser::InitLibXML();
    bool trueColorAvailable = true;
    if (!cOsdProvider::SupportsTrueColor()) {
        esyslog("skindesigner: No TrueColor OSD found! Using default Skin LCARS!");
        trueColorAvailable = false;
    } else
        dsyslog("skindesigner: TrueColor OSD found");
    config.SetPathes();
    config.ReadSkins();
    config.InitSkinIterator();
    string skin = "";
    while (config.GetSkin(skin)) {
        cSkinDesigner *newSkin = new cSkinDesigner(skin);
        skins.push_back(newSkin);
        if (!trueColorAvailable) {
            newSkin->ActivateBackupSkin();
        }
    }
    if (skins.size() == 0) {
        esyslog("skindesigner: no skins found! Using default Skin LCARS!");
    }
    return true;
}

void cPluginSkinDesigner::Stop(void) {
    delete imgCache;
    delete fontManager;
    cXmlParser::CleanupLibXML();
}

void cPluginSkinDesigner::Housekeeping(void) {
}

void cPluginSkinDesigner::MainThreadHook(void) {
}

cString cPluginSkinDesigner::Active(void) {
    return NULL;
}

time_t cPluginSkinDesigner::WakeupTime(void) {
    return 0;
}

cOsdObject *cPluginSkinDesigner::MainMenuAction(void) {
    return NULL;
}

cMenuSetupPage *cPluginSkinDesigner::SetupMenu(void) {
    return new cSkinDesignerSetup();
}

bool cPluginSkinDesigner::SetupParse(const char *Name, const char *Value) {
    return config.SetupParse(Name, Value);
}

bool cPluginSkinDesigner::Service(const char *Id, void *Data) {
    if (Data == NULL)
        return false;

    if (strcmp(Id, "RegisterPlugin") == 0) {
        libskindesigner::RegisterPlugin* call = (libskindesigner::RegisterPlugin*) Data;
        if (call->menus.size() < 1) {
            esyslog("skindesigner: error - plugin without menus registered");
            return false;
        }
        config.AddPlugin(call->name, call->menus);
        dsyslog("skindesigner: plugin %s has registered %d templates", call->name.c_str(), call->menus.size());
        return true;
    } else if (strcmp(Id, "GetDisplayMenu") == 0) {
        libskindesigner::GetDisplayMenu* call = (libskindesigner::GetDisplayMenu*) Data;
        cSkin *current = Skins.Current();
        for (vector<cSkinDesigner*>::iterator skin = skins.begin(); skin != skins.end(); skin++) {
            if (*skin == current) {
                cSDDisplayMenu *displayMenu = (*skin)->GetDisplayMenu();
                if (displayMenu) {
                    call->displayMenu = displayMenu;
                    return true;
                } else
                    return false;
            }
        }
        return false;
    }

    return false;
}

const char **cPluginSkinDesigner::SVDRPHelpPages(void) {
    static const char *HelpPages[] = {
        "RELD\n"
        "    force reload of templates and caches",
        "SCTK\n"
        "    Set custom Token name = value",
        "LCTK\n"
        "    List custom Tokens",
        "LSTF\n"
        "    List available Fonts",
        0
    };
    return HelpPages;
}

cString cPluginSkinDesigner::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode) {
    
    cSkinDesigner *activeSkin = NULL;
    for (vector<cSkinDesigner*>::iterator skin = skins.begin(); skin != skins.end(); skin++) {
        string activeSkinName = Setup.OSDSkin;
        string currentSkinName = (*skin)->Description();
        if (!currentSkinName.compare(activeSkinName)) {
            activeSkin = *skin;
            break;
        }
    }

    if (!activeSkin) {
        ReplyCode = 550;
        return "";
    }

    if (strcasecmp(Command, "RELD") == 0) {
        activeSkin->Reload();
        ReplyCode = 250;
        return "SKINDESIGNER reload of templates and caches forced.";
    } else if (strcasecmp(Command, "LSTF") == 0) {
        activeSkin->ListAvailableFonts();
        ReplyCode = 250;
        return "SKINDESIGNER available fonts listed in syslog.";
    } else if (strcasecmp(Command, "SCTK") == 0) {
        if (!Option) {
            ReplyCode = 501;
            return "SKINDESIGNER SCTK Error: no Token name = value set";
        }
        bool optionOk = activeSkin->SetCustomToken(Option);
        if (optionOk) {
            ReplyCode = 250;
            return cString::sprintf("SKINDESIGNER Set custom Token %s", Option);
        } else {
            ReplyCode = 501;
            return cString::sprintf("SKINDESIGNER Invalid custom Token %s", Option);
        }
    } else if (strcasecmp(Command, "LCTK") == 0) {
        activeSkin->ListCustomTokens();
        ReplyCode = 250;
        return "SKINDESIGNER Custom Tokens listed in Log";
    }
    ReplyCode = 502;
    return "";
}

VDRPLUGINCREATOR(cPluginSkinDesigner); // Don't touch this!
