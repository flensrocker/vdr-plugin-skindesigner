#ifndef __SKINDESIGNERSERVICES_H
#define __SKINDESIGNERSERVICES_H

using namespace std;

#include <string>
#include <map>
#include <vector>
#include <vdr/osdbase.h>

namespace libskindesigner {

enum eMenuType {
    mtList,
    mtText
};

class ISDDisplayMenu : public cSkinDisplayMenu {
public:
    virtual void SetPluginMenu(string name, int menu, int type, bool init) = 0;
    virtual bool SetItemPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens, int Index, bool Current, bool Selectable) = 0;
    virtual bool SetPluginText(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens) = 0;
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
	string name;				 //name of plugin
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
	ISDDisplayMenu *displayMenu;
};

}

#endif //__SKINDESIGNERSERVICES_H
