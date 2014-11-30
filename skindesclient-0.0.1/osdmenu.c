#include <vdr/osdbase.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include "libskindesigner/skindesignerosdbase.h"

enum eMenus {
    meListMain,
    meListSub,
    meDetail
};

class cPlugOsdMenu : public cSkindesignerOsdMenu {
private:
    void SetMenu(int numItems, bool subfolder = false);
    void SetDetailView(int element);
public:
    cPlugOsdMenu(void);
    virtual ~cPlugOsdMenu();
    virtual eOSState ProcessKey(eKeys key);
};


//***************************************************************************
// Public Functions
//***************************************************************************

cPlugOsdMenu::cPlugOsdMenu(void) : cSkindesignerOsdMenu("Skindesigner Client") {
    SetPluginName("skindesclient");
    SetMenu(10);
}

cPlugOsdMenu::~cPlugOsdMenu(void) {
    
}

eOSState cPlugOsdMenu::ProcessKey(eKeys key) {
    eOSState state = cOsdMenu::ProcessKey(key);
    switch (key) {
        case kOk: {
            int element = Current();
            if (element%2)
                SetDetailView(element);
            else
                SetMenu(25, true);
            state = osContinue;
            break;
        } case kLeft: {
            TextKeyLeft();
            state = osContinue;
            break;
        } case kRight: {
            TextKeyRight();
            state = osContinue;            
            break;
        } case kUp: {
            TextKeyUp();
            state = osContinue;            
            break;
        } case kDown: {
            TextKeyDown();
            state = osContinue;            
            break;
        }
        default:
            break;
    }
    return state;
}

//***************************************************************************
// Private Functions
//***************************************************************************

void cPlugOsdMenu::SetMenu(int numItems, bool subfolder) {
    eMenus menu = subfolder ? meListSub : meListMain;
    SetPluginMenu(menu, mtList);
    Clear();

    for (int i=0; i < numItems; i++) {
        cSkindesignerOsdItem *item = new cSkindesignerOsdItem();
        //add some tokens to the menu item
        stringstream text;
        if (i%2)
            text << "DetailItem" << (i+1);
        else
            text << "FolderItem" << (i+1);
        item->SetText(text.str().c_str());
        item->AddIntToken("itemnumber", i);
        item->AddStringToken("menuitemtext", text.str().c_str());

        stringstream text2;
        text2 << "CurrentItemText" << (i+1) << "\n";
        text2 << "CurrentItemText" << (i+1) << "\n";
        text2 << "CurrentItemText" << (i+1) << "\n";
        text2 << "CurrentItemText" << (i+1) << "\n";
        text2 << "CurrentItemText" << (i+1) << "\n";
        text2 << "CurrentItemText" << (i+1) << "\n";
        item->AddStringToken("currentitemtext", text2.str().c_str());
        
        //Loop Token Example
        for (int row=0; row<20; row++) {
            map<string, string> tokens;
            for (int col=0; col<3; col++) {
                stringstream key;
                stringstream value;
                key << "loop1[" << "col" << col << "]";
                value << "menuitem" << i << "-" << row << "x" << col;
                tokens.insert(pair<string,string>(key.str(), value.str()));
            }
            item->AddLoopToken("loop1", tokens);
        }
        //Add item
        bool current = (i==0)?true:false;
        Add(item, current);
    }
    SetHelp("Red", "Green", "Yellow", "Blue");
    Display();
}

void cPlugOsdMenu::SetDetailView(int element) {
    SetPluginMenu(meDetail, mtText);
    Clear();
    ClearTokens();
    
    SetText("Text to be displayed if skindesigner templates are not available");

    AddIntToken("menuitem", element);
    AddStringToken("tabtext", "String Token to be displayed if skindesigner template is available");

    //Loop Token Example
    for (int row=0; row<25; row++) {
        map<string, string> tokens;
        for (int col=0; col<10; col++) {
            stringstream key;
            stringstream value;
            key << "loop1[" << "col" << col << "]";
            value << "row" << row << "-" << "col" << "-" << col;
            tokens.insert(pair<string,string>(key.str(), value.str()));
        }
        AddLoopToken("loop1", tokens);
    }

    SetHelp("Red", "Green", "Yellow", "Blue");
    Display();
}
