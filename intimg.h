/**
 * Functions related to integral images
 *
 * @version $Revision:: 219   $$Date:: 2016-07-24 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_INTIMG_H__
#define __MV_INTIMG_H__

#include "mem.h"
//#include "morph.h"

#define ENH_SAUVOLA        1
#define ENH_SAUVOLA_SDMAX  2
#define ENH_WOLF           3

//#define II1 unsigned
//#define II2 long long
typedef unsigned II1;
typedef long long II2;

int malloc_II1( int imgW, int imgH, II1*** _II, char clr );
int malloc_II2( int imgW, int imgH, II2*** _II, char clr );

int compII12_graym(gray** img, gray** alph, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt);
int cropsum_II(II1** ii1, int xmin, int ymin, int cropW, int cropH);
int mean_II(II1** ii1, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _mean);
int sd_II(II1** ii1, II2** ii2, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _sd);
int meanSd_II(II1** ii1, II2** ii2, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _mean, float* _sd);
//int enhSauvola_pixelm(pixel** img, gray** alph, int imgW, int imgH, int winW, float prm, float slp);
int enhLocal_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float prm, float slp, int type);
int enhSauvola_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float prm, float slp);
int enhSauvola_sample_prm_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float slp, double *_prm, int subsamp, float prmthr);
//int maskExtend_graym(gray **img, gray **alph, int imgW, int imgH, int winW, float prm, float seR);

#endif
