#include "../config.h"
#include "parameter.h"

using namespace std;

// --- cNumericParameter -------------------------------------------------------------

cNumericParameter::cNumericParameter(string value) { 
    this->value = value;
    globals = NULL;
    isValid = false;
    width = 0;
    height = 0;
    columnWidth = -1;
    rowHeight = -1;
    hor = true;
    defaultValue = 0;
}

cNumericParameter::~cNumericParameter(void) {
}

void cNumericParameter::SetAreaSize(int w, int h) {
    width = w;
    height = h;
}

int cNumericParameter::Parse(string &parsedValue) {
    int retVal = defaultValue;

    if (IsNumber(value)) {
        isValid = true;
        retVal = atoi(value.c_str());
        return retVal;
    }

    //checking for percent value
    bool isPercentValue = CheckPercentValue(retVal);
    if (isPercentValue) {
        isValid = true;
        return retVal;
    }

    //checking for expression
    bool isValidExpression = CheckExpression(retVal, parsedValue);
    if (isValidExpression) {
        isValid = true;
        return retVal;
    }

    return retVal;
}

bool cNumericParameter::IsNumber(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool cNumericParameter::CheckPercentValue(int &val) {
    bool ok = false;
    size_t posPercent = value.find('%');
    if (posPercent != string::npos) {
        string strPerc = value.substr(0, posPercent);
        if (!IsNumber(strPerc)) {
            return ok;
        }
        int perc = atoi(strPerc.c_str());
        if (hor) {
            val = width * perc / 100;
        } else {
            val = height * perc / 100;          
        }
        ok = true;
    }
    return ok;
}

bool cNumericParameter::CheckExpression(int &val, string &parsedVal) {
    bool ok = false;
    string parsedValue = value;
    //remove white spaces
    parsedValue.erase( std::remove_if( parsedValue.begin(), parsedValue.end(), ::isspace ), parsedValue.end() );

    //check and replace {areawidth} and {areaheight} tokens
    string tokenWidth = "{areawidth}";
    string tokenHeight = "{areaheight}";

    stringstream sw;
    sw << width;
    string strWidth = sw.str();
    stringstream sh;
    sh << height;
    string strHeight = sh.str();

    bool foundToken = true;
    while(foundToken) {
        size_t foundTokenWidth = parsedValue.find(tokenWidth);
        if (foundTokenWidth != string::npos) {
            parsedValue = parsedValue.replace(foundTokenWidth, tokenWidth.size(), strWidth);
        } else {
            foundToken = false;
        }
    }

    foundToken = true;
    while(foundToken) {
        size_t foundTokenHeight = parsedValue.find(tokenHeight);
        if (foundTokenHeight != string::npos) {
            parsedValue = parsedValue.replace(foundTokenHeight, tokenHeight.size(), strHeight);
        } else {
            foundToken = false;
        }
    }

    //check and replace {columnwidth} and {rowheight} tokens for loop functions
    if (columnWidth > 0 || rowHeight > 0) {
        tokenWidth = "{columnwidth}";
        tokenHeight = "{rowheight}";
        stringstream cw;
        cw << columnWidth;
        strWidth = cw.str();
        stringstream rh;
        rh << rowHeight;
        strHeight = rh.str();

        foundToken = true;
        while(foundToken) {
            size_t foundTokenWidth = parsedValue.find(tokenWidth);
            if (foundTokenWidth != string::npos) {
                parsedValue = parsedValue.replace(foundTokenWidth, tokenWidth.size(), strWidth);
            } else {
                foundToken = false;
            }
        }

        foundToken = true;
        while(foundToken) {
            size_t foundTokenHeight = parsedValue.find(tokenHeight);
            if (foundTokenHeight != string::npos) {
                parsedValue = parsedValue.replace(foundTokenHeight, tokenHeight.size(), strHeight);
            } else {
                foundToken = false;
            }
        }
    }

    if (globals) {
        for (map<string, int>::iterator globInt = globals->intVars.begin(); globInt != globals->intVars.end(); globInt++) {
            stringstream sToken;
            sToken << "{" << globInt->first << "}";
            string token = sToken.str();
            size_t foundToken = parsedValue.find(token);
            if (foundToken != string::npos) {
                stringstream st;
                st << globInt->second;
                parsedValue = parsedValue.replace(foundToken, token.size(), st.str());
            }
        }
        for (map<string, double>::iterator globDouble = globals->doubleVars.begin(); globDouble != globals->doubleVars.end(); globDouble++) {
            stringstream sToken;
            sToken << "{" << globDouble->first << "}";
            string token = sToken.str();
            size_t foundToken = parsedValue.find(token);
            if (foundToken != string::npos) {
                stringstream st;
                st << globDouble->second;
                string doubleVal = st.str();
                if (config.replaceDecPoint) {
                    if (doubleVal.find_first_of('.') != string::npos) {
                        std::replace( doubleVal.begin(), doubleVal.end(), '.', config.decPoint);
                    }
                }
                parsedValue = parsedValue.replace(foundToken, token.size(), doubleVal);
            }
        }
    }

    if (IsNumber(parsedValue)) {
        ok = true;
        val = atoi(parsedValue.c_str());
        return ok;
    }

    if (!ValidNumericExpression(parsedValue)) {
        parsedVal = parsedValue;
        return ok;
    }
    ok = true;
    char * expression = new char[parsedValue.size() + 1];
    std::copy(parsedValue.begin(), parsedValue.end(), expression);
    expression[parsedValue.size()] = '\0';
    int expRes = EvaluateTheExpression(expression);
    val = expRes;
    delete[] expression;
    return ok;
}

bool cNumericParameter::ValidNumericExpression(string &parsedValue) {
    string::const_iterator it = parsedValue.begin();
    while (it != parsedValue.end() && (isdigit(*it) || *it == '.' || *it == ',' || *it == '+' || *it == '-' || *it == '*' || *it == '/')) ++it;
    return !parsedValue.empty() && it == parsedValue.end();
}

int cNumericParameter::EvaluateTheExpression(char* expr) {
    return round(ParseSummands(expr));
}

double cNumericParameter::ParseAtom(char*& expr) {
    // Read the number from string
    char* end_ptr;
    double res = strtod(expr, &end_ptr);
    // Advance the pointer and return the result
    expr = end_ptr;
    return res;
}

// Parse multiplication and division
double cNumericParameter::ParseFactors(char*& expr) {
    double num1 = ParseAtom(expr);
    for(;;) {
        // Save the operation
        char op = *expr;
        if(op != '/' && op != '*')
            return num1;
        expr++;
        double num2 = ParseAtom(expr);
        // Perform the saved operation
        if(op == '/') {
            if (num2 != 0) {
                num1 /= num2;
            }
        } else
            num1 *= num2;
    }
}

// Parse addition and subtraction
double cNumericParameter::ParseSummands(char*& expr) {
    double num1 = ParseFactors(expr);
    for(;;) {
        char op = *expr;
        if(op != '-' && op != '+')
            return num1;
        expr++;
        double num2 = ParseFactors(expr);
        if(op == '-')
            num1 -= num2;
        else
            num1 += num2;
    }
}

// --- cConditionalParameter -------------------------------------------------------------

cConditionalParameter::cConditionalParameter(cGlobals *globals, string value) { 
    this->globals = globals;
    isTrue = false;
    this->value = value;
    type = cpNone;
}

cConditionalParameter::~cConditionalParameter(void) {
}

void cConditionalParameter::Tokenize(void) {
    size_t posAnd = value.find("++");
    if (posAnd != string::npos) {
        type = cpAnd;
        TokenizeValue("++");
    } else {
        size_t posOr = value.find("||");
        if (posOr != string::npos) {
            type = cpOr;
            TokenizeValue("||");
        }
    }
    if (type == cpNone) {
        InsertCondition(value);
    }
}

bool cConditionalParameter::Evaluate(map < string, int > *intTokens, map < string, string > *stringTokens) {
    isTrue = false;
    bool first = true;
    for (vector<sCondition>::iterator cond = conditions.begin(); cond != conditions.end(); cond++) {
        bool tokenTrue = false;
        
        if (cond->type == ctStringSet) {
            if (stringTokens) {
                map < string, string >::iterator hit = stringTokens->find(cond->tokenName);
                if (hit != stringTokens->end()) {
                    string value = hit->second;
                    if (value.size() > 0)
                        tokenTrue = true;
                }
            }
        } else {
            int tokenValue = EvaluateParameter(cond->tokenName, intTokens, stringTokens);
            if (cond->type == ctBool) {
                tokenTrue = tokenValue;
            } else if (cond->type == ctGreater) {
                tokenTrue = (tokenValue > cond->compareValue) ? true : false;
            } else if (cond->type == ctLower) {
                tokenTrue = (tokenValue < cond->compareValue) ? true : false;
            } else if (cond->type == ctEquals) {
                tokenTrue = (tokenValue == cond->compareValue) ? true : false;
            }
        }

        if (cond->isNegated)
            tokenTrue = !tokenTrue;
        if (type == cpAnd) {
            if (first)
                isTrue = tokenTrue;
            else
                isTrue = isTrue && tokenTrue;
        } else if (type == cpOr) {
            isTrue = isTrue || tokenTrue;           
        } else {
            isTrue = tokenTrue;
        }
        first = false;
    }
}

int cConditionalParameter::EvaluateParameter(string token, map < string, int > *intTokens, map < string, string > *stringTokens) {
    //first check globals
    map < string, int >::iterator hitGlobals = globals->intVars.find(token);
    if (hitGlobals != globals->intVars.end()) {
        return hitGlobals->second;
    } else {
        //then check tokens
        if (intTokens) {
            map < string, int >::iterator hit = intTokens->find(token);
            if (hit != intTokens->end()) {
                return hit->second;
            }
        }
        if (stringTokens) {
            map < string, string >::iterator hit = stringTokens->find(token);
            if (hit != stringTokens->end()) {
                string value = hit->second;
                return atoi(value.c_str());
            }
        }
    }
    return 0;
}

void cConditionalParameter::TokenizeValue(string sep) {
    string buffer = value;
    bool sepFound = true;
    while (sepFound) {
        size_t posSep = buffer.find(sep);
        if (posSep == string::npos) {
            InsertCondition(buffer);
            sepFound = false;
        }
        string token = buffer.substr(0, posSep);
        buffer = buffer.replace(0, posSep + sep.size(), "");
        InsertCondition(token);
    }
}

void cConditionalParameter::InsertCondition(string cond) {
    cond.erase( std::remove_if( cond.begin(), cond.end(), ::isspace ), cond.end() );

    if (cond.size() < 1)
        return;

    size_t tokenStart = cond.find('{');
    size_t tokenEnd = cond.find('}');

    if (tokenStart == string::npos || tokenEnd == string::npos || tokenStart > tokenEnd)
        return;

    string tokenName = cond.substr(tokenStart + 1, tokenEnd - tokenStart - 1);
    string rest = cond.replace(tokenStart, tokenEnd - tokenStart + 1, "");

    sCondition sCond;
    sCond.tokenName = tokenName;
    sCond.type = ctBool;
    sCond.compareValue = 0;
    sCond.isNegated = false;
    if (!rest.compare("not")) {
        sCond.isNegated = true;
    } else if (!rest.compare("isset")) {
        sCond.type = ctStringSet;
    } else if (startswith(rest.c_str(), "gt(")) {
        string compVal = rest.substr(4, rest.size() - 5);
        sCond.compareValue = atoi(compVal.c_str());
        sCond.type = ctGreater;
    }  else if (startswith(rest.c_str(), "lt(")) {
        string compVal = rest.substr(4, rest.size() - 5);
        sCond.compareValue = atoi(compVal.c_str());
        sCond.type = ctLower;
    }  else if (startswith(rest.c_str(), "eq(")) {
        string compVal = rest.substr(4, rest.size() - 5);
        sCond.compareValue = atoi(compVal.c_str());
        sCond.type = ctEquals;
    }

    conditions.push_back(sCond);
}

void cConditionalParameter::Debug(void) {
    dsyslog("skindesigner: Condition %s, Type: %s, cond is %s", value.c_str(), (type == cpAnd)?"and combination":((type == cpOr)?"or combination":"single param") , isTrue?"true":"false");
    for (vector<sCondition>::iterator it = conditions.begin(); it != conditions.end(); it++) {
        dsyslog("skindesigner: cond token %s, type: %d, compareValue %d, negated: %d", it->tokenName.c_str(), it->type, it->compareValue, it->isNegated);
    }
}