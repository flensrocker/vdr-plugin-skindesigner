#include "globals.h"
#include "xmlparser.h"
#include <locale.h>

cGlobals::cGlobals(void) {
    fonts.insert(pair<string, string>("vdrOsd", Setup.FontOsd));
    fonts.insert(pair<string, string>("vdrFix", Setup.FontFix));
    fonts.insert(pair<string, string>("vdrSml", Setup.FontSml));
    language = Setup.OSDLanguage;
    dsyslog("skindesigner: using language %s", language.c_str());
}

bool cGlobals::ReadFromXML(void) {
    std::string xmlFile = "globals.xml";
    cXmlParser parser;
    if (!parser.ReadGlobals(this, xmlFile))
        return false;
    if (!parser.ParseGlobals())
        return false;
    return true;
}

bool cGlobals::Translate(string text, string &translation) {
    string transStart = "{tr(";
    string transEnd = ")}";
    size_t foundStart = text.find(transStart);
    size_t foundEnd = text.find(transEnd);
    bool translated = false;

    while (foundStart != string::npos && foundEnd != string::npos) {
        string token = text.substr(foundStart + 1, foundEnd - foundStart);
        string transToken = DoTranslate(token);
        if (transToken.size() > 0)
            translated = true;
        else
            return false;
        text.replace(foundStart, foundEnd - foundStart + 2, transToken);
        foundStart = text.find(transStart);
        foundEnd = text.find(transEnd);
    }
    if (translated)
        translation = text;
    return translated;
}

string cGlobals::DoTranslate(string token) {
    string translation = "";
    map <string, map< string, string > >::iterator hit = translations.find(token);
    if (hit == translations.end()) {
        esyslog("skindesigner: invalid translation token %s", token.c_str());
        return translation;
    }
    map< string, string > translats = hit->second;
    map< string, string >::iterator trans = translats.find(language);
    if (trans != translats.end()) {
        translation = trans->second;
    } else {
        map< string, string >::iterator transDefault = translats.find("en_EN");
        if (transDefault != translats.end()) {
            translation = transDefault->second;
        }
    }
    return translation;
}

void cGlobals::Debug(void) {
    dsyslog("skindesigner: GLOBAL VARIABLES");
    for (map <string, tColor>::iterator col = colors.begin(); col != colors.end(); col++) {
        dsyslog("skindesigner: Color \"%s\": %x", (col->first).c_str(), col->second);
    }
    for (map <string, int>::iterator myInt = intVars.begin(); myInt != intVars.end(); myInt++) {
        dsyslog("skindesigner: Integer Variable \"%s\": %d", (myInt->first).c_str(), myInt->second);
    }
    for (map <string, double>::iterator myDouble = doubleVars.begin(); myDouble != doubleVars.end(); myDouble++) {
        dsyslog("skindesigner: Double Variable \"%s\": %f", (myDouble->first).c_str(), myDouble->second);
    }
    for (map <string, string>::iterator myStr = stringVars.begin(); myStr != stringVars.end(); myStr++) {
        dsyslog("skindesigner: String Variable \"%s\": \"%s\"", (myStr->first).c_str(), (myStr->second).c_str());
    }
    for (map <string, string>::iterator font = fonts.begin(); font != fonts.end(); font++) {
        dsyslog("skindesigner: Font \"%s\": \"%s\"", (font->first).c_str(), (font->second).c_str());
    }
    for (map <string, map< string, string > >::iterator trans = translations.begin(); trans != translations.end(); trans++) {
        dsyslog("skindesigner: Translation Token %s", (trans->first).c_str());
        map< string, string > tokenTrans = trans->second;
        for (map< string, string >::iterator transTok = tokenTrans.begin(); transTok != tokenTrans.end(); transTok++) {
            dsyslog("skindesigner: language %s, translation %s", (transTok->first).c_str(), (transTok->second).c_str());
        }
    }
}
