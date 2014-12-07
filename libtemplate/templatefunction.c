#include "templatefunction.h"
#include "../config.h"
#include "../libcore/helpers.h"

using namespace std;

// --- cTemplateFunction -------------------------------------------------------------

cTemplateFunction::cTemplateFunction(eFuncType type) { 
    this->type = type;
    debug = false;
    containerX = 0; 
    containerY = 0; 
    containerWidth = 0;
    containerHeight = 0;
    columnWidth = -1;
    rowHeight = -1;
    globals = NULL;
    condParam = NULL;
    parsedCompletely = false;
    updated = false;
    alreadyCutted = false;
    parsedTextWidth = 0;
    fontName = "";
    imgPath = "";
    textboxHeight = 0;
    stringTokens = NULL;
    intTokens = NULL;
    parsedText = "";
    cuttedText = "";
}

cTemplateFunction::~cTemplateFunction(void) {
    if (condParam)
        delete condParam;
}

/*******************************************************************
* Public Functions
*******************************************************************/

void cTemplateFunction::SetParameters(vector<pair<string, string> > params) { 
    for (vector<pair<string, string> >::iterator it = params.begin(); it != params.end(); it++) {
        string name = it->first;
        pair< eParamType, string > p;
        if (!name.compare("debug")) {
            string value = it->second;
            if (!value.compare("true")) {
                debug = true;
            }
            continue;
        } else if (!name.compare("condition")) {
            p.first = ptCond;
        } else if (!name.compare("name")) {
            p.first = ptName;
        } else if (!name.compare("x")) {
            p.first = ptX;
        } else if (!name.compare("y")) {
            p.first = ptY;
        } else if (!name.compare("width")) {
            p.first = ptWidth;
        } else if (!name.compare("height")) {
            p.first = ptHeight;
        } else if (!name.compare("menuitemwidth")) {
            p.first = ptMenuItemWidth;
        } else if (!name.compare("fadetime")) {
            p.first = ptFadeTime;
        } else if (!name.compare("imagetype")) {
            p.first = ptImageType;
        } else if (!name.compare("path")) {
            p.first = ptPath;
        } else if (!name.compare("color")) {
            p.first = ptColor;
        } else if (!name.compare("font")) {
            p.first = ptFont;
        } else if (!name.compare("fontsize")) {
            p.first = ptFontSize;
        } else if (!name.compare("text")) {
            p.first = ptText;
        } else if (!name.compare("layer")) {
            p.first = ptLayer;
        } else if (!name.compare("transparency")) {
            p.first = ptTransparency;
        } else if (!name.compare("quadrant")) {
            p.first = ptQuadrant;
        } else if (!name.compare("type")) {
            p.first = ptType;
        } else if (!name.compare("align")) {
            p.first = ptAlign;
        } else if (!name.compare("valign")) {
            p.first = ptValign;
        } else if (!name.compare("delay")) {
            p.first = ptDelay;
        } else if (!name.compare("mode")) {
            p.first = ptScrollMode;
        } else if (!name.compare("scrollspeed")) {
            p.first = ptScrollSpeed;
        } else if (!name.compare("orientation")) {
            p.first = ptOrientation;
        } else if (!name.compare("numlistelements")) {
            p.first = ptNumElements;
        } else if (!name.compare("scrollelement")) {
            p.first = ptScrollElement;
        } else if (!name.compare("scrollheight")) {
            p.first = ptScrollHeight;
        } else if (!name.compare("float")) {
            p.first = ptFloat;
        } else if (!name.compare("floatwidth")) {
            p.first = ptFloatWidth;
        } else if (!name.compare("floatheight")) {
            p.first = ptFloatHeight;
        } else if (!name.compare("maxlines")) {
            p.first = ptMaxLines;
        } else if (!name.compare("columnwidth")) {
            p.first = ptColumnWidth;
        } else if (!name.compare("rowheight")) {
            p.first = ptRowHeight;
        } else if (!name.compare("overflow")) {
            p.first = ptOverflow;
        } else if (!name.compare("scaletvx")) {
            p.first = ptScaleTvX;
        } else if (!name.compare("scaletvy")) {
            p.first = ptScaleTvY;
        } else if (!name.compare("scaletvwidth")) {
            p.first = ptScaleTvWidth;
        } else if (!name.compare("scaletvheight")) {
            p.first = ptScaleTvHeight;
        } else if (!name.compare("cache")) {
            p.first = ptCache;
        } else if (!name.compare("determinatefont")) {
            p.first = ptDeterminateFont;
        } else {
            p.first = ptNone;
        }
        p.second = it->second;
        nativeParameters.insert(p);
    }
}

void cTemplateFunction::SetParameter(eParamType type, string value) {
    nativeParameters.erase(type);
    nativeParameters.insert(pair<eParamType, string>(type, value));
}

void cTemplateFunction::SetContainer(int x, int y, int w, int h) { 
    containerX = x; 
    containerY = y; 
    containerWidth = w; 
    containerHeight = h; 
}

void cTemplateFunction::SetLoopContainer(int columnWidth, int rowHeight) {
    this->columnWidth = columnWidth;
    this->rowHeight = rowHeight;
}

void cTemplateFunction::SetWidthManually(string width) {
    nativeParameters.erase(ptWidth);
    nativeParameters.insert(pair<eParamType, string>(ptWidth, width));
}

void cTemplateFunction::SetHeightManually(string height) {
    nativeParameters.erase(ptHeight);
    nativeParameters.insert(pair<eParamType, string>(ptHeight, height));
}

void cTemplateFunction::SetXManually(int newX) {
    numericParameters.erase(ptX);
    numericParameters.insert(pair<eParamType, int>(ptX, newX));
}

void cTemplateFunction::SetYManually(int newY) {
    numericParameters.erase(ptY);
    numericParameters.insert(pair<eParamType, int>(ptY, newY));
}

void cTemplateFunction::SetTextboxHeight(int boxHeight) {
    numericParameters.erase(ptHeight);
    numericParameters.insert(pair<eParamType, int>(ptHeight, boxHeight));
}

void cTemplateFunction::SetTranslatedText(string translation) {
    if (type != ftDrawText && type != ftDrawTextBox)
        return;
    if (translation.size() == 0)
        return;
    nativeParameters.erase(ptText);
    nativeParameters.insert(pair<eParamType, string>(ptText, translation));
}

void cTemplateFunction::SetMaxTextWidth(int maxWidth) {
    if (type != ftDrawText)
        return;
    numericParameters.erase(ptWidth);
    numericParameters.insert(pair<eParamType, int>(ptWidth, maxWidth));
}

bool cTemplateFunction::CalculateParameters(void) {
    bool paramValid = true;
    bool paramsValid = true;
    for (map< eParamType, string >::iterator param = nativeParameters.begin(); param != nativeParameters.end(); param++) { 
        paramValid = true;
        eParamType type = param->first;
        string value = param->second;
        switch (type) {
            case ptCond:
                paramValid = SetCondition(value);
                break;
            case ptX:
            case ptY:
            case ptWidth:
            case ptHeight:
            case ptMenuItemWidth:
            case ptFadeTime:
            case ptDelay:
            case ptFontSize:
            case ptLayer:
            case ptTransparency:
            case ptQuadrant:
            case ptType:
            case ptNumElements:
            case ptFloatWidth:
            case ptFloatHeight:
            case ptMaxLines:
            case ptColumnWidth:
            case ptRowHeight:
            case ptScaleTvX:
            case ptScaleTvY:
            case ptScaleTvWidth:
            case ptScaleTvHeight:
                SetNumericParameter(type, value);
                break;
            case ptAlign:
            case ptValign:
                paramValid = SetAlign(type, value);
                break;
            case ptImageType:
                paramValid = SetImageType(type, value);
                break;
            case ptColor:
                paramValid = SetColor(type, value);
                break;
            case ptFont:
                paramValid = SetFont(type, value);
                break;
            case ptText:
                paramValid = SetTextTokens(value);
                break;
            case ptScrollMode:
                paramValid = SetScrollMode(value);
                break;
            case ptScrollSpeed:
                paramValid = SetScrollSpeed(value);
                break;
            case ptOrientation:
                paramValid = SetOrientation(value);
                break;
            case ptFloat:
                paramValid = SetFloating(value);
                break;
            case ptOverflow:
                paramValid = SetOverflow(value);
            default:
                paramValid = true;
                break;
        }
        if (!paramValid) {
            paramsValid = false;
            esyslog("skindesigner: %s: invalid parameter %d, value %s", GetFuncName().c_str(), type, value.c_str());
        }
    }
    return paramsValid;
}    

bool cTemplateFunction::ReCalculateParameters(void) {
    bool paramValid = true;
    bool paramsValid = true;
    for (map< eParamType, string >::iterator param = nativeParameters.begin(); param != nativeParameters.end(); param++) { 
        paramValid = true;
        eParamType type = param->first;
        string value = param->second;
        switch (type) {
            case ptX:
            case ptY:
            case ptWidth:
            case ptHeight:
            case ptMenuItemWidth:
            case ptFontSize:
            case ptFloatWidth:
            case ptFloatHeight:
            case ptMaxLines:
            case ptColumnWidth:
            case ptRowHeight:
                SetNumericParameter(type, value);
                break;
        }
        if (!paramValid) {
            paramsValid = false;
            esyslog("skindesigner: %s: invalid parameter %d, value %s", GetFuncName().c_str(), type, value.c_str());
        }
    }
    return paramsValid;
}    

void cTemplateFunction::CompleteParameters(void) {
    switch (type) {
        case ftDrawImage: {
            CalculateAlign(GetNumericParameter(ptWidth), GetNumericParameter(ptHeight));
            if (imgPath.size() == 0) {
                imgPath = GetParameter(ptPath);
            }
            break; }
        case ftDrawRectangle:
            CalculateAlign(GetNumericParameter(ptWidth), GetNumericParameter(ptHeight));
            break;
        case ftDrawEllipse:
            CalculateAlign(GetNumericParameter(ptWidth), GetNumericParameter(ptHeight));
            break;
        case ftDrawSlope:
            CalculateAlign(GetNumericParameter(ptWidth), GetNumericParameter(ptHeight));
            break;
        case ftDrawText:
            CalculateAlign(GetWidth(), GetHeight());
            break;
        default:
            break;
    }
}

void cTemplateFunction::ClearDynamicParameters(void) {
    parsedText = "";
    cuttedText = "";
    alreadyCutted = false;
    parsedTextWidth = 0;
    textboxHeight = 0;

    //clear dynamically parsed int parameters
    for (map<eParamType,string>::iterator it = numericDynamicParameters.begin(); it != numericDynamicParameters.end(); it++) {
        numericParameters.erase(it->first);
    }
    //restoring dynamic numeric parameters only if x, y, width or height of other elements is needed dynamically
    for (map<eParamType,string>::iterator it = nativeParameters.begin(); it != nativeParameters.end(); it++) {
        eParamType paramType = it->first;
        if (paramType == ptX || 
            paramType == ptY || 
            paramType == ptWidth || 
            paramType == ptHeight || 
            paramType == ptFloatWidth || 
            paramType == ptFloatHeight) 
            {
            string value = it->second;
            if (value.find("{width(") != string::npos || value.find("{height(") != string::npos || value.find("{posx(") != string::npos || value.find("{posy(") != string::npos) {
                numericDynamicParameters.erase(paramType);
                SetNumericParameter(paramType, value);
            }
        }
    }

}

bool cTemplateFunction::ParseParameters(void) {
    updated = false;
    parsedCompletely = true;
    
    if (stringTokens) {
        ParseStringParameters();
    }

    if (intTokens && numericDynamicParameters.size() > 0) {
        ParseNumericalParameters();
    }
    
    if (condParam) {
        condParam->Evaluate(intTokens, stringTokens);
    }

    return parsedCompletely;
}

string cTemplateFunction::GetParameter(eParamType type) {
    map<eParamType,string>::iterator hit = nativeParameters.find(type);
    if (hit == nativeParameters.end())
        return "";
    return hit->second;
}

int cTemplateFunction::GetNumericParameter(eParamType type) {
    map<eParamType,int>::iterator hit = numericParameters.find(type);
    if (hit == numericParameters.end()) {
        //Set Default Value for Integer Parameters
        if (type == ptLayer)
            return 1;
        else if (type == ptTransparency)
            return 0;
        else if (type == ptDelay)
            return 0;
        else if (type == ptFadeTime)
            return 0;
        else if (type == ptMenuItemWidth)
            return 0;
        return -1;
    }
    return hit->second;
}

string cTemplateFunction::GetText(bool cut) { 
    if (!cut) {
        return parsedText;
    }
    if (alreadyCutted && cuttedText.size() > 0) {
        return cuttedText;        
    } 
    alreadyCutted = true;
    int maxWidth = GetNumericParameter(ptWidth);
    if (maxWidth > 0) {
        parsedTextWidth = fontManager->Width(fontName, GetNumericParameter(ptFontSize), parsedText.c_str());
        if (parsedTextWidth > maxWidth) {
            cuttedText = CutText(parsedText, maxWidth, fontName, GetNumericParameter(ptFontSize));
            return cuttedText;
        }
    }
    return parsedText; 
}


tColor cTemplateFunction::GetColorParameter(eParamType type) {
    map<eParamType,tColor>::iterator hit = colorParameters.find(type);
    if (hit == colorParameters.end())
        return 0x00000000;
    return hit->second;
}

int cTemplateFunction::GetWidth(bool cutted) {
    int funcWidth = 0;
    switch (type) {
        case ftDrawText: {
            if (cutted) {
                if (!alreadyCutted) {
                    alreadyCutted = true;
                    int maxWidth = GetNumericParameter(ptWidth);
                    if (maxWidth > 0) {
                        parsedTextWidth = fontManager->Width(fontName, GetNumericParameter(ptFontSize), parsedText.c_str());
                        if (parsedTextWidth > maxWidth) {
                            cuttedText = CutText(parsedText, maxWidth, fontName, GetNumericParameter(ptFontSize));
                        }
                    }
                }
                if (cuttedText.size() > 0)
                    return fontManager->Width(fontName, GetNumericParameter(ptFontSize), cuttedText.c_str());
            }
            if (parsedTextWidth > 0)
                funcWidth = parsedTextWidth;
            else
                funcWidth = fontManager->Width(fontName, GetNumericParameter(ptFontSize), parsedText.c_str());
            break; }
        case ftFill:
        case ftDrawImage:
        case ftDrawRectangle:
        case ftDrawEllipse:
        case ftDrawSlope:
        case ftDrawTextBox:
            funcWidth = GetNumericParameter(ptWidth);
            break;
        default:
            esyslog("skindesigner: GetWidth not implemented for funcType %d", type);
            break;
    }
    return funcWidth;
}

int cTemplateFunction::GetHeight(void) {
    int funcHeight = 0;
    switch (type) {
        case ftDrawText:
            funcHeight = fontManager->Height(fontName, GetNumericParameter(ptFontSize));
            break;
        case ftFill:
        case ftDrawImage:
        case ftDrawRectangle:
        case ftDrawEllipse:
        case ftDrawSlope:
            funcHeight = GetNumericParameter(ptHeight);
            break;
        case ftDrawTextBox: {
            int maxLines = GetNumericParameter(ptMaxLines);
            int fixedBoxHeight = GetNumericParameter(ptHeight); 
            if (maxLines > 0) {
                funcHeight = maxLines * fontManager->Height(fontName, GetNumericParameter(ptFontSize));
            } else if (fixedBoxHeight > 0) {
                funcHeight = fixedBoxHeight;
            } else if (textboxHeight > 0) {
                funcHeight = textboxHeight;
            } else {
                funcHeight = CalculateTextBoxHeight();
                textboxHeight = funcHeight;
            }
            break; }
        case ftLoop:
            //TODO: to be implemented
            break;
        default:
            esyslog("skindesigner: GetHeight not implemented for funcType %d", type);
            break;
    }
    return funcHeight;
}

void cTemplateFunction::GetNeededWidths(multimap<eParamType,string> *widths) {
    for (map<eParamType, string>::iterator param = numericDynamicParameters.begin(); param !=numericDynamicParameters.end(); param++) {
        string val = param->second;
        size_t posStart = val.find("{width(");
        while (posStart != string::npos) {
            size_t posEnd = val.find(")", posStart+1);
            if (posEnd != string::npos) {
                string label = val.substr(posStart+7, posEnd - posStart - 7);
                widths->insert(pair<eParamType,string>(param->first, label));
                val = val.replace(posStart, posEnd - posStart, "");
            } else {
                break;
            }
            posStart = val.find("{width(");
        }
    }
}

void cTemplateFunction::GetNeededHeights(multimap<eParamType,string> *heights) {
    for (map<eParamType, string>::iterator param = numericDynamicParameters.begin(); param !=numericDynamicParameters.end(); param++) {
        string val = param->second;
        size_t posStart = val.find("{height(");
        while (posStart != string::npos) {
            size_t posEnd = val.find(")", posStart + 1);
            if (posEnd != string::npos) {
                string label = val.substr(posStart + 8, posEnd - posStart - 8);
                heights->insert(pair<eParamType,string>(param->first, label));
                val = val.replace(posStart, posEnd - posStart, "");
            } else {
                break;
            }
            posStart = val.find("{height(");
        }
    }
}

void cTemplateFunction::GetNeededPosX(multimap<eParamType,string> *posXs) {
    for (map<eParamType, string>::iterator param = numericDynamicParameters.begin(); param !=numericDynamicParameters.end(); param++) {
        string val = param->second;
        size_t posStart = val.find("{posx(");
        while (posStart != string::npos) {
            size_t posEnd = val.find(")", posStart+1);
            if (posEnd != string::npos) {
                string label = val.substr(posStart+6, posEnd - posStart - 6);
                posXs->insert(pair<eParamType,string>(param->first, label));
                val = val.replace(posStart, posEnd - posStart, "");
            } else {
                break;
            }
            posStart = val.find("{posx(");
        }
    }
}

void cTemplateFunction::GetNeededPosY(multimap<eParamType,string> *posYs) {
    for (map<eParamType, string>::iterator param = numericDynamicParameters.begin(); param !=numericDynamicParameters.end(); param++) {
        string val = param->second;
        size_t posStart = val.find("{posy(");
        while (posStart != string::npos) {
            size_t posEnd = val.find(")", posStart+1);
            if (posEnd != string::npos) {
                string label = val.substr(posStart+6, posEnd - posStart - 6);
                posYs->insert(pair<eParamType,string>(param->first, label));
                val = val.replace(posStart, posEnd - posStart, "");
            } else {
                break;
            }
            posStart = val.find("{posy(");
        }
    }
}

void cTemplateFunction::SetWidth(eParamType type, string label, int funcWidth) {
    updated = false;
    map< eParamType, string >::iterator hit = numericDynamicParameters.find(type);
    if (hit == numericDynamicParameters.end())
        return;
    stringstream needle;
    needle << "{width(" << label << ")}";
    size_t posFound = (hit->second).find(needle.str());
    if (posFound == string::npos)
        return;
    stringstream repl;
    repl << funcWidth;
    string parsedVal = (hit->second).replace(posFound, needle.str().size(), repl.str());

    cNumericParameter param(parsedVal);
    param.SetAreaSize(containerWidth, containerHeight);
    param.SetGlobals(globals);
    int val = param.Parse(parsedVal);
    if (param.Valid()) {
        updated = true;
        numericParameters.insert(pair<eParamType, int>(type, val));
    } else {
        numericDynamicParameters.erase(type);
        numericDynamicParameters.insert(pair<eParamType, string>(type, parsedVal));     
    }
}

void cTemplateFunction::SetHeight(eParamType type, string label, int funcHeight) {
    updated = false;
    map< eParamType, string >::iterator hit = numericDynamicParameters.find(type);
    if (hit == numericDynamicParameters.end())
        return;
    stringstream needle;
    needle << "{height(" << label << ")}";
    size_t posFound = (hit->second).find(needle.str());
    if (posFound == string::npos)
        return;
    stringstream repl;
    repl << funcHeight;
    string parsedVal = (hit->second).replace(posFound, needle.str().size(), repl.str());

    cNumericParameter param(parsedVal);
    param.SetAreaSize(containerWidth, containerHeight);
    param.SetGlobals(globals);
    int val = param.Parse(parsedVal);
    if (param.Valid()) {
        updated = true;
        numericParameters.insert(pair<eParamType, int>(type, val));
    } else {
        numericDynamicParameters.erase(type);
        numericDynamicParameters.insert(pair<eParamType, string>(type, parsedVal));     
    }
}

void cTemplateFunction::SetX(eParamType type, string label, int funcX) {
    updated = false;
    map< eParamType, string >::iterator hit = numericDynamicParameters.find(type);
    if (hit == numericDynamicParameters.end())
        return;
    stringstream needle;
    needle << "{posx(" << label << ")}";
    size_t posFound = (hit->second).find(needle.str());
    if (posFound == string::npos)
        return;
    stringstream repl;
    repl << funcX;
    string parsedVal = (hit->second).replace(posFound, needle.str().size(), repl.str());

    cNumericParameter param(parsedVal);
    param.SetAreaSize(containerWidth, containerHeight);
    param.SetGlobals(globals);
    int val = param.Parse(parsedVal);
    if (param.Valid()) {
        updated = true;
        numericParameters.insert(pair<eParamType, int>(type, val));
    } else {
        numericDynamicParameters.erase(type);
        numericDynamicParameters.insert(pair<eParamType, string>(type, parsedVal));     
    }
}

void cTemplateFunction::SetY(eParamType type, string label, int funcY) {
    updated = false;
    map< eParamType, string >::iterator hit = numericDynamicParameters.find(type);
    if (hit == numericDynamicParameters.end())
        return;
    stringstream needle;
    needle << "{posy(" << label << ")}";
    size_t posFound = (hit->second).find(needle.str());
    if (posFound == string::npos)
        return;
    stringstream repl;
    repl << funcY;
    string parsedVal = (hit->second).replace(posFound, needle.str().size(), repl.str());

    cNumericParameter param(parsedVal);
    param.SetAreaSize(containerWidth, containerHeight);
    param.SetGlobals(globals);
    int val = param.Parse(parsedVal);
    if (param.Valid()) {
        updated = true;
        numericParameters.insert(pair<eParamType, int>(type, val));
    } else {
        numericDynamicParameters.erase(type);
        numericDynamicParameters.insert(pair<eParamType, string>(type, parsedVal));     
    }
}

bool cTemplateFunction::DoExecute(void) {
    if (!condParam)
        return true;
    return condParam->IsTrue();
}

/*******************************************************************
* Private Functions
*******************************************************************/

bool cTemplateFunction::SetCondition(string cond) {
    if (condParam)
        delete condParam;
    condParam = new cConditionalParameter(globals, cond);
    condParam->Tokenize();
    return true;
}


bool cTemplateFunction::SetNumericParameter(eParamType type, string value) {
    if (config.replaceDecPoint) {
        if (value.find_first_of('.') != string::npos) {
            std::replace( value.begin(), value.end(), '.', config.decPoint);
        }
    }

    cNumericParameter param(value);
    param.SetAreaSize(containerWidth, containerHeight);
    param.SetLoopContainer(columnWidth, rowHeight);
    param.SetGlobals(globals);
    switch (type) {
        case ptX:
        case ptWidth:
        case ptMenuItemWidth:
        case ptScaleTvX:
        case ptScaleTvWidth:
            param.SetHorizontal();
            break;
        case ptY:
        case ptHeight:
        case ptFontSize:
        case ptScaleTvY:
        case ptScaleTvHeight:
            param.SetVertical();
            break;
        case ptLayer:
            param.SetDefault(1);
            break;
        default:
            break;
    }
    string parsedValue = "";
    int val = param.Parse(parsedValue);
    if (param.Valid()) {
        if (this->type < ftLoop && type == ptX) {
            val += containerX;
        }
        if (this->type < ftLoop && type == ptY) {
            val += containerY;
        }
        numericParameters.erase(type);
        numericDynamicParameters.erase(type);
        numericParameters.insert(pair<eParamType, int>(type, val));
    } else {
        numericDynamicParameters.erase(type);
        numericDynamicParameters.insert(pair<eParamType, string>(type, parsedValue));
    }
    return param.Valid();
}

bool cTemplateFunction::SetAlign(eParamType type, string value) {
    eAlign align = alLeft;
    bool ok = true;
    if (!value.compare("center")) {
        align = alCenter;
    } else if (!value.compare("right")) {
        align = alRight;
    } else if (!value.compare("top")) {
        align = alTop;
    } else if (!value.compare("bottom")) {
        align = alBottom;
    } else if (!value.compare("left")) {
        align = alLeft;
    } else {
        ok = false;
    }
    numericParameters.insert(pair<eParamType, int>(type, align));
    return ok;  
}

bool cTemplateFunction::SetFont(eParamType type, string value) {
    //check if token
    if ((value.find("{") == 0) && (value.find("}") == (value.size()-1))) {
        value = value.substr(1, value.size()-2);
        map<string,string>::iterator hit = globals->fonts.find(value);
        if (hit != globals->fonts.end()) {
            fontName = hit->second;
        } else {
            map<string,string>::iterator def = globals->fonts.find("vdrOsd");
            if (def == globals->fonts.end())
                return false;
            fontName = def->second;
        }
    } else {
        //if no token, directly use input
        fontName = value;
    }
    return true;
}

bool cTemplateFunction::SetImageType(eParamType type, string value) {
    eImageType imgType = itImage;
    bool ok = true;
    if (!value.compare("channellogo")) {
        imgType = itChannelLogo;
    } else if (!value.compare("seplogo")) {
        imgType = itSepLogo;
    } else if (!value.compare("skinpart")) {
        imgType = itSkinPart;
    } else if (!value.compare("menuicon")) {
        imgType = itMenuIcon;
    } else if (!value.compare("icon")) {
        imgType = itIcon;
    } else if (!value.compare("image")) {
        imgType = itImage;
    } else {
        ok = false;
    }
    numericParameters.insert(pair<eParamType, int>(ptImageType, imgType));
    return ok;
}


bool cTemplateFunction::SetColor(eParamType type, string value) {
    if (globals) {
        for (map<string, tColor>::iterator col = globals->colors.begin(); col != globals->colors.end(); col++) {
            stringstream sColName;
            sColName << "{" << col->first << "}";
            string colName = sColName.str();
            if (!colName.compare(value)) {
                tColor colVal = col->second;
                colorParameters.insert(pair<eParamType, tColor>(type, colVal));
                return true;
            }
        }
    }
    if (value.size() != 8)
        return false;
    std::stringstream str;
    str << value;
    tColor colVal;
    str >> std::hex >> colVal;
    colorParameters.insert(pair<eParamType, tColor>(type, colVal));
    return true;
}

bool cTemplateFunction::SetTextTokens(string value) {
    textTokens.clear();
    //first replace globals
    for (map<string,string>::iterator globStr = globals->stringVars.begin(); globStr != globals->stringVars.end(); globStr++) {
        stringstream sToken;
        sToken << "{" << globStr->first << "}";
        string token = sToken.str();
        size_t foundToken = value.find(token);
        if (foundToken != string::npos) {
            value = value.replace(foundToken, token.size(), globStr->second);
        }
    }
    //now tokenize
    bool tokenFound = true;
    while (tokenFound) {
        //search for conditional token or normal token
        size_t tokenStart = value.find_first_of('{');
        size_t conditionStart = value.find_first_of('|');
        size_t printfStart = value.find("{printf(");
        if (tokenStart == string::npos && conditionStart == string::npos) {
        //find constant strings
            if (value.size() > 0) {
                cTextToken token;
                token.type = ttConstString;
                token.value = value;
                textTokens.push_back(token);
            }
            tokenFound = false;
            continue;
        } else if (tokenStart != string::npos && conditionStart == string::npos) {
            size_t tokenEnd = value.find_first_of('}');
            if (printfStart != string::npos && printfStart <= tokenStart) {
                //replace printf text token
                ParsePrintfTextToken(value, printfStart, tokenEnd);
            } else {
                //replace normal text token
                ParseTextToken(value, tokenStart, tokenEnd);
            }
        } else if (tokenStart != string::npos && conditionStart != string::npos) {
            if (tokenStart < conditionStart) {
                size_t tokenEnd = value.find_first_of('}');
                if (printfStart != string::npos && printfStart <= tokenStart) {
                    //replace printf text token
                    ParsePrintfTextToken(value, printfStart, tokenEnd);
                } else {
                    //replace normal text token
                    ParseTextToken(value, tokenStart, tokenEnd);
                }
            } else {
                //replace conditional text token
                size_t conditionEnd = value.find_first_of('|', conditionStart+1);
                ParseConditionalTextToken(value, conditionStart, conditionEnd);
            }
        }
    }
    return true;
}

void cTemplateFunction::ParseTextToken(string &value, size_t start, size_t end) {
    cTextToken token;
    if (start > 0) {
        string constString = value.substr(0, start);
        value = value.replace(0, start, "");
        token.type = ttConstString;
        token.value = constString;
        textTokens.push_back(token);
    } else {
        string tokenName = value.substr(1, end - start - 1);
        value = value.replace(0, end - start + 1, "");
        token.type = ttToken;
        token.value = tokenName;
        textTokens.push_back(token);
    }
}

void cTemplateFunction::ParseConditionalTextToken(string &value, size_t start, size_t end) {
    cTextToken token;
    if (start > 0) {
        string constString = value.substr(0, start);
        value = value.replace(0, start, "");
        token.type = ttConstString;
        token.value = constString;
        textTokens.push_back(token);
    } else {
        string condToken = value.substr(start + 1, end - start - 1);
        value = value.replace(0, end - start + 1, "");
        size_t tokenStart = condToken.find_first_of('{');
        size_t tokenEnd = condToken.find_first_of('}');
        vector<cTextToken> subTokens;
        if (tokenStart > 0) {
            cTextToken subToken;
            string constString = condToken.substr(0, tokenStart);
            condToken = condToken.replace(0, tokenStart, "");
            subToken.type = ttConstString;
            subToken.value = constString;
            subTokens.push_back(subToken);
        }
        string tokenName = condToken.substr(1, tokenEnd - tokenStart - 1);
        condToken = condToken.replace(0, tokenEnd - tokenStart + 1, "");
        
        cTextToken subToken2;
        subToken2.type = ttToken;
        subToken2.value = tokenName;
        subTokens.push_back(subToken2);

        if (condToken.size() > 0) {
            cTextToken subToken3;
            subToken3.type = ttConstString;
            subToken3.value = condToken;
            subTokens.push_back(subToken3);
        }

        token.type = ttConditionalToken;
        token.value = tokenName;
        token.subTokens = subTokens;
        textTokens.push_back(token);
    }

}

void cTemplateFunction::ParsePrintfTextToken(string &value, size_t start, size_t end) {
    cTextToken token;
    if (start > 0) {
        string constString = value.substr(0, start);
        value = value.replace(0, start, "");
        token.type = ttConstString;
        token.value = constString;
        textTokens.push_back(token);
    } else {
        token.type = ttPrintfToken;
        //fetch parameter list from printf
        string printfParams = value.substr(start + 8, end - start - 9);
        value = value.replace(0, end - start + 1, "");
        splitstring s(printfParams.c_str());
        vector<string> flds = s.split(',', 1);

        int numParams = flds.size();
        if (numParams < 1)
            return;
        string formatString = trim(flds[0]);
        token.value = formatString.substr(1, formatString.size() - 2);
        for (int i=1; i < numParams; i++) {
            token.parameters.push_back(trim(flds[i]));
        }
    }
    textTokens.push_back(token);
}

bool cTemplateFunction::SetScrollMode(string value) {
    eScrollMode mode = smNone;
    bool ok = true;
    if (!value.compare("forthandback"))
        mode = smForthAndBack;
    else if (!value.compare("carriagereturn"))
        mode = smCarriageReturn;
    else
        ok = false;
    numericParameters.insert(pair<eParamType, int>(ptScrollMode, mode));
    return ok;
}

bool cTemplateFunction::SetScrollSpeed(string value) {
    eScrollSpeed speed = ssMedium;
    bool ok = true;
    if (!value.compare("slow"))
        speed = ssSlow;
    else if (!value.compare("fast"))
        speed = ssFast;
    else if (!value.compare("medium"))
        speed = ssMedium;
    else
        ok = false;
    numericParameters.insert(pair<eParamType, int>(ptScrollSpeed, speed));
    return ok;

}

bool cTemplateFunction::SetOrientation(string value) {
    eOrientation orientation = orNone;
    bool ok = true;
    if (!value.compare("horizontal"))
        orientation = orHorizontal;
    else if (!value.compare("vertical"))
        orientation = orVertical;
    else if (!value.compare("absolute"))
        orientation = orAbsolute;
    else
        ok = false;
    numericParameters.insert(pair<eParamType, int>(ptOrientation, orientation));
    return ok;
}

bool cTemplateFunction::SetFloating(string value) {
    eFloatType floatType = flNone;
    bool ok = true;
    if (!value.compare("topleft"))
        floatType = flTopLeft;
    else if (!value.compare("topright"))
        floatType = flTopRight;
    else
        ok = false;
    numericParameters.insert(pair<eParamType, int>(ptFloat, floatType));
    return ok;
}

bool cTemplateFunction::SetOverflow(string value) {
    eOverflowType overflowType = otNone;
    bool ok = true;
    if (!value.compare("linewrap"))
        overflowType = otWrap;
    else if (!value.compare("cut"))
        overflowType = otCut;
    else
        ok = false;
    numericParameters.insert(pair<eParamType, int>(ptOverflow, overflowType));
    return ok;
}

void cTemplateFunction::ParseStringParameters(void) {
    //first replace stringtokens in Text (drawText)
    stringstream text;
    for (vector<cTextToken>::iterator it = textTokens.begin(); it !=textTokens.end(); it++) {
        updated = true;
        if ((*it).type == ttConstString) {
            text << (*it).value;
        } else if ((*it).type == ttToken) {
            bool found = false;
            string tokenName = (*it).value;
            if (stringTokens) {
                map < string, string >::iterator hit = stringTokens->find(tokenName);
                if (hit != stringTokens->end()) {
                    text << hit->second;
                    found = true;
                }
            }
            if (!found && intTokens) {
                map < string, int >::iterator hitInt = intTokens->find(tokenName);
                if (hitInt != intTokens->end()) {
                    text << hitInt->second;
                    found = true;
                }
            }
            if (!found) {
                text << "{" << tokenName << "}";
            }
        } else if ((*it).type == ttConditionalToken) {
            bool found = false;
            string tokenName = (*it).value;
            if (stringTokens) {
                map < string, string >::iterator hit = stringTokens->find(tokenName);
                if (hit != stringTokens->end()) {
                    string replaceText = hit->second;
                    if (replaceText.size() > 0) {
                        for (vector<cTextToken>::iterator it2 = (*it).subTokens.begin(); it2 != (*it).subTokens.end(); it2++) {
                            if ((*it2).type == ttConstString) {
                                text << (*it2).value;
                            } else {
                                text << replaceText;
                            }
                        }                       
                    }
                    found = true;
                }
            }
            if (!found && intTokens) {
                map < string, int >::iterator hitInt = intTokens->find(tokenName);
                if (hitInt != intTokens->end()) {
                    int intVal = hitInt->second;
                    if (intVal > 0) {
                        for (vector<cTextToken>::iterator it2 = (*it).subTokens.begin(); it2 != (*it).subTokens.end(); it2++) {
                            if ((*it2).type == ttConstString) {
                                text << (*it2).value;
                            } else {
                                text << intVal;
                            }
                        }                       
                    }
                    found = true;
                }
            }
        } else if ((*it).type == ttPrintfToken) {
            cTextToken token = *it;
            int paramCount = token.parameters.size();
            string printfResult = "";
            switch (paramCount) {
                case 1: {
                    int param1 = ReplaceIntToken(token.parameters[0]);
                    printfResult = *cString::sprintf(token.value.c_str(), param1);
                    break; }
                case 2: {
                    int param1 = ReplaceIntToken(token.parameters[0]);
                    int param2 = ReplaceIntToken(token.parameters[1]);
                    printfResult = *cString::sprintf(token.value.c_str(), param1, param2);
                    break; }
                case 3: {
                    int param1 = ReplaceIntToken(token.parameters[0]);
                    int param2 = ReplaceIntToken(token.parameters[1]);
                    int param3 = ReplaceIntToken(token.parameters[2]);
                    printfResult = *cString::sprintf(token.value.c_str(), param1, param2, param3);
                    break; }
                case 4: {
                    int param1 = ReplaceIntToken(token.parameters[0]);
                    int param2 = ReplaceIntToken(token.parameters[1]);
                    int param3 = ReplaceIntToken(token.parameters[2]);
                    int param4 = ReplaceIntToken(token.parameters[3]);
                    printfResult = *cString::sprintf(token.value.c_str(), param1, param2, param3, param4);
                    break; }
                case 5: {
                    int param1 = ReplaceIntToken(token.parameters[0]);
                    int param2 = ReplaceIntToken(token.parameters[1]);
                    int param3 = ReplaceIntToken(token.parameters[2]);
                    int param4 = ReplaceIntToken(token.parameters[3]);
                    int param5 = ReplaceIntToken(token.parameters[4]);
                    printfResult = *cString::sprintf(token.value.c_str(), param1, param2, param3, param4, param5);
                    break; }
                default:
                    break;
            }
            text << printfResult;
        }
    }
    parsedText = text.str();

    string path = GetParameter(ptPath);
    if (GetNumericParameter(ptImageType) == itImage && path.size() > 0) {
        //no absolute pathes allowed
        if (!startswith(path.c_str(), "{")) {
            esyslog("skindesigner: no absolute pathes allowed for images - %s", path.c_str());
        }
        if (startswith(path.c_str(), "{ressourcedir}")) {
            imgPath = path.replace(0, 14, *config.GetSkinRessourcePath());
        }
    }
    //now check further possible string variables
    if (stringTokens && path.size() > 0 && path.find("{") != string::npos) {
        for (map < string, string >::iterator it = stringTokens->begin(); it != stringTokens->end(); it++) {
            size_t found = path.find(it->first);
            if (found != string::npos) {
                updated = true;
                imgPath = path;
                if (found > 0 && ((it->first).size() + 2 <= imgPath.size()))
                    imgPath.replace(found-1, (it->first).size() + 2, it->second);
                break;
            }
        }
    }
}

void cTemplateFunction::ParseNumericalParameters(void) {
    parsedCompletely = true;
    for (map<eParamType, string>::iterator param = numericDynamicParameters.begin(); param !=numericDynamicParameters.end(); param++) {
        string val = param->second;
        for (map<string,int >::iterator tok = intTokens->begin(); tok != intTokens->end(); tok++) {
            stringstream sToken;
            sToken << "{" << tok->first << "}";
            string token = sToken.str();
            size_t foundToken = val.find(token);
            //replace token as often as it appears
            while (foundToken != string::npos) {
                stringstream sVal;
                sVal << tok->second;
                val = val.replace(foundToken, token.size(), sVal.str());
                foundToken = val.find(token);
            }
        }
        cNumericParameter p(val);
        string parsedVal = "";
        int newVal = p.Parse(parsedVal);
        if (p.Valid()) {
            updated = true;
            numericParameters.insert(pair<eParamType, int>(param->first, newVal));
        } else {
            parsedCompletely = false;
        }
    }
}

void cTemplateFunction::CalculateAlign(int elementWidth, int elementHeight) {
    int align = GetNumericParameter(ptAlign);
    //if element is used in a loop, use loop box width
    int boxWidth = (columnWidth > 0) ? columnWidth : containerWidth;
    int boxHeight = (rowHeight > 0) ? rowHeight : containerHeight;
    if (align == alCenter) {
        int xNew = (boxWidth - elementWidth) / 2;
        numericParameters.erase(ptX);
        numericParameters.insert(pair<eParamType,int>(ptX, xNew));
    } else if (align == alRight) {
        int xNew = boxWidth - elementWidth;
        numericParameters.erase(ptX);
        numericParameters.insert(pair<eParamType,int>(ptX, xNew));
    }

    int vAlign = GetNumericParameter(ptValign);
    if (vAlign == alCenter) {
        int yNew = (boxHeight - elementHeight) / 2;
        numericParameters.erase(ptY);
        numericParameters.insert(pair<eParamType,int>(ptY, yNew));
    } else if (vAlign == alBottom) {
        int yNew = boxHeight - elementHeight;
        numericParameters.erase(ptY);
        numericParameters.insert(pair<eParamType,int>(ptY, yNew));
    }
}

int cTemplateFunction::CalculateTextBoxHeight(void) {
    int width = GetNumericParameter(ptWidth);
    string fontName = GetFontName();
    int fontSize = GetNumericParameter(ptFontSize);
    string text = GetText(false);
    const cFont *font = fontManager->Font(fontName, fontSize);
    if (!font)
        return 0;

    int fontHeight = fontManager->Height(fontName, fontSize);
    int floatType = GetNumericParameter(ptFloat);

    if (floatType == flNone) {
        cTextWrapper wrapper;
        wrapper.Set(text.c_str(), font, width);
        int lines = wrapper.Lines();
        return (lines * fontHeight);
    }

    int floatWidth = GetNumericParameter(ptFloatWidth);
    int floatHeight = GetNumericParameter(ptFloatHeight);
    
    cTextWrapper wTextTall;
    cTextWrapper wTextFull;
    
    int linesNarrow = floatHeight / fontHeight;
    int widthNarrow = width - floatWidth;
    int linesDrawn = 0;
    int curY = 0;
    bool drawNarrow = true;
    
    splitstring s(text.c_str());
    vector<string> flds = s.split('\n', 1);

    if (flds.size() < 1)
        return 0;

    stringstream sstrTextTall;
    stringstream sstrTextFull;

    for (int i=0; i<flds.size(); i++) {
        if (!flds[i].size()) {
            //empty line
            linesDrawn++;
            curY += fontHeight;
            if (drawNarrow)
                sstrTextTall << "\n";
            else
                sstrTextFull << "\n";
        } else {
            cTextWrapper wrapper;
            if (drawNarrow) {
                wrapper.Set((flds[i].c_str()), font, widthNarrow);
                int newLines = wrapper.Lines();
                //check if wrapper fits completely into narrow area
                if (linesDrawn + newLines < linesNarrow) {
                    for (int line = 0; line < wrapper.Lines(); line++) {
                        sstrTextTall << wrapper.GetLine(line) << " ";
                    }
                    sstrTextTall << "\n";
                    linesDrawn += newLines;
                } else {
                    //this wrapper has to be splitted
                    for (int line = 0; line < wrapper.Lines(); line++) {
                        if (line + linesDrawn < linesNarrow) {
                            sstrTextTall << wrapper.GetLine(line) << " ";
                        } else {
                            sstrTextFull << wrapper.GetLine(line) << " ";
                        }
                    }
                    sstrTextFull << "\n";
                    drawNarrow = false;
                }
            } else {
                wrapper.Set((flds[i].c_str()), font, width);
                for (int line = 0; line < wrapper.Lines(); line++) {
                    sstrTextFull << wrapper.GetLine(line) << " ";        
                }
                sstrTextFull << "\n";
            }
        }
    }
    wTextTall.Set(sstrTextTall.str().c_str(), font, widthNarrow);
    wTextFull.Set(sstrTextFull.str().c_str(), font, width);

    int textLinesTall = wTextTall.Lines();
    int textLinesFull = wTextFull.Lines();

    return ((textLinesTall+textLinesFull) * fontHeight);
}

int cTemplateFunction::ReplaceIntToken(string intTok) {
    if (intTokens) {
        map<string,int>::iterator hit = intTokens->find(intTok);
        if (hit != intTokens->end())
            return hit->second;
    }
    if (stringTokens) {
        map<string,string>::iterator hit = stringTokens->find(intTok);
        if (hit != stringTokens->end()) {            
            return atoi(hit->second.c_str());
        }
    }
    return 0;
}

/*******************************************************************
* Helper Functions
*******************************************************************/

string cTemplateFunction::GetFuncName(void) {
    string name;
    switch (type) {
        case ftOsd:
            name = "OSD Parameters";
            break;
        case ftView:
            name = "View Parameters";
            break;
        case ftViewElement:
            name = "View Element Parameters";
            break;
        case ftPixmap:
            name = "Pixmap Parameters";
            break;
        case ftPixmapScroll:
            name = "ScrollPixmap Parameters";
            break;
        case ftLoop:
            name = "Looping Function";
            break;
        case ftFill:
            name = "Function Fill";
            break;
        case ftDrawText:
            name = "Function DrawText";
            break;
        case ftDrawTextBox:
            name = "Function DrawTextBox";
            break;
        case ftDrawImage:
            name = "Function DrawImage";
            break;
        case ftDrawRectangle:
            name = "Function DrawRectangle";
            break;
        case ftDrawEllipse:
            name = "Function DrawEllipse";
            break;
        case ftDrawSlope:
            name = "Function DrawSlope";
            break;
        case ftNone:
            name = "Undefined";
            break;
        default:
            name = "Unknown";
            break;
    };
    return name;
}

string cTemplateFunction::GetParamName(eParamType pt) {
    string name;
    switch (pt) {
        case ptCond:
            name = "Condition";
            break;
        case ptName:
            name = "Name";
            break;
        case ptX:
            name = "X";
            break;
        case ptY:
            name = "Y";
            break;
        case ptWidth:
            name = "Width";
            break;
        case ptHeight:
            name = "Height";
            break;
        case ptMenuItemWidth:
            name = "Menu Item Width";
            break;          
        case ptFadeTime:
            name = "Fade Time";
            break;
        case ptDelay:
            name = "Delay";
            break;
        case ptImageType:
            name = "Image Type";
            break;
        case ptPath:
            name = "Image Path";
            break;
        case ptColor:
            name = "Color";
            break;
        case ptFont:
            name = "Font";
            break;
        case ptFontSize:
            name = "FontSize";
            break;
        case ptText:
            name = "Text";
            break;
        case ptLayer:
            name = "Layer";
            break;
        case ptTransparency:
            name = "Transparency";
            break;
        case ptQuadrant:
            name = "Quadrant";
            break;
        case ptType:
            name = "Type";
            break;
        case ptAlign:
            name = "Align";
            break;
        case ptValign:
            name = "Vertical Align";
            break;
        case ptScrollMode:
            name = "Scroll Mode";
            break;
        case ptScrollSpeed:
            name = "Scroll Speed";
            break;
        case ptOrientation:
            name = "Orientation";
            break;
        case ptNumElements:
            name = "NumElements";
            break;
        case ptScrollElement:
            name = "Scroll Element";
            break;
        case ptScrollHeight:
            name = "Scroll Height";
            break;
        case ptFloat:
            name = "Floating Type";
            break;
        case ptFloatWidth:
            name = "Floating Width";
            break;
        case ptFloatHeight:
            name = "Floating Height";
            break;
        case ptMaxLines:
            name = "Max num of lines";
            break;
        case ptColumnWidth:
            name = "Column Width";
            break;
        case ptRowHeight:
            name = "Row Height";
            break;
        case ptOverflow:
            name = "Overflow Mode";
            break;
        case ptScaleTvX:
            name = "Scale TV Picture X";
            break;
        case ptScaleTvY:
            name = "Scale TV Picture Y";
            break;
        case ptScaleTvWidth:
            name = "Scale TV Picture Width";
            break;
        case ptScaleTvHeight:
            name = "Scale TV Picture Height";
            break;
        case ptCache:
            name = "Cache Image";
            break;
        case ptDeterminateFont:
            name = "Determinate Font";
            break;    
        default:
            name = "Unknown";
            break;
    };
    return name;
}

void cTemplateFunction::Debug(void) {
    esyslog("skindesigner: Debugging %s, Container: x = %d, y = %d, Size: %dx%d", GetFuncName().c_str(), containerX, containerY, containerWidth, containerHeight);
    esyslog("skindesigner: --- Native Parameters:");

    if (condParam) {
        condParam->Debug();
    }
    esyslog("skindesigner: --- Native Parameters:");
    for (map<eParamType,string>::iterator it = nativeParameters.begin(); it != nativeParameters.end(); it++) {
        esyslog("skindesigner: \"%s\" = \"%s\"", GetParamName(it->first).c_str(), (it->second).c_str());
    }
    if (numericParameters.size() > 0) {
        esyslog("skindesigner: --- Integer Parameters: ");
        for (map<eParamType,int>::iterator it = numericParameters.begin(); it != numericParameters.end(); it++) {
            esyslog("skindesigner: %s = %d", GetParamName(it->first).c_str(), it->second);
        }
    }
    if (numericDynamicParameters.size() > 0) {
        esyslog("skindesigner: --- Dynamic Integer Parameters: ");
        for (map<eParamType,string>::iterator it = numericDynamicParameters.begin(); it != numericDynamicParameters.end(); it++) {
            esyslog("skindesigner: %s = %s", GetParamName(it->first).c_str(), (it->second).c_str());
        }
    }
    if (colorParameters.size() > 0) {
        esyslog("skindesigner: --- Color Parameters:");
        for (map<eParamType,tColor>::iterator it = colorParameters.begin(); it != colorParameters.end(); it++) {
            esyslog("skindesigner: %s = %x", GetParamName(it->first).c_str(), it->second);
        }
    }
    if (textTokens.size() > 0) {
        esyslog("skindesigner: --- Text Tokens:");
        int i=0;
        for (vector<cTextToken>::iterator it = textTokens.begin(); it != textTokens.end(); it++) {
            eTextTokenType tokenType = (*it).type;
            string tokType = "";
            if (tokenType == ttConstString)
                tokType = "Const: ";
            else if (tokenType == ttToken)
                tokType = "Token: ";
            else if (tokenType == ttConditionalToken)
                tokType = "Conditional Token: ";
            else if (tokenType == ttPrintfToken)
                tokType = "PrintF Token: ";
            esyslog("skindesigner: %s %d = \"%s\"", tokType.c_str(), i++, (*it).value.c_str());
            if (tokenType == ttConditionalToken) {
                for (vector<cTextToken>::iterator it2 = (*it).subTokens.begin(); it2 != (*it).subTokens.end(); it2++) {
                    eTextTokenType tokenTypeCond = (*it2).type;
                    string tokTypeCond = "";
                    if (tokenTypeCond == ttConstString)
                        tokTypeCond = "Const: ";
                    else if (tokenTypeCond == ttToken)
                        tokTypeCond = "Token: ";
                    esyslog("skindesigner: %s \"%s\"", tokTypeCond.c_str(), (*it2).value.c_str());
                }
            }
            if (tokenType == ttPrintfToken) {
                for (vector<string>::iterator it2 = (*it).parameters.begin(); it2 != (*it).parameters.end(); it2++) {
                    esyslog("skindesigner: Printf parameter: %s", (*it2).c_str());
                }                
            }
        }
    }
    if (fontName.size() > 0) {
        esyslog("skindesigner: --- Font Name: \"%s\"", fontName.c_str());
    }
    if (parsedText.size() > 0) {
        esyslog("skindesigner: --- Parsed Text: \"%s\"", parsedText.c_str());
    }
    if (type == ftDrawText) {
        esyslog("skindesigner: --- Cutted Text: \"%s\"", cuttedText.c_str());
        esyslog("skindesigner: --- Parsed Text Width: %d", parsedTextWidth);
        esyslog("skindesigner: --- Already Cutted: %s", alreadyCutted ? "true" : "false");
    }
    if (imgPath.size() > 0) {
        esyslog("skindesigner: --- Image Path: \"%s\"", imgPath.c_str());
    }
}
