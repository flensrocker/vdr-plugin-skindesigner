#ifndef __IMAGECREATOR_H
#define __IMAGECREATOR_H

#include <cairo.h>
#include <vdr/osd.h>
#include <string>
#include <sstream>

using namespace std;

class cImageCreator {
private:
    int width;
    int height;
    cairo_surface_t *surface;
    cairo_t *cr;
    void SetColor(tColor color);
public:
    cImageCreator(void);
    virtual ~cImageCreator();
    bool InitCairoImage(int width, int height);
    void DrawEllipse(tColor color, int quadrants = 0);
    cImage *GetImage(void);
};

#endif //__IMAGECREATOR_H
