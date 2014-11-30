#include "templateviewelement.h"
#include "../config.h"

cTemplateViewElement::cTemplateViewElement(void) {
    debugTokens = false;
    parameters = NULL;
    containerX = 0;
    containerY = 0;
    containerWidth = 0;
    containerHeight = 0;
    pixOffset = -1;
}

cTemplateViewElement::~cTemplateViewElement(void) {
    if (parameters)
        delete parameters;
    for (vector<cTemplatePixmap*>::iterator it = viewPixmaps.begin(); it != viewPixmaps.end(); it++) {
        delete (*it);
    }
}

void cTemplateViewElement::SetContainer(int x, int y, int width, int height) { 
    containerX = x;
    containerY = y;
    containerWidth = width; 
    containerHeight = height; 
}

void cTemplateViewElement::SetGlobals(cGlobals *globals) { 
    this->globals = globals;
    for (vector<cTemplatePixmap*>::iterator pix = viewPixmaps.begin(); pix != viewPixmaps.end(); pix++) {
        (*pix)->SetGlobals(globals);
    }
}

void cTemplateViewElement::SetParameters(vector<pair<string, string> > &params) {
    parameters = new cTemplateFunction(ftViewElement);
    parameters->SetGlobals(globals);
    parameters->SetParameters(params);
}

bool cTemplateViewElement::CalculateParameters(void) {
    if (!parameters)
        return true;
    bool paramsValid = true;
    parameters->SetContainer(containerX, containerY, containerWidth, containerHeight);
    parameters->SetGlobals(globals);
    paramsValid = parameters->CalculateParameters();

    return paramsValid;
}

bool cTemplateViewElement::CalculatePixmapParameters(void) {
    bool paramsValid = true;
    for (vector<cTemplatePixmap*>::iterator pix = viewPixmaps.begin(); pix != viewPixmaps.end(); pix++) {
        (*pix)->SetContainer(containerX, containerY, containerWidth, containerHeight);
        (*pix)->SetGlobals(globals);
        paramsValid = paramsValid && (*pix)->CalculateParameters();
    }
    return paramsValid;
}

bool cTemplateViewElement::CalculatePixmapParametersList(int orientation, int numElements) {
    bool paramsValid = true;
    for (vector<cTemplatePixmap*>::iterator pix = viewPixmaps.begin(); pix != viewPixmaps.end(); pix++) {
        (*pix)->SetContainer(containerX, containerY, containerWidth, containerHeight);
        (*pix)->SetGlobals(globals);
        if (orientation == orHorizontal) {
            if (numElements > 0) {
                int width = containerWidth / numElements;
                (*pix)->SetWidth(width);
            }
        } else if (orientation == orVertical) {
            if (numElements > 0) {
                int height = containerHeight / numElements;
                (*pix)->SetHeight(height);
            }
        } 
        paramsValid = paramsValid && (*pix)->CalculateParameters();
    }
    return paramsValid;
}

int cTemplateViewElement::GetNumericParameter(eParamType type) {
    if (!parameters)
        return -1;
    return parameters->GetNumericParameter(type);
}

void cTemplateViewElement::InitIterator(void) {
    pixIterator = viewPixmaps.begin();
}

cTemplatePixmap *cTemplateViewElement::GetNextPixmap(void) {
    if (pixIterator == viewPixmaps.end())
        return NULL;
    cTemplatePixmap *pix = *pixIterator;
    pixIterator++;
    return pix; 
}

cTemplateFunction *cTemplateViewElement::GetFunction(string name) {
    InitIterator();
    cTemplatePixmap *pix = NULL;
    while (pix = GetNextPixmap()) {
        pix->InitIterator();
        cTemplateFunction *func = NULL;
        while(func = pix->GetNextFunction()) {
            if (func->GetType() == ftDrawText) {
                string funcName = func->GetParameter(ptName);
                if (!funcName.compare(name))
                    return func;
            } else {
                continue;
            }
        }
    }
    return NULL;
}

bool cTemplateViewElement::DebugTokens(void) {
    if (!parameters)
        return false;
    return parameters->DoDebug();
}


void cTemplateViewElement::Debug(void) {
    esyslog("skindesigner: viewelement container size x: %d, y: %d, width: %d, height %d", containerX, containerY, containerWidth, containerHeight);
    if (parameters)
        parameters->Debug();
    for (vector<cTemplatePixmap*>::iterator it = viewPixmaps.begin(); it != viewPixmaps.end(); it++) {
        (*it)->Debug();
    }
}