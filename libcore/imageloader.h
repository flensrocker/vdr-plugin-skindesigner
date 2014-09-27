#ifndef __NOPACITY_IMAGELOADER_H
#define __NOPACITY_IMAGELOADER_H

#define X_DISPLAY_MISSING

#include <vdr/osd.h>
#include <vdr/skins.h>
#include <Magick++.h>
#include "imagemagickwrapper.h"

using namespace Magick;

class cImageLoader : public cImageMagickWrapper {
public:
    cImageLoader();
    ~cImageLoader();
    cImage GetImage();
    bool LoadImage(const char *path, int width, int height);
    void DeterminateChannelLogoSize(int &width, int &height);
private:
};

#endif //__NOPACITY_IMAGELOADER_H
