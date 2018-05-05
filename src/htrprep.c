/**
 * Handwritten text recognition preprocessing functions
 *
 * @version $Revision: 170 $$Date:: 2015-02-26 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "htrprep.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


int rlsa4_graym(gray** img, int imgW, int imgH, char op, int* lengths, char negate, gray** res) {
  int n;
  if( negate )
    for( n=imgW*imgH-1; n>=0; n-- )
      img[0][n] = 255-img[0][n];

  gray **freeimg = NULL;
  if( img == res ) {
    if( clone_graym(img,imgW,imgH,&freeimg) ) {
      fprintf(stderr,"rlsa4_graym: error: unable to reserve memory\n");
      return FAILURE;
    }
    img = freeimg;
  }
  else
    memcpy(res[0],img[0],imgW*imgH*sizeof(gray));

  /*** bit 0 -> '-' ***/
  if( op & 0x01 ) {
    //fprintf(stderr,"rlsa4_graym: '-'\n");
    int length = lengths[0];
    int y;
    for( y=imgH-1; y>=0; y-- ) {
      gray *imgy = img[y];
      gray *resy = res[y];

      int x = imgW-1;
      while( x>0 ) {
        int xx, prevx = x;
        /* move all white pixels */
        while( x>=0 && imgy[x] )
          x--;
        /* change white pixels to black if less than length */
        if( x>=0 && (prevx-x)<length && prevx!=imgW-1 )
          for( xx=prevx; xx>x; xx-- )
            resy[xx] = 0;
        /* move all black pixels */
        while( x>=0 && !imgy[x] )
          x--;
      }
    }
  }

  /*** bit 1 -> '|' ***/
  if( (op>>1) & 0x01 ) {
    //fprintf(stderr,"rlsa4_graym: '|'\n");
    int length = lengths[1];
    int x;
    for( x=imgW-1; x>=0; x-- ) {

      int y = imgH-1;
      while( y>0 ) {
        int yy, prevy = y;
        /* move all white pixels */
        while( y>=0 && img[y][x] )
          y--;
        /* change white pixels to black if less than length */
        if( y>=0 && (prevy-y)<length && prevy!=imgH-1 )
          for( yy=prevy; yy>y; yy-- )
            res[yy][x] = 0;
        /* move all black pixels */
        while( y>=0 && !img[y][x] )
          y--;
      }
    }
  }

  /*** bit 2 -> '/' ***/
  if( (op>>2) & 0x01 ) {
    //fprintf(stderr,"rlsa4_graym: '/'\n");
    int length = lengths[2];

    int n;
    for( n=-imgH; n<imgW; n++ ) {
      int x = n<0 ? 0 : n ;
      int y = n<0 ? imgH+n : imgH-1 ;

      while( x<imgW-1 && y>0 ) {
        int xx, yy, prevx = x, prevy = y;
        /* move all white pixels */
        while( x<=imgW-1 && y>=0 && img[y][x] ) {
          x++;
          y--;
        }
        /* change white pixels to black if less than length */
        if( x<=imgW-1 && y>=0 &&
            (prevy-y)<length &&
            prevx!=0 && prevy!=imgH-1 )
          for( xx=prevx, yy=prevy; yy>y; xx++, yy-- )
            res[yy][xx] = 0;
        /* move all black pixels */
        while( x<=imgW-1 && y>=0 && !img[y][x] ) {
          x++;
          y--;
        }
      }
    }
  }

  /*** bit 3 -> '\' ***/
  if( (op>>3) & 0x01 ) {
    //fprintf(stderr,"rlsa4_graym: '\\'\n");
    int length = lengths[3];

    int n;
    for( n=-imgH; n<imgW; n++ ) {
      int x = n<0 ? imgW-1 : imgW-1-n ;
      int y = n<0 ? imgH+n : imgH-1 ;

      while( x>0 && y>0 ) {
        int xx, yy, prevx = x, prevy = y;
        /* move all white pixels */
        while( x>=0 && y>=0 && img[y][x] ) {
          x--;
          y--;
        }
        /* change white pixels to black if less than length */
        if( x>=0 && y>=0 &&
            (prevy-y)<length &&
            prevx!=imgW-1 && prevy!=imgH-1 )
          for( xx=prevx, yy=prevy; yy>y; xx--, yy-- )
            res[yy][xx] = 0;
        /* move all black pixels */
        while( x>=0 && y>=0 && !img[y][x] ) {
          x--;
          y--;
        }
      }
    }
  }

  if( negate ) {
    for( n=imgW*imgH-1; n>=0; n-- )
      res[0][n] = 255-res[0][n];
    if( freeimg == NULL )
      for( n=imgW*imgH-1; n>=0; n-- )
        img[0][n] = 255-img[0][n];
  }

  if( freeimg != NULL )
    free(freeimg);

  return SUCCESS;
}


void histogram(double* vec,int D,int bins,int* hist)
{
  int d;
  double min,max;

  for(d=D-1,min=vec[d],max=vec[d];d--;) {
    if(min>vec[d])
      min=vec[d];
    else if(max<vec[d])
      max=vec[d];
  }
  if(min!=max)
    max=(double)(bins-1)/(max-min);
  else
    max=0.0;
  for(d=bins;d--;)
    hist[d]=0;
  for(d=D;d--;)
    hist[(int)((vec[d]-min)*max+0.5)]++;
}

int skew_angle(gray** img,int imgW,int imgH,double amax,double astep,double* _angle,int* _hist)
{
  int i,j,n,N,m,M;
  double a,angle,var,cos_a,sin_a,s,s2;
  double *proj;
  int *hist;

  N=0;
  //for(angle=-amax;angle<=amax;angle+=astep)
  a=amax+astep/2;
  for(angle=-amax;angle<a;angle+=astep)
    N++;

  M=0;
  for(i=imgH;i--;) {
    gray *imgi = img[i];
    for(j=imgW;j--;)
      if(!imgi[j])
        M++;
  }

  if(_hist!=NULL) {
    hist = _hist;
    if(malloc_D1v(M,&proj,0))
      return EXIT_FAILURE;
  }
  else {
    if( malloc_I1v(imgH,&hist,0) ||
        malloc_D1v(M,&proj,0) )
      return EXIT_FAILURE;
  }

  var = 0.0;
  angle = 0.0;
  for(n=0,a=-amax;n<N;n++,a+=astep) {
    cos_a = cos(a);
    sin_a = sin(a);
    m = 0;
    for(i=imgH;i--;) {
      gray *imgi = img[i];
      for(j=imgW;j--;)
        if(!imgi[j]) {
          proj[m] = i*cos_a+j*sin_a;
          m++;
        }
    }
    histogram(proj,M,imgH,hist);
    s = 0.0;
    s2 = 0.0;
    for(i=imgH;i--;) {
      s += hist[i];
      s2 += hist[i]*hist[i];
    }
    s /= imgH;
    s2 = s2/imgH-s*s;
    if(var<s2) {
      angle = a;
      var = s2;
    }
  }

  *_angle = angle;

  free(proj);
  if(_hist!=NULL) {
    cos_a = cos(angle);
    sin_a = sin(angle);
    m = 0;
    for(i=imgH;i--;) {
      gray *imgi = img[i];
      for(j=imgW;j--;)
        if(!imgi[j]) {
          proj[m] = i*cos_a+j*sin_a;
          m++;
        }
    }
    histogram(proj,M,imgH,hist);
  }
  else
    free(hist);

  return EXIT_SUCCESS;
}
