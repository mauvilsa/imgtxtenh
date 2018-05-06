/**
 * Handwritten text recognition preprocessing functions
 *
 * @version $Revision: 170 $$Date:: 2015-02-26 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_HTRPREP_H__
#define __MV_HTRPREP_H__

#include "mem.h"

#define DEG_TO_RAD 0.01745329251994329547
#define RAD_TO_DEG 57.2957795130823228646

int rlsa4_graym(gray** img, int imgW, int imgH, char op, int* lengths, char negate, gray** res);
int skew_angle(gray** img,int imgW,int imgH,double amax,double astep,double* _angle,int* _hist);

#endif
