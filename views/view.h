#ifndef __VIEW_H
#define __VIEW_H

#include "string"
#include "map"
#include "../libcore/pixmapcontainer.h"
#include "../libtemplate/template.h"

using namespace std;

class cView : public cPixmapContainer {
private:
    void Init(void);
    void DoFill(int num, cTemplateFunction *func);
    void DoDrawText(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void DoDrawTextBox(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void DoDrawFloatingTextBox(int num, cTemplateFunction *func);
    void DoDrawRectangle(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void DoDrawEllipse(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void DoDrawSlope(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void DoDrawImage(int num, cTemplateFunction *func, int x0 = 0, int y0 = 0);
    void ActivateScrolling(void);
protected:
    cTemplateView *tmplView;
    cTemplateViewElement *tmplItem;
    cTemplateViewTab *tmplTab;
    //scaling window
    cRect scalingWindow;
    bool tvScaled;
    bool viewInit;
    //true if view is scrollable in general
    bool scrolling;
    //true if view is actually starting scrolling
    bool currentlyScrolling;
    eViewElement veScroll;
    int scrollingPix;
    int scrollOrientation;
    int scrollDelay;
    int scrollMode;
    int scrollSpeed;
    void DrawViewElement(eViewElement ve, map <string,string> *stringTokens = NULL, map <string,int> *intTokens = NULL, map < string, vector< map< string, string > > > *loopTokens = NULL);
    void ClearViewElement(eViewElement ve);
    bool ViewElementImplemented(eViewElement ve);
    void CreateViewPixmap(int num, cTemplatePixmap *pix, cRect *size = NULL);
    void CreateScrollingPixmap(int num, cTemplatePixmap *pix, cSize &drawportSize);
    void DrawPixmap(int num, cTemplatePixmap *pix, map < string, vector< map< string, string > > > *loopTokens = NULL, bool flushPerLoop = false);
    void DrawLoop(int numPixmap, cTemplateFunction *func, map < string, vector< map< string, string > > > *loopTokens);
    void DebugTokens(string viewElement, map<string,string> *stringTokens, map<string,int> *intTokens, map < string, vector< map< string, string > > > *loopTokens = NULL);
    virtual void Action(void);
public:
    cView(cTemplateView *tmplView);
    cView(cTemplateViewElement *tmplItem);
    cView(cTemplateViewTab *tmplTab);
    virtual ~cView();
    virtual void Stop(void);
};

class cViewListItem : public cView {
protected:
    int pos;
    int numTotal; 
    cRect container;
    int align;
    int listOrientation;
    void SetListElementPosition(cTemplatePixmap *pix);
public:
    cViewListItem(cTemplateViewElement *tmplItem);
    virtual ~cViewListItem();
    cRect DrawListItem(map <string,string> *stringTokens, map <string,int> *intTokens);
    void ClearListItem(void);
};

#endif //__VIEW_H