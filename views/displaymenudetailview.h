#ifndef __DISPLAYMENUDETAILVIEW_H
#define __DISPLAYMENUDETAILVIEW_H

#include <list>
#include "../libtemplate/template.h"
#include "view.h"
#include "viewhelpers.h"
#include "displaymenutabview.h"

class cDisplayMenuDetailView : public cView, public cViewHelpers {
private:
    bool detailViewInit;
    bool isPluginTextView;
    const cEvent *event;
    const cRecording *recording;
    const char *text;
    cTemplateViewTab *currentTmplTab;
    list<cTemplateViewTab*> activeTabs;
    list<cTemplateViewTab*>::iterator atIt;
    cDisplayMenuTabView *tabView;
    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > loopTokens;
    void SetTokens(void);
    void InitTabs(void);
    bool LoadReruns(vector< map< string, string > > *reruns);
    void LoadRecordingInformation(void);
    void SetEpgPictures(int eventId);
    void SetRecordingImages(const char *recPath);
    string StripXmlTag(string &Line, const char *Tag);
    int ReadSizeVdr(const char *strPath);
    void DrawHeader(void);
    void DrawScrollbar(void);
    void DrawTabLabels(void);
    cTemplateViewTab *GetPrevTab(void);
    cTemplateViewTab *GetNextTab(void);
public:
    cDisplayMenuDetailView(cTemplateView *tmplDetailView);
    virtual ~cDisplayMenuDetailView();
    void SetEvent(const cEvent *event) { this->event = event; };
    void SetRecording(const cRecording *recording) { this->recording = recording; };
    void SetText(const char *text) { this->text = text; };
    void SetPluginTokens(map<string,string> *plugStringTokens, map<string,int> *plugIntTokens, map<string,vector<map<string,string> > > *plugLoopTokens);
    void Clear(void);
    void Render(void);
    void KeyLeft(void);
    void KeyRight(void);
    void KeyUp(void);
    void KeyDown(void);
};

#endif //__DISPLAYMENUDETAILVIEW_H
