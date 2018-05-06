/**
 * Image conversion functions
 *
 * @version $Revision: 204 $$Date:: 2015-11-17 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "imgconv.h"

#include <stdlib.h>
//#include <math.h>

int fscan_floatv(FILE* file,float* vec,int maxD,FILE* mess) {
  int err=0, d=0, ch=0;
  char cval[32];
  float dummy;

  while(1) {
    ch=fgetc(file);
    if(ch=='\n' || feof(file))
      break;
    else if(ch==' ' || ch=='\t')
      continue;
    else {
      ungetc(ch,file);
      int p=0;
      do {
        ch=fgetc(file);
        if(p<32)
          cval[p++]=ch;
        else
          err--;
      } while(!(ch==' ' || ch=='\t' || ch=='\n' || ch=='\r'));
      cval[p-1]='\0';
      ungetc(ch,file);
      float *pf=vec+d;
      if(d>=maxD)
        pf=&dummy;
      int rf=sscanf(cval,"%f",pf);
      if(rf==1)
        d++;
      else
        err--;
    }
  }

  if(err) {
    if(mess!=NULL)
      fprintf(mess,"fscan_floatv: error: problems reading vector\n");
    return err;
  }
  if(d>maxD && mess!=NULL)
      fprintf(mess,"fscan_floatv: warning: dimensionality of vector higher than expected\n");

  return d;
}

static inline int hist_graym(gray **img, gray **alph, int imgW, int imgH, int *hist) {
  int n;
  for(n=255;n>=0;n--)
    hist[n] = 0;
  int cnt = 0;
  for(n=imgW*imgH-1;n>=0;n--)
    if(alph==NULL || alph[0][n]!=0) {
      hist[img[0][n]]++;
      cnt++;
    }
  return cnt;
}

void stretch_graym(gray **img, gray **alph, int imgW, int imgH, float satu) {
  int hist[256];
  int cnt = hist_graym(img,alph,imgW,imgH,hist);
  int thr = satu*cnt+0.5;
  thr = thr==0 ? 1 : thr;

  int min;
  cnt = hist[0];
  for(min=0,cnt=0;min<=255;min++) {
    cnt += hist[min];
    if(cnt>thr)
      break;
  }
  int max;
  cnt = hist[255];
  for(max=255,cnt=0;max>=0;max--) {
    cnt += hist[max];
    if(cnt>thr)
      break;
  }

  double fact = 255.0/(max-min);
  int n;
  for(n=imgW*imgH-1;n>=0;n--)
    if( alph==NULL || alph[0][n]!=0 )
      img[0][n] = limit_gray( fact*(img[0][n]-min)+0.5 );
}

int regstretch_graym( gray **img, gray **reg, int imgW, int imgH, float satu ) {
  int **hist = NULL;
  int *cnt = NULL;
  double *fact = NULL;
  if( malloc_I1m( 256, 256, &hist, TRUE ) ||
      malloc_I1v( 2*256, &cnt, TRUE ) ||
      malloc_D1v( 256, &fact, FALSE ) )
    return FAILURE;
  int *min = cnt+256;

  int n;
  for( n=imgW*imgH-1; n>=0; n-- ) {
    hist[reg[0][n]][img[0][n]] ++;
    cnt[reg[0][n]] ++;
  }

  for( n=0; n<256; n++ )
    if( cnt[n] > 0 ) {
      int sum, max;
      int thr = satu*cnt[n]+0.5 ;
      thr = thr==0 ? 1 : thr ;

      for( min[n]=0,sum=0; min[n]<=255; min[n]++ ) {
        sum += hist[n][min[n]];
        if( sum > thr )
          break;
      }
      for( max=255,sum=0; max>=0; max-- ) {
        sum += hist[n][max];
        if( sum > thr )
          break;
      }

      fact[n] = max > min[n] ? 255.0/(max-min[n]) : 1.0 ;
    }

  for( n=imgW*imgH-1; n>=0; n-- ) {
    int r = reg[0][n];
    img[0][n] = limit_gray( fact[r]*(img[0][n]-min[r])+0.5 );
  }

  free(hist);
  free(cnt);
  free(fact);

  return SUCCESS;
}


/*static inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = { a };
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}*/

// should be much more precise with large b
static inline double fastPrecisePow(double a, double b) {
  // calculate approximation with fraction of the exponent
  int e = (int) b;
  union {
    double d;
    int x[2];
  } u = { a };
  u.x[1] = (int)((b - e) * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
 
  // exponentiation by squaring with the exponent's integer part
  // double r = u.d makes everything much slower, not sure why
  double r = 1.0;
  while (e) {
    if (e & 1) {
      r *= a;
    }
    a *= a;
    e >>= 1;
  }
 
  return r * u.d;
}

static inline double gamma_correct(double v,double g) {
  v = ((v)<0.0?0.0:(v))>1.0?1.0:((v)<0.0?0.0:(v));
  //return pow(v,g);
  //return exp(g*log(v));
  //return fastPow(v,g);
  return fastPrecisePow(v,g);
}

int rgbm2graym_proj(pixel** img, int imgW, int imgH, gray** out, float* base, int D, int hsv) {
  float min = base[0];
  float rang = base[1];
  pixel *imgv = img[0];
  gray *outv = out[0];
  base = base+2;
  D -= 2;

  float gamma = 0;
  if( D==4 || D==10 || D==14 ) {
    gamma = base[D-1];
    rang = 1/(rang-min);
    D --;
  }
  else
    rang = IMGIO_GRAYMAX/(rang-min);

  int n;
  float pv;
  switch(D) {
  case 3:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        pv =
          base[0]*h + base[1]*s + base[2]*v;
      }
      else
        pv =
          base[0]*p.r + base[1]*p.g + base[2]*p.b;
      if(gamma)
        outv[n] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(pv-min),gamma)+0.5 );
      else
        outv[n] = limit_gray( rang*(pv-min)+0.5 );
    }
    break;
  case 9:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        pv =
          base[0]*h + base[1]*s + base[2]*v +
          base[3]*h*s + base[4]*s*v + base[5]*h*v +
          base[6]*h*h + base[7]*s*s + base[8]*v*v;
      }
      else
        pv =
          base[0]*p.r + base[1]*p.g + base[2]*p.b +
          base[3]*p.r*p.g + base[4]*p.g*p.b + base[5]*p.r*p.b +
          base[6]*p.r*p.r + base[7]*p.g*p.g + base[8]*p.b*p.b;
      if(gamma)
        outv[n] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(pv-min),gamma)+0.5 );
      else
        outv[n] = limit_gray( rang*(pv-min)+0.5 );
    }
    break;
  case 13:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        pv =
          base[0]*h + base[1]*s + base[2]*v +
          base[3]*h*s + base[4]*s*v + base[5]*h*v +
          base[6]*h*h + base[7]*s*s + base[8]*v*v +
          base[9]*h*s*v +
          base[10]*h*h*h + base[11]*s*s*s + base[12]*v*v*v;
      }
      else
        pv =
          base[0]*p.r + base[1]*p.g + base[2]*p.b +
          base[3]*p.r*p.g + base[4]*p.g*p.b + base[5]*p.r*p.b +
          base[6]*p.r*p.r + base[7]*p.g*p.g + base[8]*p.b*p.b +
          base[9]*p.r*p.g*p.b +
          base[10]*p.r*p.r*p.r + base[11]*p.g*p.g*p.g + base[12]*p.b*p.b*p.b;
      if(gamma)
        outv[n] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(pv-min),gamma)+0.5 );
      else
        outv[n] = limit_gray( rang*(pv-min)+0.5 );
    }
    break;
  default:
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int compare_float(const void *a, const void *b) {
  if (*(float*)a < *(float*)b)
    return -1;
  else if (*(float*)a > *(float*)b)
    return 1;
  else
    return 0;
}

int rgbm2graym_proj_stretch(pixel** img, int imgW, int imgH, gray** out, float* base, int D, float satu, int hsv) {
  float *tmpv;
  int N = imgW*imgH;
  int err = mem((N+N)*sizeof(float),0,(char**)(&tmpv));
  if(err)
    return err;

  float gamma = 0;
  if( D==4 || D==10 || D==14 ) {
    gamma = base[D-1];
    D --;
  }

  float *tmps = tmpv+N;
  pixel *imgv = img[0];
  int n;
  switch(D) {
  case 3:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        tmps[n] = tmpv[n] =
          base[0]*h + base[1]*s + base[2]*v;
      }
      else
        tmps[n] = tmpv[n] =
          base[0]*p.r + base[1]*p.g + base[2]*p.b;
    }
    break;
  case 9:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        tmps[n] = tmpv[n] =
          base[0]*h + base[1]*s + base[2]*v +
          base[3]*h*s + base[4]*s*v + base[5]*h*v +
          base[6]*h*h + base[7]*s*s + base[8]*v*v;
      }
      else
        tmps[n] = tmpv[n] =
          base[0]*p.r + base[1]*p.g + base[2]*p.b +
          base[3]*p.r*p.g + base[4]*p.g*p.b + base[5]*p.r*p.b +
          base[6]*p.r*p.r + base[7]*p.g*p.g + base[8]*p.b*p.b;
    }
    break;
  case 13:
    for(n=imgW*imgH-1;n>=0;n--) {
      pixel p = imgv[n];
      if(hsv) {
        float h,s,v;
        RGBtoHSV(p.r/255.0,p.g/255.0,p.b/255.0,&h,&s,&v);
        tmps[n] = tmpv[n] =
          base[0]*h + base[1]*s + base[2]*v +
          base[3]*h*s + base[4]*s*v + base[5]*h*v +
          base[6]*h*h + base[7]*s*s + base[8]*v*v +
          base[9]*h*s*v +
          base[10]*h*h*h + base[11]*s*s*s + base[12]*v*v*v;
      }
      else
        tmps[n] = tmpv[n] =
          base[0]*p.r + base[1]*p.g + base[2]*p.b +
          base[3]*p.r*p.g + base[4]*p.g*p.b + base[5]*p.r*p.b +
          base[6]*p.r*p.r + base[7]*p.g*p.g + base[8]*p.b*p.b +
          base[9]*p.r*p.g*p.b +
          base[10]*p.r*p.r*p.r + base[11]*p.g*p.g*p.g + base[12]*p.b*p.b*p.b;
    }
    break;
  default:
    return EXIT_FAILURE;
  }

  qsort(tmps,N,sizeof(float),compare_float);

  n = (int)(satu*N+0.5);
  float min = tmps[n];
  float max = tmps[N-n-1];

  gray *outv = out[0];

  if(gamma) {
    float rang = 1/(max-min);
    for(n=N-1;n>=0;n--)
      outv[n] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(tmpv[n]-min),gamma)+0.5 );
  }
  else {
    float rang = IMGIO_GRAYMAX/(max-min);
    for(n=N-1;n>=0;n--)
      outv[n] = limit_gray( rang*(tmpv[n]-min)+0.5 );
  }

  free(tmpv);

  return EXIT_SUCCESS;
}

int rgbm2graym_stretch(pixel** img, int imgW, int imgH, gray** out, float satu) {
  float base[3] = {0.299,0.587,0.114};
  return rgbm2graym_proj_stretch(img,imgW,imgH,out,base,3,satu,0);
}


#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )
#define MIN3( A, B, C ) ( (A) < (B) ? MIN2(A, C) : MIN2(B, C) )
#define MAX3( A, B, C ) ( (A) > (B) ? MAX2(A, C) : MAX2(B, C) )

// r,g,b,h,s,v = [0,1]
void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v ) {
  *h = 0.0;
  *s = MIN3(r,g,b);
  *v = MAX3(r,g,b);

  if( (*s) != (*v) ) { // not gray
    if( r == *v ) { // red hue
      *h = ( g - b ) / ( 6.0 * (*v - *s) );
      if(*h < 0)
        *h -= 1.0;
    } else {
      if ( g == *v ) { // green hue
        *h = 1.0/3.0 + ( b - r ) / ( 6.0 * (*v - *s) );
      } else /* b == *v */ { // blue hue
        *h = 2.0/3.0 + ( r - g ) / ( 6.0 * (*v - *s) );
      }
    }
    *s = 1.0-(*s)/(*v);
  }
  else // gray
    *s = 0.0;
}
