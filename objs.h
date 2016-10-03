/**
 * Connected component functions for images
 *
 * @version $Revision: 171 $$Date:: 2015-02-26 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_OBJS_H__
#define __MV_OBJS_H__

#include "mem.h"

#define bcalloc_intm(R,C,B,M) bmem((R),(C),sizeof(int),1,(B),(char***)(M))

int objlabel4(gray** img,int imgW,int imgH,int** out,int* _obj);
int objlabel8(gray** img,int imgW,int imgH,int** out,int* _obj);
int objinfo(int** img,int imgW,int imgH,int obj,int** bbox,int* area,float** cent);
int objmeanWH(int** img,int imgW,int imgH,int obj,int** meanWH);
//void objarea(int** img,int imgW,int imgH,int obj,int* _area);
//void objperim(int** img,int imgW,int imgH,int obj,int* _perim);
//void objcompactness(int** img,int imgW,int imgH,int obj,double* _comp);
//int objbbox(int** img,int imgW,int imgH,int obj,int* _minX,int* _maxX,int* _minY,int* _maxY);
//void objcentroid(int** img,int imgW,int imgH,int obj,double* _cX,double* _cY);
//void objorient(int** img,int imgW,int imgH,int obj,double* _orient);

#endif
