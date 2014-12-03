#include "imagecreator.h"

cImageCreator::cImageCreator(void) {
	surface = NULL;
	cr = NULL;
}

cImageCreator::~cImageCreator() {
    if (cr)
	    cairo_destroy (cr);
    if (surface)
	    cairo_surface_destroy (surface);
}

bool cImageCreator::InitCairoImage(int width, int height) {
	this->width = width;
	this->height = height;
	if (width < 1 || height < 1 || width > 1920 || height > 1080)
		return false;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
    return true;
}

/**********************************************************************************
* Public Functions
**********************************************************************************/

///< 0       draws the entire ellipse
///< 1..4    draws only the first, second, third or fourth quadrant, respectively
///< 5..8    draws the right, top, left or bottom half, respectively
///< -1..-4  draws the inverted part of the given quadrant
///< If Quadrants is not 0, the coordinates are those of the actual area, not
///< the full circle!
void cImageCreator::DrawEllipse(tColor color, int quadrants) {
    if (!cr || !surface)
        return;
	//center of the ellipse
	double x, y;
	//radius 
	double radius;
	//start and stop angle (radian)
	double arcStart, arcStop;
	//scaling factors
	double scaleX, scaleY;
	//draw inverted ellipse
	bool inverted;

	scaleX = width;
	scaleY = height;
	inverted = false;

	switch (quadrants) {
		case 0:
			x = 0.5;
			y = 0.5;
			radius = 0.5;
			arcStart = 0.0;
			arcStop = 2 * M_PI;
			break;
		case 1:
			x = 0.0;
			y = 1.0;
			radius = 1.0;
			arcStart = M_PI;
			arcStop = 2 * M_PI;
			break;
		case 2:
			x = 1.0;
			y = 1.0;
			radius = 1.0;
			arcStart = M_PI;
			arcStop = 2 * M_PI;
			break;
		case 3:
			x = 1.0;
			y = 0.0;
			radius = 1.0;
			arcStart = 0;
			arcStop = M_PI;
			break;
		case 4:
			x = 0.0;
			y = 0.0;
			radius = 1.0;
			arcStart = 0;
			arcStop = M_PI;
			break;
		case 5:
		    scaleX = 2 * width;
		    x = 0.0;
		    y = 0.5;
		    radius = 0.5;
		    arcStart = 0;
		    arcStop = 2* M_PI;
			break;
		case 6:
		    scaleY = 2 * height;
		    x = 0.5;
		    y = 0.5;
		    radius = 0.5;
		    arcStart = 0;
		    arcStop = 2* M_PI;
			break;
		case 7:
		    scaleX = 2 * width;
		    x = 0.5;
		    y = 0.5;
		    radius = 0.5;
		    arcStart = 0;
		    arcStop = 2* M_PI;
			break;
		case 8:
		    scaleY = 2 * height;
		    x = 0.5;
		    y = 0.0;
		    radius = 0.5;
		    arcStart = 0;
		    arcStop = 2* M_PI;
			break;
		case -1:
		    x = 0.0;
		    y = 1.0;
		    radius = 1.0;
		    arcStart = M_PI;
		    arcStop = 2* M_PI;
		    inverted = true;
		    break;
		case -2:
			x = 1.0;
			y = 1.0;
			radius = 1.0;
			arcStart = M_PI;
			arcStop = 2 * M_PI;
		    inverted = true;
		    break;
		case -3:
			x = 1.0;
			y = 0.0;
			radius = 1.0;
			arcStart = 0;
			arcStop = M_PI;
		    inverted = true;
			break;
		case -4:
			x = 0.0;
			y = 0.0;
			radius = 1.0;
			arcStart = 0;
			arcStop = M_PI;
		    inverted = true;
			break;
		default:
			x = 0.5;
			y = 0.5;
			radius = 0.5;
			arcStart = 0.0;
			arcStop = 2 * M_PI;
			break;
	}

	SetColor(color);
	//Draw Background for inverted ellipses
	if (inverted) {
	    cairo_rectangle(cr, 0.0, 0.0, width, height);
	    cairo_fill (cr);
	    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	}
	//Draw Ellipse
	cairo_scale(cr, scaleX, scaleY);
	cairo_arc(cr, x, y, radius, arcStart, arcStop);
	cairo_fill(cr);
}

cImage *cImageCreator::GetImage(void) {
    if (!cr || !surface)
        NULL;
	unsigned char *data = cairo_image_surface_get_data(surface);
    cImage *image = new cImage(cSize(width, height), (tColor*)data);
    return image;
}

/**********************************************************************************
* Private Functions
**********************************************************************************/

void cImageCreator::SetColor(tColor color) {
    if (!cr || !surface)
        return;
	tIndex tAlpha = (color & 0xFF000000) >> 24;
    tIndex tRed = (color & 0x00FF0000) >> 16;
    tIndex tGreen = (color & 0x0000FF00) >> 8;
    tIndex tBlue = (color & 0x000000FF);
    
    double a = (int)tAlpha / (double)255;
    double r = (int)tRed / (double)255;
    double g = (int)tGreen / (double)255;
    double b = (int)tBlue / (double)255;

    cairo_set_source_rgba(cr, r, g, b, a);
}
