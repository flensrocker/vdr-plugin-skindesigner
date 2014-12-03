#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

#include "globals.h"
#include "templateview.h"
#include "templateviewelement.h"
#include "templatepixmap.h"
#include "templatefunction.h"

using namespace std;

// --- cTemplate -------------------------------------------------------------
enum eViewType {
    vtDisplayChannel,
    vtDisplayMenu,
    vtDisplayReplay,
    vtDisplayVolume,
    vtDisplayAudioTracks,
    vtDisplayMessage
};

class cTemplate {
private:
    eViewType viewType;
    void CachePixmapImages(cTemplatePixmap *pix);
    void CacheImage(cTemplateFunction *func);
    void CacheEllipse(cTemplateFunction *func);
protected:
    cGlobals *globals;
    cTemplateView *rootView;
    void CreateView(void);
    void GetUsedFonts(cTemplateView *view, vector< pair<string, int> > &usedFonts);
    void CacheImages(cTemplateView *view);
public:
    cTemplate(eViewType viewType);
    virtual ~cTemplate(void);
    bool ReadFromXML(void);
    void SetGlobals(cGlobals *globals);
    cTemplateView *GetRootView(void) { return rootView; };
    void Translate(void);
    void PreCache(void);
    //get fonts for pre caching
    vector< pair<string, int> > GetUsedFonts(void);
    void CacheImages(void);
    //Debug
    void Debug(void);
};

#endif //__TEMPLATE_H