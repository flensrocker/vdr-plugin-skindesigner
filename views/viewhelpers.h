#ifndef __VIEWHELPERS_H
#define __VIEWHELPERS_H

class cViewHelpers {
private:
    bool devicesInit;
    int* lastSignalStrength;
    int* lastSignalQuality;
    bool* recDevices;
protected:
    void InitDevices(void);
    bool SetDevices(bool initial, map<string,int> *intTokens, vector<map<string,string> > *devices);
    bool CheckNewMails(void);
    void SetScraperTokens(const cEvent *event, const cRecording *recording, map < string, string > &stringTokens, map < string, int > &intTokens, map < string, vector< map< string, string > > > &loopTokens);
public:
    cViewHelpers(void);
    virtual ~cViewHelpers(void);
};

#endif //__VIEWHELPERS_H