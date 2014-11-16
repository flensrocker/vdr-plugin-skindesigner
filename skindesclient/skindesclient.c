/*
 * skindesclient.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <libskindesigner/services.h>
#include "osdmenu.c"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "SkinDesigner Test Client";
static const char *MAINMENUENTRY  = "Skindesclient";

class cPluginSkindesclient : public cPlugin {
private:
  // Add any member variables or functions you may need here.
public:
  cPluginSkindesclient(void);
  virtual ~cPluginSkindesclient();
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
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginSkindesclient::cPluginSkindesclient(void)
{

}

cPluginSkindesclient::~cPluginSkindesclient()
{
}

const char *cPluginSkindesclient::CommandLineHelp(void)
{
  return NULL;
}

bool cPluginSkindesclient::ProcessArgs(int argc, char *argv[])
{
  return true;
}

bool cPluginSkindesclient::Initialize(void)
{
  return true;
}

bool cPluginSkindesclient::Start(void)
{
  RegisterPlugin reg;
  reg.name = "skindesclient";
  reg.SetMenu(meListMain, "menulistmain.xml");
  reg.SetMenu(meListSub, "menulistsub.xml");
  reg.SetMenu(meDetail, "menudetail.xml");
  static cPlugin *pSkinDesigner = cPluginManager::GetPlugin("skindesigner");
  if (pSkinDesigner) {
    bool ok = pSkinDesigner->Service("RegisterPlugin", &reg);
  } else {
    esyslog("skindesclient: skindesigner not available");
  }
  return true;
}

void cPluginSkindesclient::Stop(void)
{
  // Stop any background activities the plugin is performing.
}

void cPluginSkindesclient::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginSkindesclient::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginSkindesclient::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginSkindesclient::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginSkindesclient::MainMenuAction(void)
{
  cOsdObject *menu = new cPlugOsdMenu();  
  return menu;
}

cMenuSetupPage *cPluginSkindesclient::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginSkindesclient::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginSkindesclient::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginSkindesclient::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginSkindesclient::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

VDRPLUGINCREATOR(cPluginSkindesclient); // Don't touch this!
