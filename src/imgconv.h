/**
 * Image conversion functions
 *
 * @version $Revision: 203 $$Date:: 2015-11-17 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_IMGCONV_H__
#define __MV_IMGCONV_H__

#include "mem.h"

#include <stdio.h>

int fscan_floatv(FILE* file,float* vec,int maxD,FILE* mess);
void stretch_graym(gray **img, gray **alph, int imgW, int imgH, float satu);
int regstretch_graym( gray **img, gray **reg, int imgW, int imgH, float satu );
int rgbm2graym_proj(pixel** img, int imgW, int imgH, gray** out, float* base, int D, int hsv);
int rgbm2graym_proj_stretch(pixel** img, int imgW, int imgH, gray** out, float* base, int D, float satu, int hsv);
int rgbm2graym_stretch(pixel** img, int imgW, int imgH, gray** out, float satu);
void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v );

#endif
