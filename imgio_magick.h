/**
 * Functions for reading and writing images
 *
 * @version $Revision:: 207   $$Date:: 2015-11-24 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_IMGIO_MAGICK_H__
#define __MV_IMGIO_MAGICK_H__

#include "mem.h"

#include <stdio.h>
#include <magick/MagickCore.h>

#ifndef GetPixelGray
#define GetPixelAlpha(pixel) (QuantumRange-(pixel)->opacity)
#define GetPixelGray(pixel) ((pixel)->red)
#define GetPixelRed(pixel) ((pixel)->red)
#define GetPixelGreen(pixel) ((pixel)->green)
#define GetPixelBlue(pixel) ((pixel)->blue)
#define SetPixelGreen(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelBlue(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelRed(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelRgb(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
}
#endif

typedef struct {
  int width; // image width in pixels
  int height; // image height in pixels
  float res_x; // horizontal spatial resolution in pixels per cm (ppc)
  float res_y; // vertical spatial resolution in pixels per cm (ppc)
  unsigned char is_gray;   // whether image is grayscale or RGB
  unsigned char is_opaque; // whether image has alpha channel or not
  ImageInfo *info;
  Image *image;
} Img;

void free_Img( Img *img );
Img* create_img( ImageInfo* image_info, Image* image );
int readimg_magick( char const* fname, Img** _img, FILE* logfile );
int scanimg_magick( FILE* file, Img** _img, FILE* logfile );
int writeimg_magick( char const* fname, Img* img, FILE* logfile );
int printimg_magick( FILE* file, char const* format, Img* img, FILE* logfile );

int getalpha_magick_graym( Img* img, gray* gimg );
int getpixels_magick_graym( Img* img, gray* gimg );
int getpixels_magick_pixelm( Img* img, pixel* cimg );
int setpixels_magick_graym( Img* img, gray* gimg );
int setpixels_magick_grayalphm( Img* img, gray* gimg, gray* alph );
int setpixels_magick_pixelm( Img* img, pixel* cimg );

int togray_magick( Img* img );
void set_density_magick( Img* img, double density );

int add_border_magick( Img* img, size_t size, PixelPacket color );

#endif
