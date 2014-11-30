#include "templateview.h"

// --- cTemplateView -------------------------------------------------------------

cTemplateView::cTemplateView(void) {
    globals = NULL;
    parameters = NULL;
    containerX = 0;
    containerY = 0;
    containerWidth = 0;
    containerHeight = 0;
    SetFunctionDefinitions();
}

cTemplateView::~cTemplateView() {

    for (map < eViewElement, cTemplateViewElement* >::iterator it = viewElements.begin(); it != viewElements.end(); it++) {
        delete it->second;
    }

    for (map < eViewList, cTemplateViewList* >::iterator it = viewLists.begin(); it != viewLists.end(); it++) {
        delete it->second;
    }

    for (vector < cTemplateViewTab* >::iterator it = viewTabs.begin(); it != viewTabs.end(); it++) {
        delete *it;
    }

    for (map < eSubView, cTemplateView* >::iterator it = subViews.begin(); it != subViews.end(); it++) {
        delete it->second;
    }

    for (map < string, map< int, cTemplateView*> >::iterator it = pluginViews.begin(); it != pluginViews.end(); it++) {
        map< int, cTemplateView*> plugViews = it->second;
        for (map< int, cTemplateView*>::iterator it2 = plugViews.begin(); it2 != plugViews.end(); it2++) {
            delete it2->second;
        }
    }

    if (parameters)
        delete parameters;

}

/*******************************************************************
* Public Functions
*******************************************************************/

void cTemplateView::SetParameters(vector<pair<string, string> > &params) {
    parameters = new cTemplateFunction(ftView);
    parameters->SetGlobals(globals);
    parameters->SetParameters(params);
}

void cTemplateView::SetContainer(int x, int y, int width, int height) {
    containerX = x;
    containerY = y;
    containerWidth = width; 
    containerHeight = height; 
}

cTemplateViewElement *cTemplateView::GetViewElement(eViewElement ve) {
    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end())
        return NULL;
    return hit->second;
}

void cTemplateView::InitViewElementIterator(void) {
    veIt = viewElements.begin();
}

cTemplateViewElement *cTemplateView::GetNextViewElement(void) {
    if (veIt == viewElements.end())
        return NULL;
    cTemplateViewElement *viewElement = veIt->second;
    veIt++;
    return viewElement; 
}

cTemplateViewList *cTemplateView::GetViewList(eViewList vl) {
    map < eViewList, cTemplateViewList* >::iterator hit = viewLists.find(vl);
    if (hit == viewLists.end())
        return NULL;
    return hit->second;
}

void cTemplateView::InitViewListIterator(void) {
    vlIt = viewLists.begin();
}

cTemplateViewList *cTemplateView::GetNextViewList(void) {
    if (vlIt == viewLists.end())
        return NULL;
    cTemplateViewList *viewList = vlIt->second;
    vlIt++;
    return viewList;    
}

cTemplateView *cTemplateView::GetSubView(eSubView sv) {
    map < eSubView, cTemplateView* >::iterator hit = subViews.find(sv);
    if (hit == subViews.end())
        return NULL;
    return hit->second;
}

cTemplateView *cTemplateView::GetPluginView(string pluginName, int pluginMenu) {
    map < string, map< int, cTemplateView*> >::iterator hit = pluginViews.find(pluginName);

    if (hit == pluginViews.end())
       return NULL;

    map< int, cTemplateView*> plugViews = hit->second;
    map< int, cTemplateView*>::iterator hit2 = plugViews.find(pluginMenu);

    if (hit2 == plugViews.end())
        return NULL;

    return hit2->second;
}


void cTemplateView::InitViewTabIterator(void) {
    vtIt = viewTabs.begin();
}

cTemplateViewTab *cTemplateView::GetNextViewTab(void) {
    if (vtIt == viewTabs.end()) {
        return NULL;
    }
    cTemplateViewTab *tab = *vtIt;
    vtIt++;
    return tab;
}

void cTemplateView::InitSubViewIterator(void) {
    svIt = subViews.begin();
}

cTemplateView *cTemplateView::GetNextSubView(void) {
    if (svIt == subViews.end())
        return NULL;
    cTemplateView *subView = svIt->second;
    svIt++;
    return subView; 
}

int cTemplateView::GetNumericParameter(eParamType type) {
    if (!parameters)
        return 0;
    return parameters->GetNumericParameter(type);
}

cRect cTemplateView::GetOsdSize(void) {
    cRect osdSize;
    if (!parameters) {
        return osdSize;
    }
    osdSize.SetX(parameters->GetNumericParameter(ptX));
    osdSize.SetY(parameters->GetNumericParameter(ptY));
    osdSize.SetWidth(parameters->GetNumericParameter(ptWidth));
    osdSize.SetHeight(parameters->GetNumericParameter(ptHeight));
    return osdSize;
}

int cTemplateView::GetNumPixmaps(void) {
    int numPixmaps = 0;
    for (map < eViewElement, cTemplateViewElement* >::iterator it = viewElements.begin(); it != viewElements.end(); it++) {
        cTemplateViewElement *viewElement = it->second;
        numPixmaps += viewElement->GetNumPixmaps();
    }
    return numPixmaps;
}

int cTemplateView::GetNumPixmapsViewElement(eViewElement ve) {
    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end())
        return 0;
    cTemplateViewElement *viewElement = hit->second; 
    return viewElement->GetNumPixmaps();
}

int cTemplateView::GetNumListViewMenuItems(void) {
    int numElements = 0;
    cTemplateViewList *menuList = GetViewList(vlMenuItem);
    if (!menuList)
        return numElements;
    return menuList->GetNumericParameter(ptNumElements);
}

bool cTemplateView::GetScalingWindow(cRect &scalingWindow) {
    if (!parameters)
        return false;
    bool doScale = false;
    int scaleX = parameters->GetNumericParameter(ptScaleTvX) + cOsd::OsdLeft();
    int scaleY = parameters->GetNumericParameter(ptScaleTvY) + cOsd::OsdTop();
    int scaleWidth = parameters->GetNumericParameter(ptScaleTvWidth);
    int scaleHeight = parameters->GetNumericParameter(ptScaleTvHeight);
    if (scaleX > -1 && scaleY > -1 && scaleWidth > -1 && scaleHeight > -1) {
        cRect suggestedScaleWindow(scaleX, scaleY, scaleWidth, scaleHeight);
        scalingWindow = cDevice::PrimaryDevice()->CanScaleVideo(suggestedScaleWindow);
        doScale = true;
    } else {
        scalingWindow = cDevice::PrimaryDevice()->CanScaleVideo(cRect::Null);
    }
    return doScale;
}

bool cTemplateView::ValidViewElement(const char *viewElement) {
    set<string>::iterator hit = viewElementsAllowed.find(viewElement);
    if (hit == viewElementsAllowed.end())
        return false;
    return true;
}

bool cTemplateView::ValidSubView(const char *subView) {
    set<string>::iterator hit = subViewsAllowed.find(subView);
    if (hit == subViewsAllowed.end())
        return false;
    return true;        
}

bool cTemplateView::ValidViewList(const char *viewList) {
    set<string>::iterator hit = viewListsAllowed.find(viewList);
    if (hit == viewListsAllowed.end())
        return false;
    return true;    
}

bool cTemplateView::ValidFunction(const char *func) {
    map < string, set < string > >::iterator hit = funcsAllowed.find(func);
    if (hit == funcsAllowed.end())
        return false;
    return true;
}

bool cTemplateView::ValidAttribute(const char *func, const char *att) {
    map < string, set < string > >::iterator hit = funcsAllowed.find(func);
    if (hit == funcsAllowed.end())
        return false;
    
    set<string>::iterator hitAtt = (hit->second).find(att);
    if (hitAtt == (hit->second).end())
        return false;

    return true;
}

void cTemplateView::Translate(void) {
    //Translate ViewElements
    InitViewElementIterator();
    cTemplateViewElement *viewElement = NULL;
    while(viewElement = GetNextViewElement()) {
        viewElement->InitIterator();
        cTemplatePixmap *pix = NULL;
        while(pix = viewElement->GetNextPixmap()) {
            pix->InitIterator();
            cTemplateFunction *func = NULL;
            while(func = pix->GetNextFunction()) {
                if (func->GetType() == ftDrawText || func->GetType() == ftDrawTextBox) {
                    string text = func->GetParameter(ptText);
                    string translation;
                    bool translated = globals->Translate(text, translation);
                    if (translated) {
                        func->SetTranslatedText(translation);
                    }
                }
            }
        }
    }
    //Translate viewLists
    InitViewListIterator();
    cTemplateViewList *viewList = NULL;
    while(viewList = GetNextViewList()) {
        viewList->InitIterator();
        cTemplatePixmap *pix = NULL;
        while(pix = viewList->GetNextPixmap()) {
            pix->InitIterator();
            cTemplateFunction *func = NULL;
            while(func = pix->GetNextFunction()) {
                if (func->GetType() == ftDrawText || func->GetType() == ftDrawTextBox) {
                    string text = func->GetParameter(ptText);
                    string translation;
                    bool translated = globals->Translate(text, translation);
                    if (translated) {
                        func->SetTranslatedText(translation);
                    }
                }
            }
        }
        cTemplateViewElement *listElement = viewList->GetListElement();
        listElement->InitIterator();
        while(pix = listElement->GetNextPixmap()) {
            pix->InitIterator();
            cTemplateFunction *func = NULL;
            while(func = pix->GetNextFunction()) {
                if (func->GetType() == ftDrawText || func->GetType() == ftDrawTextBox) {
                    string text = func->GetParameter(ptText);
                    string translation;
                    bool translated = globals->Translate(text, translation);
                    if (translated) {
                        func->SetTranslatedText(translation);
                    }
                }
            }
        }
        
        cTemplateViewElement *listElementCurrent = viewList->GetListElementCurrent();
        if (listElementCurrent) {
            listElementCurrent->InitIterator();
            while(pix = listElementCurrent->GetNextPixmap()) {
                pix->InitIterator();
                cTemplateFunction *func = NULL;
                while(func = pix->GetNextFunction()) {
                    if (func->GetType() == ftDrawText || func->GetType() == ftDrawTextBox) {
                        string text = func->GetParameter(ptText);
                        string translation;
                        bool translated = globals->Translate(text, translation);
                        if (translated) {
                            func->SetTranslatedText(translation);
                        }
                    }
                }
            }
        }
    }

    //Translate viewTabs
    InitViewTabIterator();
    cTemplateViewTab *viewTab = NULL;
    while(viewTab = GetNextViewTab()) {
        string tabName = viewTab->GetName();
        string tabTrans;
        bool translated = globals->Translate(tabName, tabTrans);
        if (translated) {
            viewTab->SetName(tabTrans);            
        }
        viewTab->InitIterator();
        cTemplateFunction *func = NULL;
        while(func = viewTab->GetNextFunction()) {
            if (func->GetType() == ftDrawText || func->GetType() == ftDrawTextBox) {
                string text = func->GetParameter(ptText);
                string translation;
                translated = globals->Translate(text, translation);
                if (translated) {
                    func->SetTranslatedText(translation);
                }
            }
        }
    }

    //Translate Subviews
    InitSubViewIterator();
    cTemplateView *subView = NULL;
    while(subView = GetNextSubView()) {
        subView->Translate();
    }
}

void cTemplateView::PreCache(bool isSubview) {

    if (!isSubview) {
        int osdW = cOsd::OsdWidth();
        int osdH = cOsd::OsdHeight();
        parameters->SetContainer(0, 0, osdW, osdH);
    } else {
        parameters->SetContainer(containerX, containerY, containerWidth, containerHeight);      
    }
    //Calculate OSD Size
    parameters->CalculateParameters();

    int osdX = parameters->GetNumericParameter(ptX);
    int osdY = parameters->GetNumericParameter(ptY);
    int osdWidth = parameters->GetNumericParameter(ptWidth);
    int osdHeight = parameters->GetNumericParameter(ptHeight);
    int pixOffset = 0;

    //Cache ViewElements
    for (map < eViewElement, cTemplateViewElement* >::iterator it = viewElements.begin(); it != viewElements.end(); it++) {
        cTemplateViewElement *viewElement = it->second;
        viewElement->SetGlobals(globals);
        if (!isSubview)
            viewElement->SetContainer(0, 0, osdWidth, osdHeight);
        else
            viewElement->SetContainer(osdX, osdY, osdWidth, osdHeight);
        viewElement->CalculateParameters();
        viewElement->CalculatePixmapParameters();
        viewElement->SetPixOffset(pixOffset);
        pixOffset += viewElement->GetNumPixmaps();
    }


    //Cache ViewLists
    for (map < eViewList, cTemplateViewList* >::iterator it = viewLists.begin(); it != viewLists.end(); it++) {
        cTemplateViewList *viewList = it->second;
        viewList->SetGlobals(globals);
        //viewlists are only in subviews
        viewList->SetContainer(osdX, osdY, osdWidth, osdHeight);
        viewList->CalculateParameters();
        viewList->CalculateListParameters();
    }

    //Cache ViewTabs
    for (vector<cTemplateViewTab*>::iterator tab = viewTabs.begin(); tab != viewTabs.end(); tab++) {
        //viewtabs are only in subviews
        (*tab)->SetContainer(osdX, osdY, osdWidth, osdHeight);
        (*tab)->SetGlobals(globals);
        (*tab)->CalculateParameters();
    }

    //Cache Subviews
    for (map < eSubView, cTemplateView* >::iterator it = subViews.begin(); it != subViews.end(); it++) {
        cTemplateView *subView = it->second;
        subView->SetContainer(0, 0, osdWidth, osdHeight);
        subView->PreCache(true);
    }

    //Cache Plugin Subviews
    for (map < string, map< int, cTemplateView*> >::iterator it = pluginViews.begin(); it != pluginViews.end(); it++) {
        map< int, cTemplateView*> plugViews = it->second;
        for (map< int, cTemplateView*>::iterator it2 = plugViews.begin(); it2 != plugViews.end(); it2++) {
            cTemplateView *plugView = it2->second;
            plugView->SetContainer(0, 0, osdWidth, osdHeight);
            plugView->PreCache(true);
        }
    }
}

void cTemplateView::Debug(void) {

    esyslog("skindesigner: TemplateView %s", viewName.c_str());;
    
    parameters->Debug();

    for (map < eViewElement, cTemplateViewElement* >::iterator it = viewElements.begin(); it != viewElements.end(); it++) {
        esyslog("skindesigner: ++++++++ ViewElement: %s", GetViewElementName(it->first).c_str());
        cTemplateViewElement *viewElement = it->second;
        viewElement->Debug();
    }

    for (map < eViewList, cTemplateViewList* >::iterator it = viewLists.begin(); it != viewLists.end(); it++) {
        esyslog("skindesigner: ++++++++ ViewList: %s", GetViewListName(it->first).c_str());
        cTemplateViewList *viewList = it->second;
        viewList->Debug();
    }

    for (vector<cTemplateViewTab*>::iterator tab = viewTabs.begin(); tab != viewTabs.end(); tab++) {
        esyslog("skindesigner: ++++++++ ViewTab");
        (*tab)->Debug();
    }

    for (map < eSubView, cTemplateView* >::iterator it = subViews.begin(); it!= subViews.end(); it++) {
        esyslog("skindesigner: ++++++++ SubView: %s", GetSubViewName(it->first).c_str());
        cTemplateView *subView = it->second;
        subView->Debug();       
    }
    
    for (map < string, map< int, cTemplateView*> >::iterator it = pluginViews.begin(); it!= pluginViews.end(); it++) {
        esyslog("skindesigner: ++++++++ Plugin: %s", it->first.c_str());
        map< int, cTemplateView*> plugViews = it->second;
        for (map< int, cTemplateView*>::iterator it2 = plugViews.begin(); it2 != plugViews.end(); it2++) {
            esyslog("skindesigner: Tmpl %d", it2->first);
            ((cTemplateView*)it2->second)->Debug();
        }
    }

}


void cTemplateView::SetFunctionDefinitions(void) {
    
    string name = "viewelement";
    set<string> attributes;
    attributes.insert("debug");
    attributes.insert("delay");
    attributes.insert("fadetime");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "area";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("layer");
    attributes.insert("transparency");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "areascroll";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("orientation");
    attributes.insert("delay");
    attributes.insert("mode");
    attributes.insert("scrollspeed");
    attributes.insert("condition");
    attributes.insert("scrollelement");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("layer");
    attributes.insert("transparency");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "loop";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("name");
    attributes.insert("orientation");
    attributes.insert("condition");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("columnwidth");
    attributes.insert("rowheight");
    attributes.insert("overflow");
    attributes.insert("maxitems");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "fill";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("color");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawtext";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("font");
    attributes.insert("fontsize");
    attributes.insert("color");
    attributes.insert("text");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawtextbox";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("align");
    attributes.insert("maxlines");  
    attributes.insert("font");
    attributes.insert("fontsize");
    attributes.insert("color");
    attributes.insert("text");
    attributes.insert("float");
    attributes.insert("floatwidth");
    attributes.insert("floatheight");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawimage";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("imagetype");
    attributes.insert("path");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("cache");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawrectangle";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("color");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawellipse";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("color");
    attributes.insert("quadrant");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));

    name = "drawslope";
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("condition");
    attributes.insert("name");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("align");
    attributes.insert("valign");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("color");
    attributes.insert("type");
    funcsAllowed.insert(pair< string, set<string> >(name, attributes));
}

/************************************************************************************
* cTemplateViewChannel
************************************************************************************/

cTemplateViewChannel::cTemplateViewChannel(void) {

    viewName = "displaychannel";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    SetViewElements();
}

cTemplateViewChannel::~cTemplateViewChannel() {
}

void cTemplateViewChannel::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("channelinfo");
    viewElementsAllowed.insert("channelgroup");
    viewElementsAllowed.insert("epginfo");
    viewElementsAllowed.insert("progressbar");
    viewElementsAllowed.insert("progressbarback");
    viewElementsAllowed.insert("statusinfo");
    viewElementsAllowed.insert("audioinfo");
    viewElementsAllowed.insert("screenresolution");
    viewElementsAllowed.insert("signalquality");
    viewElementsAllowed.insert("signalqualityback");
    viewElementsAllowed.insert("devices");
    viewElementsAllowed.insert("scrapercontent");
    viewElementsAllowed.insert("datetime");
    viewElementsAllowed.insert("message");
    viewElementsAllowed.insert("customtokens");
}

string cTemplateViewChannel::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veChannelInfo:
            name = "ChannelInfo";
            break;
        case veChannelGroup:
            name = "ChannelGroup";
            break;
        case veEpgInfo:
            name = "EpgInfo";
            break;
        case veProgressBar:
            name = "ProgressBar";
            break;
        case veProgressBarBack:
            name = "ProgressBar Background";
            break;
        case veStatusInfo:
            name = "StatusInfo";
            break;
        case veAudioInfo:
            name = "AudioInfo";
            break;
        case veScreenResolution:
            name = "Screen Resolution";
            break;
        case veSignalQuality:
            name = "Signal Quality";
            break;
        case veSignalQualityBack:
            name = "Signal Quality Background";
            break;
        case veDevices:
            name = "Devices";
            break;
        case veScraperContent:
            name = "Scraper Content";
            break;
        case veDateTime:
            name = "DateTime";
            break;
        case veMessage:
            name = "Message";
            break;
        case veCustomTokens:
            name = "Custom Tokens";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewChannel::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if (!sViewElement.compare("channelinfo")) {
        ve = veChannelInfo;
    } else if (!sViewElement.compare("channelgroup")) {
        ve = veChannelGroup;
    } else if (!sViewElement.compare("epginfo")) {
        ve = veEpgInfo;
    } else if (!sViewElement.compare("progressbar")) {
        ve = veProgressBar;
    } else if (!sViewElement.compare("progressbarback")) {
        ve = veProgressBarBack;
    } else if (!sViewElement.compare("statusinfo")) {
        ve = veStatusInfo;
    } else if (!sViewElement.compare("audioinfo")) {
        ve = veAudioInfo;
    } else if (!sViewElement.compare("screenresolution")) {
        ve = veScreenResolution;
    } else if (!sViewElement.compare("signalquality")) {
        ve = veSignalQuality;
    } else if (!sViewElement.compare("signalqualityback")) {
        ve = veSignalQualityBack;
    } else if (!sViewElement.compare("devices")) {
        ve = veDevices;
    } else if (!sViewElement.compare("scrapercontent")) {
        ve = veScraperContent;
    } else if (!sViewElement.compare("datetime")) {
        ve = veDateTime;
    } else if (!sViewElement.compare("message")) {
        ve = veMessage;
    } else if (!sViewElement.compare("customtokens")) {
        ve = veCustomTokens;
    }

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displaychannel: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}

/************************************************************************************
* cTemplateViewMenu
************************************************************************************/

cTemplateViewMenu::cTemplateViewMenu(void) {

    viewName = "displaymenu";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    string subViewName = "menudefault";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight"); 
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menumain";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menusetup";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menuschedules";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menuchannels";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menutimers";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menurecordings";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menudetailedepg";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menudetailedrecording";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    subViewName = "menudetailedtext";
    //definition of allowed parameters for subtemplate menumain 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(subViewName, attributes));

    //definition of allowed parameters for timerlist viewlist 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("orientation");
    attributes.insert("align");
    attributes.insert("numlistelements");
    funcsAllowed.insert(pair< string, set<string> >("timerlist", attributes));

    //definition of allowed parameters for menuitems viewlist 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("orientation");
    attributes.insert("align");
    attributes.insert("menuitemwidth");
    attributes.insert("determinatefont");
    attributes.insert("numlistelements");
    funcsAllowed.insert(pair< string, set<string> >("menuitems", attributes));

    //definition of allowed parameters for currentitems viewlist 
    attributes.clear();
    attributes.insert("delay");
    attributes.insert("fadetime");
    funcsAllowed.insert(pair< string, set<string> >("currentelement", attributes));

    //definition of allowed parameters for viewtab
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("name");
    attributes.insert("condition");
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("layer");
    attributes.insert("transparency");
    attributes.insert("scrollheight");
    funcsAllowed.insert(pair< string, set<string> >("tab", attributes));

    //definition of allowed parameters for plugin menus
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >("menuplugin", attributes));

    SetSubViews();
    SetViewElements();
    SetViewLists();

}

cTemplateViewMenu::~cTemplateViewMenu() {
}

void cTemplateViewMenu::SetSubViews(void) {
    subViewsAllowed.insert("menudefault");
    subViewsAllowed.insert("menumain");
    subViewsAllowed.insert("menusetup");
    subViewsAllowed.insert("menuschedules");
    subViewsAllowed.insert("menutimers");
    subViewsAllowed.insert("menurecordings");
    subViewsAllowed.insert("menuchannels");
    subViewsAllowed.insert("menudetailedepg");
    subViewsAllowed.insert("menudetailedrecording");
    subViewsAllowed.insert("menudetailedtext");
}

void cTemplateViewMenu::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("datetime");
    viewElementsAllowed.insert("header");
    viewElementsAllowed.insert("colorbuttons");
    viewElementsAllowed.insert("message");
    viewElementsAllowed.insert("discusage");
    viewElementsAllowed.insert("systemload");
    viewElementsAllowed.insert("timers");
    viewElementsAllowed.insert("devices");
    viewElementsAllowed.insert("scrollbar");
    viewElementsAllowed.insert("detailheader");
    viewElementsAllowed.insert("tablabels");
}

void cTemplateViewMenu::SetViewLists(void) {
    viewListsAllowed.insert("timerlist");
    viewListsAllowed.insert("menuitems");
}

string cTemplateViewMenu::GetSubViewName(eSubView sv) {
    string name;
    switch (sv) {
        case svMenuDefault:
            name = "Default Menu";
            break;
        case svMenuMain:
            name = "Main Menu";
            break;
        case svMenuSetup:
            name = "Setup Menu";
            break;
        case svMenuSchedules:
            name = "Schedules Menu";
            break;
        case svMenuTimers:
            name = "Timers Menu";
            break;
        case svMenuRecordings:
            name = "Recordings Menu";
            break;
        case svMenuChannels:
            name = "Channels Menu";
            break;
        case svMenuDetailedEpg:
            name = "Detailed EPG";
            break;
        case svMenuDetailedRecording:
            name = "Detailed Recording";
            break;
        case svMenuDetailedText:
            name = "Detailed Text";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}


string cTemplateViewMenu::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veDateTime:
            name = "DateTime";
            break;
        case veHeader:
            name = "Header";
            break;
        case veButtons:
            name = "Color Buttons";
            break;
        case veMessage:
            name = "Message";
            break;
        case veDiscUsage:
            name = "Disc Usage";
            break;
         case veSystemLoad:
            name = "System Load";
            break;
        case veTimers:
            name = "Timers";
            break;
        case veDevices:
            name = "Devices";
            break;
        case veMenuItem:
            name = "Menu Item";
            break;
        case veMenuCurrentItemDetail:
            name = "Menu Current Item Detail";
            break;
        case veScrollbar:
            name = "Scrollbar";
            break;
        case veDetailHeader:
            name = "Detail header";
            break;
        case veTabLabels:
            name = "tab labels";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

string cTemplateViewMenu::GetViewListName(eViewList vl) {
    string name;
    switch (vl) {
        case vlTimerList:
            name = "Timer List";
            break;
        case vlMenuItem:
            name = "Menu Item";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewMenu::AddSubView(string sSubView, cTemplateView *subView) {
    eSubView sv = svUndefined;
    
    if (!sSubView.compare("menumain")) {
        sv = svMenuMain;
    } else if (!sSubView.compare("menudefault")) {
        sv = svMenuDefault;
    } else if (!sSubView.compare("menuschedules")) {
        sv = svMenuSchedules;
    } else if (!sSubView.compare("menusetup")) {
        sv = svMenuSetup;
    } else if (!sSubView.compare("menuschedules")) {
        sv = svMenuSchedules;
    } else if (!sSubView.compare("menutimers")) {
        sv = svMenuTimers;
    } else if (!sSubView.compare("menurecordings")) {
        sv = svMenuRecordings;
    } else if (!sSubView.compare("menuchannels")) {
        sv = svMenuChannels;
    } else if (!sSubView.compare("menudetailedepg")) {
        sv = svMenuDetailedEpg;
    } else if (!sSubView.compare("menudetailedrecording")) {
        sv = svMenuDetailedRecording;
    } else if (!sSubView.compare("menudetailedtext")) {
        sv = svMenuDetailedText;
    }

    if (sv == svUndefined) {
        esyslog("skindesigner: unknown SubView in displayMenu: %s", sSubView.c_str());
        return;
    }
    subView->SetGlobals(globals);
    subViews.insert(pair<eSubView, cTemplateView*>(sv, subView));
}

void cTemplateViewMenu::AddPluginView(string plugName, int templNo, cTemplateView *plugView) {
    plugView->SetGlobals(globals);

    map < string, map< int, cTemplateView*> >::iterator hit = pluginViews.find(plugName);

    if (hit == pluginViews.end()) {
        map< int, cTemplateView*> plugTemplates;
        plugTemplates.insert(pair<int, cTemplateView*>(templNo, plugView));
        pluginViews.insert(pair< string, map< int, cTemplateView*> >(plugName, plugTemplates));
    } else {
        hit->second.insert(pair<int, cTemplateView*>(templNo, plugView));
    }
}

void cTemplateViewMenu::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if (!sViewElement.compare("datetime")) {
        ve = veDateTime;
    } else if (!sViewElement.compare("header")) {
        ve = veHeader;
    } else if (!sViewElement.compare("colorbuttons")) {
        ve = veButtons;
    } else if (!sViewElement.compare("message")) {
        ve = veMessage;
    } else if (!sViewElement.compare("discusage")) {
        ve = veDiscUsage;
    } else if (!sViewElement.compare("systemload")) {
        ve = veSystemLoad;
    } else if (!sViewElement.compare("timers")) {
        ve = veTimers;
    } else if (!sViewElement.compare("devices")) {
        ve = veDevices;
    } else if (!sViewElement.compare("scrollbar")) {
        ve = veScrollbar;
    } else if (!sViewElement.compare("detailheader")) {
        ve = veDetailHeader;
    } else if (!sViewElement.compare("tablabels")) {
        ve = veTabLabels;
    }

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displayMenu: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}

void cTemplateViewMenu::AddViewList(string sViewList, cTemplateViewList *viewList) {
    
    eViewList vl = vlUndefined;
    if (!sViewList.compare("timerlist")) {
        vl = vlTimerList;
    } else if (!sViewList.compare("menuitems")) {
        vl = vlMenuItem;
    }

    if (vl == vlUndefined) {
        esyslog("skindesigner: unknown ViewList in displaymenu: %s", sViewList.c_str());
        return;
    }
    
    viewList->SetGlobals(globals);
    viewLists.insert(pair< eViewList, cTemplateViewList*>(vl, viewList));
}

void cTemplateViewMenu::AddViewTab(cTemplateViewTab *viewTab) {
    viewTabs.push_back(viewTab);
}

/************************************************************************************
* cTemplateViewMessage
************************************************************************************/

cTemplateViewMessage::cTemplateViewMessage(void) {

    viewName = "displaymessage";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    SetViewElements();
}

cTemplateViewMessage::~cTemplateViewMessage() {
}

void cTemplateViewMessage::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("message");
}

string cTemplateViewMessage::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veMessage:
            name = "Message";
            break;      
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewMessage::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if (!sViewElement.compare("message")) {
        ve = veMessage;
    } 

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displaymessage: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}

/************************************************************************************
* cTemplateViewReplay
************************************************************************************/

cTemplateViewReplay::cTemplateViewReplay(void) {

    viewName = "displayreplay";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    //definition of allowed parameters for onpause and onpausemodeonly viewelement 
    attributes.clear();
    attributes.insert("debug");
    attributes.insert("delay");
    attributes.insert("fadetime");
    funcsAllowed.insert(pair< string, set<string> >("onpause", attributes));
    funcsAllowed.insert(pair< string, set<string> >("onpausemodeonly", attributes));

    SetViewElements();
}

cTemplateViewReplay::~cTemplateViewReplay() {
}

void cTemplateViewReplay::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("backgroundmodeonly");
    viewElementsAllowed.insert("datetime");
    viewElementsAllowed.insert("rectitle");
    viewElementsAllowed.insert("recinfo");
    viewElementsAllowed.insert("scrapercontent");
    viewElementsAllowed.insert("currenttime");
    viewElementsAllowed.insert("totaltime");
    viewElementsAllowed.insert("progressbar");
    viewElementsAllowed.insert("cutmarks");
    viewElementsAllowed.insert("controlicons");
    viewElementsAllowed.insert("controliconsmodeonly");
    viewElementsAllowed.insert("jump");
    viewElementsAllowed.insert("message");
    viewElementsAllowed.insert("onpause");
    viewElementsAllowed.insert("onpausemodeonly");
}

string cTemplateViewReplay::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veDateTime:
            name = "DateTime";
            break;
        case veRecTitle:
            name = "Recording Title";
            break;      
        case veRecInfo:
            name = "Recording Information";
            break;      
        case veRecCurrent:
            name = "Recording current Time";
            break;
        case veRecTotal:
            name = "Recording total Time";
            break;
        case veRecProgressBar:
            name = "Rec Progress Bar";
            break;
        case veCuttingMarks:
            name = "Cutting Marks";
            break;
        case veControlIcons:
            name = "Control Icons";
            break;
        case veControlIconsModeOnly:
            name = "Control Icons Mode only";
            break;
        case veBackgroundModeOnly:
            name = "Background Mode only";
            break;   
        case veRecJump:
            name = "Recording Jump";
            break;   
        case veScraperContent:
            name = "Scraper Content";
            break;
        case veOnPause:
            name = "On Pause";
            break;
        case veOnPauseModeOnly:
            name = "On Pause Mode Only";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewReplay::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if (!sViewElement.compare("datetime")) {
        ve = veDateTime;
    } else if (!sViewElement.compare("rectitle")) {
        ve = veRecTitle;
    } else if (!sViewElement.compare("recinfo")) {
        ve = veRecInfo;
    } else if (!sViewElement.compare("scrapercontent")) {
        ve = veScraperContent;
    } else if (!sViewElement.compare("currenttime")) {
        ve = veRecCurrent;
    } else if (!sViewElement.compare("totaltime")) {
        ve = veRecTotal;
    } else if (!sViewElement.compare("progressbar")) {
        ve = veRecProgressBar;
    } else if (!sViewElement.compare("cutmarks")) {
        ve = veCuttingMarks;
    } else if (!sViewElement.compare("controlicons")) {
        ve = veControlIcons;
    } else if (!sViewElement.compare("controliconsmodeonly")) {
        ve = veControlIconsModeOnly;
    } else if (!sViewElement.compare("backgroundmodeonly")) {
        ve = veBackgroundModeOnly;
    } else if (!sViewElement.compare("jump")) {
        ve = veRecJump;
    } else if (!sViewElement.compare("message")) {
        ve = veMessage;
    } else if (!sViewElement.compare("onpause")) {
        ve = veOnPause;
    } else if (!sViewElement.compare("onpausemodeonly")) {
        ve = veOnPauseModeOnly;
    }

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displayreplay: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}


/************************************************************************************
* cTemplateViewVolume
************************************************************************************/

cTemplateViewVolume::cTemplateViewVolume(void) {

    viewName = "displayvolume";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    SetViewElements();
}

cTemplateViewVolume::~cTemplateViewVolume() {
}

void cTemplateViewVolume::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("volume");
}

string cTemplateViewVolume::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veVolume:
            name = "Volume";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewVolume::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if (!sViewElement.compare("volume")) {
        ve = veVolume;
    } 

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displayvolume: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}

/************************************************************************************
* cTemplateViewAudioTracks
************************************************************************************/

cTemplateViewAudioTracks::cTemplateViewAudioTracks(void) {

    viewName = "displayaudiotracks";
    //definition of allowed parameters for class itself 
    set<string> attributes;
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("fadetime");
    attributes.insert("scaletvx");
    attributes.insert("scaletvy");
    attributes.insert("scaletvwidth");
    attributes.insert("scaletvheight");
    funcsAllowed.insert(pair< string, set<string> >(viewName, attributes));

    //definition of allowed parameters for menuitems viewlist 
    attributes.clear();
    attributes.insert("x");
    attributes.insert("y");
    attributes.insert("width");
    attributes.insert("height");
    attributes.insert("orientation");
    attributes.insert("align");
    attributes.insert("menuitemwidth");
    attributes.insert("numlistelements");
    funcsAllowed.insert(pair< string, set<string> >("menuitems", attributes));

    SetViewElements();
    SetViewLists();
}

cTemplateViewAudioTracks::~cTemplateViewAudioTracks() {
}

void cTemplateViewAudioTracks::SetViewElements(void) {
    viewElementsAllowed.insert("background");
    viewElementsAllowed.insert("header");
}

void cTemplateViewAudioTracks::SetViewLists(void) {
    viewListsAllowed.insert("menuitems");
}

string cTemplateViewAudioTracks::GetViewElementName(eViewElement ve) {
    string name;
    switch (ve) {
        case veBackground:
            name = "Background";
            break;
        case veHeader:
            name = "Header";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

string cTemplateViewAudioTracks::GetViewListName(eViewList vl) {
    string name;
    switch (vl) {
        case vlMenuItem:
            name = "Menu Item";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateViewAudioTracks::AddPixmap(string sViewElement, cTemplatePixmap *pix, vector<pair<string, string> > &viewElementattributes) {
    eViewElement ve = veUndefined;
    
    if (!sViewElement.compare("background")) {
        ve = veBackground;
    } else if(!sViewElement.compare("header")) {
        ve = veHeader;
    }

    if (ve == veUndefined) {
        esyslog("skindesigner: unknown ViewElement in displayaudiotracks: %s", sViewElement.c_str());
        return;
    }

    pix->SetGlobals(globals);

    map < eViewElement, cTemplateViewElement* >::iterator hit = viewElements.find(ve);
    if (hit == viewElements.end()) {
        cTemplateViewElement *viewElement = new cTemplateViewElement();
        viewElement->SetParameters(viewElementattributes);
        viewElement->AddPixmap(pix);
        viewElements.insert(pair< eViewElement, cTemplateViewElement*>(ve, viewElement));
    } else {
        (hit->second)->AddPixmap(pix);
    }
}

void cTemplateViewAudioTracks::AddViewList(string sViewList, cTemplateViewList *viewList) {
    
    eViewList vl = vlUndefined;
    if (!sViewList.compare("menuitems")) {
        vl = vlMenuItem;
    }

    if (vl == vlUndefined) {
        esyslog("skindesigner: unknown ViewList in displaymenu: %s", sViewList.c_str());
        return;
    }
    
    viewList->SetGlobals(globals);
    viewLists.insert(pair< eViewList, cTemplateViewList*>(vl, viewList));
}
