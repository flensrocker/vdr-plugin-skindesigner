#ifndef __RECFOLDERINFO_H
#define __RECFOLDERINFO_H

#include <vdr/recording.h>


class cRecordingsFolderInfo {
public:
  class cFolderInfoIntern;

private:
  cRecordings        &_recordings;
  int                 _recState;
  cFolderInfoIntern  *_root;
  mutable cMutex      _rootLock;

  void Rebuild(void);

public:
  class cFolderInfo {
  public:
    cString Name;
    cString FullName;
    time_t  Latest;
    int     Count;
    cString LatestFileName;

    cFolderInfo(const char *Name, const char *FullName, time_t Latest, int Count, const char *LatestFileName);
  };

  cRecordingsFolderInfo(cRecordings &Recordings);
  ~cRecordingsFolderInfo(void);

  // caller must delete the cInfo object!
  // returns NULL if folder doesn't exists
  // will rebuild tree if recordings' state has changed
  // is thread-safe
  cFolderInfo  *Get(const char *Folder);

  cString DebugOutput(void) const;
};

#endif // __RECFOLDERINFO_H