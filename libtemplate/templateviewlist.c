#include "templateviewlist.h"
#include "../config.h"

cTemplateViewList::cTemplateViewList(void) : cTemplateViewElement() {
    listElement = NULL;
    currentElement = NULL;
}

cTemplateViewList::~cTemplateViewList(void) {
    if (listElement)
        delete listElement;
    if (currentElement)
        delete currentElement;
}

void cTemplateViewList::SetGlobals(cGlobals *globals) { 
    cTemplateViewElement::SetGlobals(globals);
    if (listElement)
        listElement->SetGlobals(globals);
    if (currentElement)
        currentElement->SetGlobals(globals);
}

bool cTemplateViewList::CalculateListParameters(void) {
    if (!parameters)
        return false;
    parameters->SetContainer(containerX, containerY, containerWidth, containerHeight);
    parameters->SetGlobals(globals);
    bool paramsValid = parameters->CalculateParameters();
    if (!listElement)
        return false;
    listElement->SetContainer(parameters->GetNumericParameter(ptX),
                              parameters->GetNumericParameter(ptY),
                              parameters->GetNumericParameter(ptWidth), 
                              parameters->GetNumericParameter(ptHeight));
    paramsValid = listElement->CalculateParameters();
    paramsValid = listElement->CalculatePixmapParametersList(parameters->GetNumericParameter(ptOrientation), 
                                                             parameters->GetNumericParameter(ptNumElements));
    if (!currentElement)
        return paramsValid;
    currentElement->SetContainer(parameters->GetNumericParameter(ptX),
                                 parameters->GetNumericParameter(ptY),
                                 parameters->GetNumericParameter(ptWidth), 
                                 parameters->GetNumericParameter(ptHeight));
    paramsValid = currentElement->CalculateParameters();
    paramsValid = currentElement->CalculatePixmapParameters();
    currentElement->SetPixOffset(0);
    return paramsValid;
}

bool cTemplateViewList::CalculateListParameters(map < string, int > *intTokens) {
    if (!parameters)
        return false;
    parameters->ClearDynamicParameters();
    parameters->SetIntTokens(intTokens);
    parameters->ParseParameters();
    parameters->UnsetIntTokens();

    listElement->SetContainer(parameters->GetNumericParameter(ptX),
                              parameters->GetNumericParameter(ptY),
                              parameters->GetNumericParameter(ptWidth), 
                              parameters->GetNumericParameter(ptHeight));
    bool paramsValid = listElement->CalculateParameters();
    paramsValid = listElement->CalculatePixmapParametersList(parameters->GetNumericParameter(ptOrientation), 
                                                             parameters->GetNumericParameter(ptNumElements));
    return paramsValid;
}

int cTemplateViewList::GetAverageFontWidth(void) {
    int defaultAverageFontWidth = 20;

    if (!listElement)
        return defaultAverageFontWidth;

    int numItems = GetNumericParameter(ptNumElements);
    int listHeight = GetNumericParameter(ptHeight);
    if (listHeight <= 0)
        return defaultAverageFontWidth;
    int itemHeight = (double)listHeight / (double)numItems;
    string fontFuncName = parameters->GetParameter(ptDeterminateFont);

    cTemplateFunction *fontFunc = listElement->GetFunction(fontFuncName);
    if (!fontFunc)
        return defaultAverageFontWidth;

    string fontNameToken = fontFunc->GetParameter(ptFont);
    string paramFontSize = fontFunc->GetParameter(ptFontSize);

    string fontName = "";
    if ((fontNameToken.find("{") == 0) && (fontNameToken.find("}") == (fontNameToken.size()-1))) {
        fontNameToken = fontNameToken.substr(1, fontNameToken.size()-2);
        map<string,string>::iterator hit = globals->fonts.find(fontNameToken);
        if (hit != globals->fonts.end()) {
            fontName = hit->second;
        } else {
            map<string,string>::iterator def = globals->fonts.find("vdrOsd");
            if (def == globals->fonts.end())
                return defaultAverageFontWidth;
            fontName = def->second;
        }
    } else {
        //if no token, directly use input
        fontName = fontNameToken;
    }

    cNumericParameter pFontSize(paramFontSize);
    pFontSize.SetGlobals(globals);
    pFontSize.SetAreaSize(1000, itemHeight);
    pFontSize.SetVertical();
    int fontSize = pFontSize.Parse(paramFontSize);
    if (!pFontSize.Valid())
        return defaultAverageFontWidth;

    int averageFontWidth = fontManager->Width(fontName, fontSize, "x") + 3;
    return averageFontWidth;
}

cFont *cTemplateViewList::GetTextAreaFont(void) {
    if (!listElement)
        return NULL;

    int fontWidth = 0;
    int numItems = GetNumericParameter(ptNumElements);
    int listHeight = GetNumericParameter(ptHeight);
    if (listHeight <= 0)
        return NULL;
    int itemHeight = (double)listHeight / (double)numItems;
    string fontFuncName = parameters->GetParameter(ptDeterminateFont);

    cTemplateFunction *fontFunc = listElement->GetFunction(fontFuncName);
    if (!fontFunc)
        return NULL;

    string fontNameToken = fontFunc->GetParameter(ptFont);
    string paramFontSize = fontFunc->GetParameter(ptFontSize);

    string fontName = "";
    if ((fontNameToken.find("{") == 0) && (fontNameToken.find("}") == (fontNameToken.size()-1))) {
        fontNameToken = fontNameToken.substr(1, fontNameToken.size()-2);
        map<string,string>::iterator hit = globals->fonts.find(fontNameToken);
        if (hit != globals->fonts.end()) {
            fontName = hit->second;
        } else {
            map<string,string>::iterator def = globals->fonts.find("vdrOsd");
            if (def == globals->fonts.end())
                return NULL;
            fontName = def->second;
        }
    } else {
        //if no token, directly use input
        fontName = fontNameToken;
    }

    cNumericParameter pFontSize(paramFontSize);
    pFontSize.SetGlobals(globals);
    pFontSize.SetAreaSize(1000, itemHeight);
    pFontSize.SetVertical();
    int fontSize = pFontSize.Parse(paramFontSize);
    if (!pFontSize.Valid())
        return NULL;

    return fontManager->FontUncached(fontName, fontSize);
}


int cTemplateViewList::GetMenuItemWidth(void) {
    return GetNumericParameter(ptMenuItemWidth);    
}


int cTemplateViewList::GetNumPixmaps(void) {
    if (!listElement)
        return 0;
    return listElement->GetNumPixmaps();
}

void cTemplateViewList::Debug(void) {
    if (parameters)
        parameters->Debug();
    esyslog("skindesigner: --- listelement: ");
    if (listElement)
        listElement->Debug();
    esyslog("skindesigner: --- currentelement: ");
    if (currentElement)
        currentElement->Debug();
}