#ifndef __NOPACITY_IMAGELOADER_H
#define __NOPACITY_IMAGELOADER_H

#include <cairo.h>
#include <librsvg/rsvg.h>
#ifndef LIBRSVG_CHECK_VERSION // Workaround for librsvg < 2.36.2
    #include <librsvg/rsvg-cairo.h>
    #include <librsvg/librsvg-features.h>
#endif
#include <jpeglib.h>
#include <setjmp.h>
#include <vdr/osd.h>
#include <vdr/tools.h>

//
// Image importers
//
class cImageImporter {
public:
    cImageImporter() {};
    virtual ~cImageImporter() {};
    virtual bool LoadImage(const char *path) { return false; };
    virtual void DrawToCairo(cairo_t *cr) {};
    virtual void GetImageSize(int &width, int &height) {};
};

// Image importer for PNG
class cImageImporterPNG : public cImageImporter {
public:
    cImageImporterPNG();
    ~cImageImporterPNG();
    bool LoadImage(const char *path);
    void DrawToCairo(cairo_t *cr);
    void GetImageSize(int &width, int &height);
private:
    cairo_surface_t *surface;
};

// Image importer for SVG
#if !LIBRSVG_CHECK_VERSION(2, 36, 0)
    #error librsvg version 2.36.0 or above required!
#endif

class cImageImporterSVG : public cImageImporter {
public:
    cImageImporterSVG();
    ~cImageImporterSVG();
    bool LoadImage(const char *path);
    void DrawToCairo(cairo_t *cr);
    void GetImageSize(int &width, int &height);
    static void InitLibRSVG();
private:
    RsvgHandle *handle;
};

// Image importer for JPG
#if BITS_IN_JSAMPLE != 8
    #error libjpeg has to be compiled with 8-bit samples!
#endif

class cImageImporterJPG : public cImageImporter {
public:
    cImageImporterJPG();
    ~cImageImporterJPG();
    bool LoadImage(const char *path);
    void DrawToCairo(cairo_t *cr);
    void GetImageSize(int &width, int &height);
private:
    j_decompress_ptr cinfo;
    FILE *infile;
};

//
// Image loader class
//
class cImageLoader {
private:
    cImageImporter *importer;
public:
    cImageLoader();
    virtual ~cImageLoader();
    cImage *CreateImage(int width, int height, bool preserveAspect = true);
    bool LoadImage(std::string Path, std::string FileName, std::string Extension);
    bool LoadImage(const char *fullpath);
};

#endif //__NOPACITY_IMAGELOADER_H
