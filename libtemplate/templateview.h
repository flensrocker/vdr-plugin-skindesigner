#ifndef __TEMPLATEVIEW_H
#define __TEMPLATEVIEW_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

#include "templateviewelement.h"
#include "templateviewlist.h"
#include "templatepixmap.h"
#include "templateviewtab.h"
#include "templatefunction.h"

using namespace std;

// --- cTemplateView -------------------------------------------------------------

enum eSubView {
    svUndefined,
    svMenuDefault,
    svMenuMain,
    svMenuSetup,
    svMenuSchedules,
    svMenuTimers,
    svMenuRecordings,
    svMenuChannels,
    svMenuDetailedEpg,
    svMenuDetailedRecording,
    svMenuDetailedText,
    svMenuPlugin,
};

class cTemplateView {
private:
protected:
    cGlobals *globals;
    //view parameters
    string viewName;
    cTemplateFunction *parameters;
    int containerX;
    int containerY;
    int containerWidth;
    int containerHeight;
    //basic view data structures
    map < eViewElement, cTemplateViewElement* > viewElements;
    map < eViewList, cTemplateViewList* > viewLists;
    map < eSubView, cTemplateView* > subViews;
    vector< cTemplateViewTab* > viewTabs;
    map < string, map< int, cTemplateView*> > pluginViews;
    //helpers to iterate data structures
    map < eViewElement, cTemplateViewElement* >::iterator veIt;
    map < eViewList, cTemplateViewList* >::iterator vlIt;
    map < eSubView, cTemplateView* >::iterator svIt;
    vector< cTemplateViewTab* >::iterator vtIt;
    //helpers to check valid xml templates
    set<string> subViewsAllowed;
    set<string> viewElementsAllowed;
    set<string> viewListsAllowed;
    map < string, set < string > > funcsAllowed;
    void SetFunctionDefinitions(void);
public:
    cTemplateView(void);
    virtual ~cTemplateView(void);
    virtual string GetSubViewName(eSubView sv) { return ""; };
    virtual string GetViewElementName(eViewElement ve) { return ""; };
    virtual string GetViewListName(eViewList vl) { return ""; };
    virtual void AddSubView(string sSubView, cTemplateView *subView) {};
    virtual void AddPluginView(string plugName, int templNo, cTemplateView *plugView) {};
    virtual void AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {};
    virtual void AddViewList(string sViewList, cTemplateViewList *viewList) {};
    virtual void AddViewTab(cTemplateViewTab *viewTab) {};
    //Setter Functions
    void SetGlobals(cGlobals *globals) { this->globals = globals; };
    void SetParameters(vector<pair<string, string> > &params);
    void SetContainer(int x, int y, int width, int height);
    //access view elements
    cTemplateViewElement *GetViewElement(eViewElement ve);
    void InitViewElementIterator(void);
    cTemplateViewElement *GetNextViewElement(void);
    //access list elements
    cTemplateViewList *GetViewList(eViewList vl);
    void InitViewListIterator(void);
    cTemplateViewList *GetNextViewList(void);
    bool IsListView(void) { return viewLists.size() > 0 ? true : false; };
    //access tabs
    void InitViewTabIterator(void);
    cTemplateViewTab *GetNextViewTab(void);
    //access sub views
    cTemplateView *GetSubView(eSubView sv);
    void InitSubViewIterator(void);
    cTemplateView *GetNextSubView(void);
    //access plugin views
    cTemplateView *GetPluginView(string pluginName, int pluginMenu);
    //Getter Functions
    const char *GetViewName(void) { return viewName.c_str(); };
    int GetNumericParameter(eParamType type);
    cRect GetOsdSize(void);
    int GetNumPixmaps(void);
    int GetNumPixmapsViewElement(eViewElement ve);
    int GetNumListViewMenuItems(void);
    bool GetScalingWindow(cRect &scalingWindow);
    map<string,string> GetCustomTokens(void) { return globals->customTokens; };
    //Checks for parsing template XML files
    bool ValidSubView(const char *subView);
    bool ValidViewElement(const char *viewElement);
    bool ValidViewList(const char *viewList);    
    bool ValidFunction(const char *func);
    bool ValidAttribute(const char *func, const char *att);
    //Caching
    void Translate(void);
    void PreCache(bool isSubview);
    //Debug
    void Debug(void);
};

// --- cTemplateViewChannel -------------------------------------------------------------

class cTemplateViewChannel : public cTemplateView {
private:
    void SetViewElements(void);
    void SetViewLists(void);
public:
    cTemplateViewChannel(void);
    virtual ~cTemplateViewChannel(void);
    string GetViewElementName(eViewElement ve);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
};

// --- cTemplateViewMenu -------------------------------------------------------------

class cTemplateViewMenu : public cTemplateView {
private:
    void SetSubViews(void);
    void SetViewElements(void);
    void SetViewLists(void);
public:
    cTemplateViewMenu(void);
    virtual ~cTemplateViewMenu(void);
    string GetSubViewName(eSubView sv);
    string GetViewElementName(eViewElement ve);
    string GetViewListName(eViewList vl);
    void AddSubView(string sSubView, cTemplateView *subView);
    void AddPluginView(string plugName, int templNo, cTemplateView *plugView);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
    void AddViewList(string sViewList, cTemplateViewList *viewList);
    void AddViewTab(cTemplateViewTab *viewTab);
};

// --- cTemplateViewMessage -------------------------------------------------------------

class cTemplateViewMessage : public cTemplateView {
private:
    void SetViewElements(void);
public:
    cTemplateViewMessage(void);
    virtual ~cTemplateViewMessage(void);
    string GetViewElementName(eViewElement ve);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
};

// --- cTemplateViewReplay -------------------------------------------------------------

class cTemplateViewReplay : public cTemplateView {
private:
    void SetViewElements(void);
public:
    cTemplateViewReplay(void);
    virtual ~cTemplateViewReplay(void);
    string GetViewElementName(eViewElement ve);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
};

// --- cTemplateViewVolume -------------------------------------------------------------

class cTemplateViewVolume : public cTemplateView {
private:
    void SetViewElements(void);
public:
    cTemplateViewVolume(void);
    virtual ~cTemplateViewVolume(void);
    string GetViewElementName(eViewElement ve);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
};

// --- cTemplateViewAudioTracks -------------------------------------------------------------

class cTemplateViewAudioTracks : public cTemplateView {
private:
    void SetViewElements(void);
    void SetViewLists(void);
public:
    cTemplateViewAudioTracks(void);
    virtual ~cTemplateViewAudioTracks(void);
    string GetViewElementName(eViewElement ve);
    string GetViewListName(eViewList vl);
    void AddPixmap(string viewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes);
    void AddViewList(string sViewList, cTemplateViewList *viewList);
};

#endif //__TEMPLATEVIEW_H
