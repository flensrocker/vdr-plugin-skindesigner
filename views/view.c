#include "view.h"
#include "../config.h"
#include "../libcore/helpers.h"
#include "../libcore/imageloader.h"

using namespace std;

cView::cView(cTemplateView *tmplView) : cPixmapContainer(tmplView->GetNumPixmaps()) {
    this->tmplView = tmplView;
    tvScaled = tmplView->GetScalingWindow(scalingWindow);
    if (tvScaled) {
        cDevice::PrimaryDevice()->ScaleVideo(scalingWindow);
    }
    tmplItem = NULL;
    tmplTab = NULL;
    Init();
}

cView::cView(cTemplateViewElement *tmplItem) : cPixmapContainer(tmplItem->GetNumPixmaps()) {
    this->tmplItem = tmplItem;
    tmplView = NULL;
    tmplTab = NULL;
    tvScaled = false;
    Init();
}

cView::cView(cTemplateViewTab *tmplTab) : cPixmapContainer(1) {
    this->tmplTab = tmplTab;
    tmplView = NULL;
    tmplItem = NULL;
    tvScaled = false;
    Init();
}

cView::~cView() {
    if (tvScaled) {
        cDevice::PrimaryDevice()->ScaleVideo(cRect::Null);
    }
}

void cView::Init(void) {
    viewInit = true;
    scrolling = false;
    veScroll = veUndefined;
    scrollingPix = -1;
    scrollOrientation = orHorizontal;
    scrollDelay = 0;
    scrollMode = smNone;
    scrollSpeed = ssMedium;
    currentlyScrolling = false;
}

void cView::Action(void) {
    if (scrolling) {
        DoSleep(scrollDelay);
        if (scrollOrientation == orHorizontal) {
            ActivateScrolling();
            ScrollHorizontal(scrollingPix, scrollDelay, scrollSpeed, scrollMode);
        } else {
            ScrollVertical(scrollingPix, scrollDelay, scrollSpeed);
        }
    }
}

void cView::Stop(void) {
    CancelSave();
}

/********************************************************************************
* Protected Functions
********************************************************************************/

void cView::DrawViewElement(eViewElement ve, map <string,string> *stringTokens, map <string,int> *intTokens, map < string, vector< map< string, string > > > *loopTokens) {
    //setting correct ViewElement, depending which constructor was used
    cTemplateViewElement *viewElement;
    if (tmplItem && (ve == veMenuCurrentItemDetail || ve == veOnPause)) {
        viewElement = tmplItem;
    } else if (tmplView) {
        viewElement = tmplView->GetViewElement(ve);
    }
    if (!viewElement)
        return;

    if (viewElement->DebugTokens()) {
        DebugTokens(tmplView ? (tmplView->GetViewElementName(ve)) : "current view", stringTokens, intTokens, loopTokens);
    }
    //iterate through pixmaps of viewelement
    int pixCurrent = viewElement->GetPixOffset();
    if (pixCurrent < 0)
        return;
    viewElement->InitIterator();
    cTemplatePixmap *pix = NULL;
    while(pix = viewElement->GetNextPixmap()) {
        //reset Template 
        pix->ClearDynamicParameters();
        //create Pixmap if already fully parsed
        if (!PixmapExists(pixCurrent) && pix->Ready() && pix->DoExecute() && !pix->Scrolling()) {
            CreateViewPixmap(pixCurrent, pix);
        }
        //check if pixmap needs dynamic parameters  
        if ((!pix->Ready() || !pix->DoExecute()) && !pix->Scrolling()) {
            //parse dynamic parameters and initiate functions
            pix->ParseDynamicParameters(intTokens, true);
            if (pix->Ready() && pix->DoExecute()) {
                CreateViewPixmap(pixCurrent, pix);
            }
        } else {
            //parse dynamic parameters but not initiate functions
            pix->ParseDynamicParameters(intTokens, false);
        }
        //if pixmap still not valid, skip
        if (!pix->Ready() && !pix->Scrolling()) {
            pixCurrent++;
            continue;
        }
        //if condition for pixmap set, check if cond is true 
        if (!pix->DoExecute()) {
            pixCurrent++;
            continue;
        }
        //parse dynamic tokens of pixmap functions
        pix->ClearDynamicFunctionParameters();
        pix->ParseDynamicFunctionParameters(stringTokens, intTokens);

        if (!PixmapExists(pixCurrent) && pix->Scrolling()) {
            cSize drawportSize;
            scrolling = pix->CalculateDrawPortSize(drawportSize, loopTokens);
            if (scrolling) {
                CreateScrollingPixmap(pixCurrent, pix, drawportSize);
                pix->SetScrollingTextWidth();
                veScroll = ve;
                scrollingPix = pixCurrent;
                scrollOrientation = pix->GetNumericParameter(ptOrientation); 
                scrollMode = pix->GetNumericParameter(ptScrollMode);
                scrollDelay = pix->GetNumericParameter(ptDelay);
                scrollSpeed = pix->GetNumericParameter(ptScrollSpeed);
            } else {
                CreateViewPixmap(pixCurrent, pix);              
            }
        }
        if (pix->DoDebug()) {
            pix->Debug();
        }
        
        DrawPixmap(pixCurrent, pix, loopTokens);
        pixCurrent++; 
    }
}

void cView::ClearViewElement(eViewElement ve) {
    if (!tmplView)
        return;
    cTemplateViewElement *viewElement = tmplView->GetViewElement(ve);
    if (!viewElement)
        return;
    int pixCurrent = viewElement->GetPixOffset();
    if (pixCurrent < 0)
        return;
    cTemplatePixmap *pix = NULL;
    viewElement->InitIterator();
    while(pix = viewElement->GetNextPixmap()) {
        Fill(pixCurrent, clrTransparent);
        pixCurrent++; 
    }
}

void cView::ActivateScrolling(void) {
    if (veScroll == veUndefined)
        return;
    cTemplateViewElement *scrollViewElement = NULL;
    if (tmplView) {
        scrollViewElement = tmplView->GetViewElement(veScroll);
    }
    if (!scrollViewElement)
        return;

    ClearViewElement(veScroll);
    currentlyScrolling = true;
    
    int pixCurrent = scrollViewElement->GetPixOffset();
    if (pixCurrent < 0)
        return;
    scrollViewElement->InitIterator();
    cTemplatePixmap *pix = NULL;
    while(pix = scrollViewElement->GetNextPixmap()) {
        DrawPixmap(pixCurrent, pix);
        pixCurrent++;
    }
}

bool cView::ViewElementImplemented(eViewElement ve) {
    return tmplView->GetNumPixmapsViewElement(ve);
}

void cView::CreateViewPixmap(int num, cTemplatePixmap *pix, cRect *size) {
    cRect pixSize;
    if (size) {
        pixSize = *size;
    } else {
        pixSize = pix->GetPixmapSize();
    }
    int layer = pix->GetNumericParameter(ptLayer);
    int transparency = pix->GetNumericParameter(ptTransparency);
    SetTransparency(num, transparency);
    CreatePixmap(num, layer, pixSize);
}

void cView::CreateScrollingPixmap(int num, cTemplatePixmap *pix, cSize &drawportSize) {
    cRect pixViewPort = pix->GetPixmapSize();
    cRect drawPort;
    drawPort.SetX(0);
    drawPort.SetY(0);
    drawPort.SetWidth(drawportSize.Width());
    drawPort.SetHeight(drawportSize.Height());
    int layer = pix->GetNumericParameter(ptLayer);
    int transparency = pix->GetNumericParameter(ptTransparency);
    SetTransparency(num, transparency);
    CreatePixmap(num, layer, pixViewPort, drawPort);
}

void cView::DrawPixmap(int num, cTemplatePixmap *pix, map < string, vector< map< string, string > > > *loopTokens, bool flushPerLoop) {
    pix->InitIterator();
    cTemplateFunction *func = NULL;
    while(func = pix->GetNextFunction()) {
        eFuncType type = func->GetType();
        if (func->DoDebug()) {
            func->Debug();
        }
        if (!func->DoExecute()) {
            continue;
        }
        switch (type) {
            case ftFill:
                DoFill(num, func);
                break;
            case ftDrawText:
                DoDrawText(num, func);
                break;
            case ftDrawTextBox: {
                int floating = func->GetNumericParameter(ptFloat);
                if (floating > flNone) {
                    DoDrawFloatingTextBox(num, func);
                } else {
                    DoDrawTextBox(num, func);
                }
                break; }
            case ftDrawRectangle:
                DoDrawRectangle(num, func);
                break;
            case ftDrawEllipse:
                DoDrawEllipse(num, func);
                break;
            case ftDrawSlope:
                DoDrawSlope(num, func);
                break;
            case ftDrawImage:
                DoDrawImage(num, func);
                break;
            case ftLoop:
                if (loopTokens)
                    DrawLoop(num, func, loopTokens);
                break;
            default:
                break;
        }
        if (flushPerLoop) {
            DoFlush();
        }
    }
}

void cView::DrawLoop(int numPixmap, cTemplateFunction *func, map < string, vector< map< string, string > > > *loopTokens) {
    cTemplateLoopFunction *loopFunc = dynamic_cast<cTemplateLoopFunction*>(func);
    if (!loopFunc)
        return;
    
    int loopX0 = loopFunc->GetNumericParameter(ptX);
    if (loopX0 < 0) loopX0 = 0;
    int loopY0 = loopFunc->GetNumericParameter(ptY);
    if (loopY0 < 0) loopY0 = 0;
    int orientation = loopFunc->GetNumericParameter(ptOrientation);
    int loopWidth = loopFunc->GetNumericParameter(ptWidth);
    if (loopWidth <= 0)
        loopWidth = loopFunc->GetContainerWidth();
    int loopHeight = loopFunc->GetNumericParameter(ptHeight);
    if (loopHeight <= 0)
        loopHeight = loopFunc->GetContainerHeight();
    int columnWidth = loopFunc->GetNumericParameter(ptColumnWidth);
    int rowHeight = loopFunc->GetNumericParameter(ptRowHeight);
    int overflow = loopFunc->GetNumericParameter(ptOverflow);
    int maxItems = loopFunc->GetNumericParameter(ptNumElements);

    int x0 = loopX0;
    int y0 = loopY0;

    string loopTokenName = loopFunc->GetParameter(ptName);

    map < string, vector< map< string, string > > >::iterator hit = loopTokens->find(loopTokenName);
    if (hit == loopTokens->end())
        return;
    vector< map<string,string> > loopToken = hit->second;
    int lineNumber=0;
    for (vector< map<string,string> >::iterator line = loopToken.begin(); line != loopToken.end(); line++) {
        //check overflow behaviour
        if (overflow == otCut) {
            if (orientation == orHorizontal) {
                if (lineNumber * columnWidth > loopWidth) {
                    return;
                }
            } else if (orientation == orVertical) {
                if (lineNumber * rowHeight > loopHeight) {
                    return;
                }                
            }
        } else if (overflow == otWrap && orientation == orHorizontal) {
            if (x0 + columnWidth > loopWidth) {
                x0 = loopX0;
                if (rowHeight > 0) {
                    y0 += rowHeight;
                } else {
                    y0 += loopFunc->GetLoopElementsHeight();
                } 
            }
        }
        map<string,string> tokens = *line;
        loopFunc->ClearDynamicParameters();
        loopFunc->ParseDynamicParameters(&tokens);
        loopFunc->InitIterator();
        cTemplateFunction *func = NULL;
        while(func = loopFunc->GetNextFunction()) {
            //do debug?
            if (func->DoDebug())
                func->Debug();
            //check if set condition is true
            if (!func->DoExecute()) {
                continue;
            }
            //execute
            eFuncType type = func->GetType();
            switch (type) {
                case ftDrawText:
                    DoDrawText(numPixmap, func, x0, y0);
                    break;
                case ftDrawTextBox:
                    DoDrawTextBox(numPixmap, func, x0, y0);
                    break;
                case ftDrawRectangle:
                    DoDrawRectangle(numPixmap, func, x0, y0);
                    break;
                case ftDrawEllipse:
                    DoDrawEllipse(numPixmap, func, x0, y0);
                    break;
                case ftDrawSlope:
                    DoDrawSlope(numPixmap, func, x0, y0);
                    break;
                case ftDrawImage:
                    DoDrawImage(numPixmap, func, x0, y0);
                    break;
                default:
                    break;
            }
        }
        //calculate position of next loop element 
        if (orientation == orHorizontal) {
            if (columnWidth > 0) {
                x0 += columnWidth;
            } else {
                x0 += loopFunc->GetLoopElementsWidth();
            }
        } else if (orientation == orVertical) {
            if (rowHeight > 0) {
                y0 += rowHeight;
            } else {
                y0 += loopFunc->GetLoopElementsHeight();
            }
        }
        lineNumber++;
        //DoFlush();
    }
}

void cView::DebugTokens(string viewElement, map<string,string> *stringTokens, map<string,int> *intTokens, map < string, vector< map< string, string > > > *loopTokens) {
    esyslog("skindesigner: ------------------------------ Tokens for %s:", viewElement.c_str());
    if (stringTokens) {
        for (map<string,string>::iterator st = stringTokens->begin(); st != stringTokens->end(); st++) {
            esyslog("skindesigner: string var \"%s\" = \"%s\"", (st->first).c_str(), (st->second).c_str());
        }
    }
    if (intTokens) {
        for (map<string,int>::iterator it = intTokens->begin(); it != intTokens->end(); it++) {
            esyslog("skindesigner: int var \"%s\" = %d", (it->first).c_str(), it->second);
        }
    }
    if (loopTokens) {
        for(map < string, vector< map< string, string > > >::iterator it1 = loopTokens->begin(); it1 != loopTokens->end(); it1++) {
            int line = 0;
            string tokenName = it1->first; 
            vector< map<string,string> > tokens = it1->second;
            esyslog("skindesigner: loop token %s", tokenName.c_str());
            for (vector< map<string,string> >::iterator it2 = tokens.begin(); it2 != tokens.end(); it2++) {
                esyslog("skindesigner: loop tokens line %d:", line++);
                map<string,string> element = *it2;
                for (map<string,string>::iterator el = element.begin(); el != element.end(); el++) {
                    esyslog("skindesigner: name: %s, value: %s", (el->first).c_str(), (el->second).c_str());
                }
            }
        }
    }
}

/*****************************************************************
* Private Functions
*****************************************************************/

void cView::DoFill(int num, cTemplateFunction *func) {
    tColor col = func->GetColorParameter(ptColor);
    Fill(num, col);
}

void cView::DoDrawText(int num, cTemplateFunction *func, int x0, int y0) {
    int x = func->GetNumericParameter(ptX);
    if (x < 0) x = 0;
    x += x0;
    int y = func->GetNumericParameter(ptY);
    if (y < 0) y = 0;
    y += y0;
    cPoint pos(x,y);
    string fontName = func->GetFontName();
    int fontSize = func->GetNumericParameter(ptFontSize);
    tColor clr = func->GetColorParameter(ptColor);
    tColor clrBack = clrTransparent;
    string text = "";
    if (!currentlyScrolling) {
        text = func->GetText(true);
    } else {
        text = func->GetText(false);
    }
    DrawText(num, pos, text.c_str(), clr, clrBack, fontName, fontSize);
}

void cView::DoDrawTextBox(int num, cTemplateFunction *func, int x0, int y0) {
    string text = func->GetText(false);
    if (text.size() < 3)
        return;
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
        x += x0;
    if (y < 0) y = 0;
        y += y0;        
    int width = func->GetNumericParameter(ptWidth);
    int height = func->GetNumericParameter(ptHeight);
    string fontName = func->GetFontName();
    int fontSize = func->GetNumericParameter(ptFontSize);
    int align = func->GetNumericParameter(ptAlign);
    int maxLines = func->GetNumericParameter(ptMaxLines);
    tColor clr = func->GetColorParameter(ptColor);
    tColor clrBack = clrTransparent;
    const cFont *font = fontManager->Font(fontName, fontSize);
    if (!font)
        return;
    cTextWrapper wrapper;
    wrapper.Set(text.c_str(), font, width);
    int fontHeight = fontManager->Height(fontName, fontSize);
    int lines = wrapper.Lines();
    int yLine = y;
    for (int line=0; line < lines; line++) {
        int xLine = x;
        if (align == alCenter) {
            int textWidth = font->Width(wrapper.GetLine(line));
            xLine += (width - textWidth)/2;
        } else if (align == alRight) {
            int textWidth = font->Width(wrapper.GetLine(line));
            xLine += (width - textWidth);           
        }
        cPoint pos(xLine, yLine);
        if (maxLines > 0 && line == maxLines-1) {
            string lastLine = wrapper.GetLine(line);
        if (lines > maxLines) {
                lastLine += "...";
            }
        DrawText(num, pos, lastLine.c_str(), clr, clrBack, fontName, fontSize);
            break;
        } else if (height > 0 && yLine - y + 2*fontHeight > height) {
            DrawText(num, pos, "...", clr, clrBack, fontName, fontSize);
            break;
        }
        DrawText(num, pos, wrapper.GetLine(line), clr, clrBack, fontName, fontSize);
        yLine += fontHeight;
    }
}

void cView::DoDrawFloatingTextBox(int num, cTemplateFunction *func) {
    string text = func->GetText(false);
    if (text.size() < 3)
        return;
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    int width = func->GetNumericParameter(ptWidth);
    int height = func->GetNumericParameter(ptHeight);
    string fontName = func->GetFontName();
    int fontSize = func->GetNumericParameter(ptFontSize);
    tColor clr = func->GetColorParameter(ptColor);
    tColor clrBack = clrTransparent;
    const cFont *font = fontManager->Font(fontName, fontSize);
    if (!font)
        return;
    int floatType = func->GetNumericParameter(ptFloat);
    int floatWidth = func->GetNumericParameter(ptFloatWidth);
    int floatHeight = func->GetNumericParameter(ptFloatHeight);

    cTextWrapper wTextTall;
    cTextWrapper wTextFull;
    
    int fontHeight = fontManager->Height(fontName, fontSize);
    int linesNarrow = floatHeight / fontHeight;
    int widthNarrow = width - floatWidth;
    int linesDrawn = 0;
    int curY = 0;
    bool drawNarrow = true;

    splitstring s(text.c_str());
    std::vector<std::string> flds = s.split('\n', 1);

    if (flds.size() < 1)
        return;

    std::stringstream sstrTextTall;
    std::stringstream sstrTextFull;

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
    //VDRs textwrapper swallows linebreaks at the end, so we have to fix that manually
    string textTall = sstrTextTall.str();
    size_t posLastCarriageReturn = textTall.find_last_not_of("\n");

    int numLinesToAddAtTall = 0;
    if (posLastCarriageReturn != string::npos && (posLastCarriageReturn < textTall.size() - 1)) {
        numLinesToAddAtTall = textTall.size() - posLastCarriageReturn - 2;
    }

    wTextTall.Set(textTall.c_str(), font, widthNarrow);
    wTextFull.Set(sstrTextFull.str().c_str(), font, width);

    int textLinesTall = wTextTall.Lines();
    int textLinesFull = wTextFull.Lines();

    int textXTall = x;
    if (floatType == flTopLeft)
        textXTall = x + floatWidth;

    int yLine = y;
    for (int line=0; line < textLinesTall; line++) {
        cPoint pos(textXTall, yLine);
        DrawText(num, pos, wTextTall.GetLine(line), clr, clrBack, fontName, fontSize);
        yLine += fontHeight;
    }

    if (numLinesToAddAtTall) {
        yLine += numLinesToAddAtTall * fontHeight;
    }

    for (int line=0; line < textLinesFull; line++) {
        cPoint pos(x, yLine);
        if (height > 0 && yLine - y + 2*fontHeight > height) {
            DrawText(num, pos, "...", clr, clrBack, fontName, fontSize);
            break;
        }
        DrawText(num, pos, wTextFull.GetLine(line), clr, clrBack, fontName, fontSize);
        yLine += fontHeight;
    }
}

void cView::DoDrawRectangle(int num, cTemplateFunction *func, int x0, int y0) {
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
    x += x0;
    if (y < 0) y = 0;
    y += y0;
    int w = func->GetNumericParameter(ptWidth);
    int h = func->GetNumericParameter(ptHeight);
    cRect size(x, y, w, h);
    tColor clr = func->GetColorParameter(ptColor);
    DrawRectangle(num, size, clr);
}

void cView::DoDrawEllipse(int num, cTemplateFunction *func, int x0, int y0) {
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
    x += x0;
    if (y < 0) y = 0;
    y += y0;
    int w = func->GetNumericParameter(ptWidth);
    int h = func->GetNumericParameter(ptHeight);
    cRect size(x, y, w, h);
    tColor clr = func->GetColorParameter(ptColor);
    int quadrant = func->GetNumericParameter(ptQuadrant);
    if (quadrant < -4 || quadrant > 8) {
        esyslog("skindesigner: wrong quadrant %d for drawellipse, allowed values are from -4 to 8", quadrant);
        quadrant = 0;
    }
    DrawEllipse(num, size, clr, quadrant);
}

void cView::DoDrawSlope(int num, cTemplateFunction *func, int x0, int y0) {
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
    x += x0;
    if (y < 0) y = 0;
    y += y0;
    int w = func->GetNumericParameter(ptWidth);
    int h = func->GetNumericParameter(ptHeight);
    cRect size(x, y, w, h);
    tColor clr = func->GetColorParameter(ptColor);
    int type = func->GetNumericParameter(ptType);
    if (type < 0 || type > 7) {
        esyslog("skindesigner: wrong type %d for drawslope, allowed values are from 0 to 7", type);
        type = 0;
    }
    DrawSlope(num, size, clr, type);
}

void cView::DoDrawImage(int num, cTemplateFunction *func, int x0, int y0) {
    int x = func->GetNumericParameter(ptX);
    int y = func->GetNumericParameter(ptY);
    if (x < 0) x = 0;
    x += x0;
    if (y < 0) y = 0;
    y += y0;
    cPoint pos(x,y);
    int width = func->GetNumericParameter(ptWidth);
    int height = func->GetNumericParameter(ptHeight);
    string path = func->GetImagePath();
    eImageType type = (eImageType)func->GetNumericParameter(ptImageType);
    switch (type) {
        case itChannelLogo: {
            cImage *logo = imgCache->GetLogo(path, width, height);
            if (logo) {
                DrawImage(num, pos, *logo);
            }
            break; }
        case itSepLogo: {
            cImage *sepLogo = imgCache->GetSeparatorLogo(path, width, height);
            if (sepLogo) {
                DrawImage(num, pos, *sepLogo);
            }
            break; }
        case itSkinPart: {
            cImage *skinpart = imgCache->GetSkinpart(path, width, height);
            if (skinpart) {
                DrawImage(num, pos, *skinpart);
            }
            break; }
        case itIcon: {
            cImage *icon = imgCache->GetIcon(type, path, width, height);
            if (icon) {
                DrawImage(num, pos, *icon);
            }
            break; }
        case itMenuIcon: {
            cImage *icon = imgCache->GetIcon(type, path, width, height);
            if (icon) {
                DrawImage(num, pos, *icon);
            }
            break; }
        case itImage: {
            cImageLoader imgLoader;
            if (imgLoader.LoadImage(path.c_str())) {
                cImage *image = imgLoader.CreateImage(width, height);
                DrawImage(num, pos, *image);
                delete(image);
            }
            break; }
        default:
            break;
    }
}

/***********************************************************************
* cViewListItem
************************************************************************/

cViewListItem::cViewListItem(cTemplateViewElement *tmplItem) : cView(tmplItem) {
    pos = -1;
    numTotal = 0;
    align = alLeft;
    listOrientation = orVertical;
}

cViewListItem::~cViewListItem() {

}

cRect cViewListItem::DrawListItem(map <string,string> *stringTokens, map <string,int> *intTokens) {
    cRect posItem;
    if (!tmplItem)
        return posItem;

    if (tmplItem->DebugTokens()) {
        DebugTokens("ListItem", stringTokens, intTokens);
    }

    tmplItem->InitIterator();
    cTemplatePixmap *pix = NULL;
    int pixCurrent = 0;

    while(pix = tmplItem->GetNextPixmap()) {
        SetListElementPosition(pix);
        if (pixCurrent == 0) {
            posItem = pix->GetPixmapSize();
        }
        if (!PixmapExists(pixCurrent)) {
            pix->ParseDynamicParameters(intTokens, true);
        } else {
            pix->ParseDynamicParameters(intTokens, false);            
        }
        if (!PixmapExists(pixCurrent) && pix->Ready() && pix->DoExecute() && !pix->Scrolling()) {
            CreateViewPixmap(pixCurrent, pix);
        }
        //if pixmap still not valid, skip
        if (!pix->Ready()  && !pix->Scrolling()) {
            pixCurrent++;
            continue;
        }
        //if condition for pixmap set, check if cond is true 
        if (!pix->DoExecute()) {
            pixCurrent++;
            continue;
        }
    
        pix->ClearDynamicFunctionParameters();
        pix->ParseDynamicFunctionParameters(stringTokens, intTokens);

        if (!PixmapExists(pixCurrent) && pix->Scrolling()) {
            cSize drawportSize;
            scrolling = pix->CalculateDrawPortSize(drawportSize);
            pix->SetScrollingTextWidth();
            if (scrolling) {
                CreateScrollingPixmap(pixCurrent, pix, drawportSize);
                scrollingPix = pixCurrent;
                scrollOrientation = pix->GetNumericParameter(ptOrientation); 
                scrollMode = pix->GetNumericParameter(ptScrollMode);
                scrollDelay = pix->GetNumericParameter(ptDelay);
                scrollSpeed = pix->GetNumericParameter(ptScrollSpeed);
            } else {
                CreateViewPixmap(pixCurrent, pix);
            }
        }
        if (pix->DoDebug()) {
            pix->Debug();
        }
        DrawPixmap(pixCurrent, pix);
        pixCurrent++;
    }
    return posItem;
}

void cViewListItem::ClearListItem(void) {
    int pixMax = NumPixmaps();
    for (int pixCurrent = 0; pixCurrent < pixMax; pixCurrent++) {
        Fill(pixCurrent, clrTransparent);
    }
}

void cViewListItem::SetListElementPosition(cTemplatePixmap *pix) {
    int itemWidth = pix->GetNumericParameter(ptWidth);
    int itemHeight = pix->GetNumericParameter(ptHeight);
    int x = 0;
    int y = 0;
    if (listOrientation == orHorizontal) {
        x = container.X();
        int totalWidth = numTotal * itemWidth;
        if (align == alCenter) {
            y += (container.Width() - totalWidth) / 2;
        } else if (align == alBottom) {
            y += (container.Width() - totalWidth);
        }
        x += pos * itemWidth;
        y = pix->GetNumericParameter(ptY);
    } else if (listOrientation == orVertical) {
        y = container.Y();
        int totalHeight = numTotal * itemHeight;
        if (align == alCenter) {
            y += (container.Height() - totalHeight) / 2;
        } else if (align == alBottom) {
            y += (container.Height() - totalHeight);
        }
        y += pos * itemHeight;
        x = pix->GetNumericParameter(ptX);
    }
    pix->SetX(x);
    pix->SetY(y);
}

