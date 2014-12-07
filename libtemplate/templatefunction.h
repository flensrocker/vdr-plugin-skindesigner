#ifndef __TEMPLATEFUNCTION_H
#define __TEMPLATEFUNCTION_H

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
#include "parameter.h"

using namespace std;

// --- cTemplateFunction -------------------------------------------------------------

enum eFuncType {
    ftOsd,
    ftView,
    ftViewElement,
    ftViewList,
    ftPixmap,
    ftPixmapScroll,
    ftLoop,
    ftFill,
    ftDrawText,
    ftDrawTextBox,
    ftDrawImage,
    ftDrawRectangle,
    ftDrawEllipse,
    ftDrawSlope,
    ftNone
};

enum eParamType {
    ptCond,
    ptName,
    ptX,
    ptY,
    ptWidth,
    ptHeight,
    ptMenuItemWidth,
    ptFadeTime,
    ptDelay,
    ptImageType,
    ptPath,
    ptColor,
    ptFont,
    ptFontSize,
    ptText,
    ptLayer,
    ptTransparency,
    ptQuadrant,
    ptType,
    ptAlign,
    ptValign,
    ptScrollMode,
    ptScrollSpeed,
    ptOrientation,
    ptNumElements,
    ptScrollElement,
    ptScrollHeight,
    ptFloat,
    ptFloatWidth,
    ptFloatHeight,
    ptMaxLines,
    ptColumnWidth,
    ptRowHeight,
    ptOverflow,
    ptScaleTvX,
    ptScaleTvY,
    ptScaleTvWidth,
    ptScaleTvHeight,
    ptCache,
    ptDeterminateFont,
    ptNone
};

enum eImageType {
    itChannelLogo,
    itSepLogo,
    itSkinPart,
    itMenuIcon,
    itIcon,
    itImage
};

enum eFloatType {
    flNone,
    flTopLeft,
    flTopRight
};

enum eOverflowType {
    otNone,
    otWrap,
    otCut
};

class cTemplateFunction {
protected:
    eFuncType type;
    bool debug;
    int containerX;                                                       //X of parent container
    int containerY;                                                       //Y of parent container
    int containerWidth;                                                   //width of parent container
    int containerHeight;                                                  //height of parent container
    int columnWidth;                                                      //if func is executed in a loop, width of loop column
    int rowHeight;                                                        //if func is executed in a loop, height of loop row
    cGlobals *globals;                                                    //globals
    map< eParamType, string > nativeParameters;                           //native parameters directly from xml 
    map< eParamType, int > numericParameters;                             //sucessfully parsed numeric parameters
    map< eParamType, string > numericDynamicParameters;                   //numeric parameters with dynamic tokens
    bool parsedCompletely;
    bool updated;
    map< eParamType, tColor > colorParameters;
    cConditionalParameter *condParam;
    //drawimage parameters
    string imgPath;
    //drawtext parameters
    string fontName;
    vector<cTextToken> textTokens;
    string parsedText;
    int parsedTextWidth;
    string cuttedText;
    bool alreadyCutted;
    //drawtextbox parameters
    int textboxHeight;
    //dynamic tokens
    map < string, string > *stringTokens;
    map < string, int > *intTokens;
    //private functions
    bool SetCondition(string cond);
    bool SetNumericParameter(eParamType type, string value);
    bool SetAlign(eParamType type, string value);
    bool SetFont(eParamType type, string value);
    bool SetImageType(eParamType type, string value);
    bool SetColor(eParamType type, string value);
    bool SetTextTokens(string value);
    void ParseTextToken(string &value, size_t start, size_t end);
    void ParseConditionalTextToken(string &value, size_t start, size_t end);
    void ParsePrintfTextToken(string &value, size_t start, size_t end);
    bool SetScrollMode(string value);
    bool SetScrollSpeed(string value);
    bool SetOrientation(string value);
    bool SetFloating(string value);
    bool SetOverflow(string value);
    void ParseStringParameters(void);
    void ParseNumericalParameters(void);
    void CalculateAlign(int elementWidth, int elementHeight);
    int CalculateTextBoxHeight(void);
    int ReplaceIntToken(string intTok);
public:
    cTemplateFunction(eFuncType type);
    virtual ~cTemplateFunction(void);
    //Setter Functions
    void SetParameters(vector<pair<string, string> > params);
    void SetParameter(eParamType type, string value);
    void SetContainer(int x, int y, int w, int h);
    void SetLoopContainer(int columnWidth, int rowHeight);
    void SetWidthManually(string width);
    void SetHeightManually(string height);
    void SetXManually(int newX);
    void SetYManually(int newY);
    void SetMaxTextWidth(int maxWidth);
    void SetTextboxHeight(int boxHeight);
    void SetGlobals(cGlobals *globals) { this->globals = globals; };
    void SetTranslatedText(string translation);
    //PreCache Parameters 
    bool CalculateParameters(void);
    //Complete Parameters 
    bool ReCalculateParameters(void);
    void CompleteParameters(void);
    //Set and Unset Dynamic Tokens from view
    void SetStringTokens(map < string, string > *tok) { stringTokens = tok; };
    void SetIntTokens(map < string, int > *tok) { intTokens = tok; };
    void UnsetIntTokens(void) { intTokens = NULL; };
    void UnsetStringTokens(void) { stringTokens = NULL; };
    //Clear dynamically parameters
    void ClearDynamicParameters(void);
    //Parse parameters with dynamically set Tokens
    bool ParseParameters(void);
    //Getter Functions
    eFuncType GetType(void) { return type; };
    bool DoDebug(void) { return debug; };
    string GetParameter(eParamType type);
    int GetNumericParameter(eParamType type);
    string GetText(bool cut = true);
    string GetImagePath(void) { return imgPath; };
    tColor GetColorParameter(eParamType type);
    string GetFontName(void) { return fontName; };
    string GetFuncName(void);
    string GetParamName(eParamType pt);
    //Dynamic width or height parameter
    int GetWidth(bool cutted = true);
    int GetHeight(void);
    void GetNeededWidths(multimap<eParamType,string> *widths);
    void GetNeededHeights(multimap<eParamType,string> *heights);
    void GetNeededPosX(multimap<eParamType,string> *posXs);
    void GetNeededPosY(multimap<eParamType,string> *posYs);
    void SetWidth(eParamType type, string label, int funcWidth);
    void SetHeight(eParamType type, string label, int funcHeight);
    void SetX(eParamType type, string label, int funcX);
    void SetY(eParamType type, string label, int funcY);
    //Status Functions
    bool ParsedCompletely(void) { return parsedCompletely; };
    bool DoExecute(void);
    bool Updated(void) { return updated; };
    //Debug
    void Debug(void);
};

#endif //__TEMPLATEFUNCTION_H