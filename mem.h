/**
 * Functions for allocating memory
 *
 * @version $Revision:: 186   $$Date:: 2015-05-11 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_MEM_H__
#define __MV_MEM_H__

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef FAILURE
#define FAILURE 1
#endif
#ifndef SUCCESS
#define SUCCESS 0
#endif

#define IMGIO_GRAYMAX 255
#define limit_gray(a) (gray)(((a)<0?0:(a))>IMGIO_GRAYMAX?IMGIO_GRAYMAX:((a)<0?0:(a)))

typedef int I1;
typedef float F1;
typedef double D1;

typedef unsigned char gray;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} rgb;

typedef rgb pixel;

/*typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} pixel;*/

typedef struct {
  I1 idx;
  I1 val;
} iI1;

typedef struct {
  I1 idx;
  F1 val;
} iF1;

typedef struct {
  int x;
  int y;
} IV2;

void bfree( void* mat, int brd );

void zeros_I1( I1* vec, int D );
void zeros_F1( F1* vec, int D );
void zeros_D1( D1* vec, int D );

void copy_I1( I1* src, int D, I1* des );
void copy_F1( F1* src, int D, F1* des );
void copy_D1( D1* src, int D, D1* des );

int clone_graym( gray** mat, int imW, int imH, gray*** _clon );
int clone_pixelm( pixel** mat, int imW, int imH, pixel*** _clon );

int malloc_grayv( int size, gray** _vec, char clr );
int malloc_pixelv( int size, pixel** _vec, char clr );
int malloc_I1v( int size, I1** _vec, char clr );
int malloc_F1v( int size, F1** _vec, char clr );
int malloc_D1v( int size, D1** _vec, char clr );
int malloc_iI1v( int size, iI1** _vec, char clr );
int malloc_iF1v( int size, iF1** _vec, char clr );
int malloc_IV2v( int size, IV2** _vec, char clr );

int malloc_graym( int imW, int imH, gray*** _im, char clr );
int malloc_pixelm( int imW, int imH, pixel*** _im, char clr );
int malloc_I1m( int R, int C, I1*** _mat, char clr );
int malloc_F1m( int R, int C, F1*** _mat, char clr );
int malloc_D1m( int R, int C, D1*** _mat, char clr );

int malloc_F1t( int D, F1*** _mat, char clr );
int malloc_D1t( int D, D1*** _mat, char clr );

void nullfree(void* ptr);
int mclone(char** mat, int R, int C, int size, char*** _clon);
int mem(int size,char clr,char** _p);
int mmem(int R,int C,int size,char clr,char*** _mat);
int bmem(int R,int C,int size,char clr,int brd,char*** _mat);
int tmem(int D,int size,char clr,char*** _mat);
void vrmem_index( int size, int* R, int C, char** mat );
int vrmem( int size, int tnnz, int* R, int C, char clr, char*** _mat, int** _R );

#endif
