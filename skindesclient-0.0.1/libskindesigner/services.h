#ifndef __SKINDESIGNERSERVICES_H
#define __SKINDESIGNERSERVICES_H

using namespace std;

#include <string>
#include <map>

enum eMenuType {
    mtList,
    mtText
};

class cSDDisplayMenu : public cSkinDisplayMenu {
public:
    virtual void SetPluginMenu(string name, int menu, int type, bool init);
    virtual bool SetItemPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens, int Index, bool Current, bool Selectable);
    virtual bool SetPluginText(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens);
};

/*********************************************************************
* Data Structures for Service Calls
*********************************************************************/

// Data structure for service "RegisterPlugin"
class RegisterPlugin {
public:
    RegisterPlugin(void) {
        name = "";
    };
    void SetMenu(int key, string templateName) {
        menus.insert(pair<int, string>(key, templateName));
    }
// in
    string name;                 //name of plugin
    map< int, string > menus;    //menus as key -> templatename hashmap 
//out
};

// Data structure for service "GetDisplayMenu"
class GetDisplayMenu {
public:
    GetDisplayMenu(void) {
        displayMenu = NULL;
    };
// in
//out   
    cSDDisplayMenu *displayMenu;
};
#endif //__SKINDESIGNERSERVICES_H