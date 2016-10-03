/**
 * Functions related to integral images
 *
 * @version $Revision: 221 $$Date:: 2016-08-19 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "intimg.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int malloc_II1( int imW, int imH, II1*** _II, char clr )
  { return mmem(imW,imH,sizeof(II1),clr,(char***)_II); }
int malloc_II2( int imW, int imH, II2*** _II, char clr )
  { return mmem(imW,imH,sizeof(II2),clr,(char***)_II); }

int compII12_graym(gray** img, gray** alph, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt) {
  int x,y;

  if(imgW*imgH>66051 && sizeof(II2)<=4)
    fprintf(stderr,"compII12_graym: warning: posible II overflow.\n");

  if(alph!=NULL)
    for(y=0;y<imgH;y++)
      for(x=0;x<imgW;x++)
        if(alph[y][x]==0)
          img[y][x]=0;

  if(alph!=NULL)
    cnt[0][0] = alph[0][0] ? 1 : 0;
  ii1[0][0] = (II1)img[0][0];
  if(ii2!=NULL)
    ii2[0][0] = (II2)img[0][0]*(II2)img[0][0];
  for(y=1;y<imgH;y++) {
    if(alph!=NULL)
      cnt[y][0] = cnt[y-1][0] + ( alph[y][0] ? 1 : 0 );
    ii1[y][0] = ii1[y-1][0]+(II1)img[y][0];
    if(ii2!=NULL)
      ii2[y][0] = ii2[y-1][0]+(II2)img[y][0]*(II2)img[y][0];
  }
  for(x=1;x<imgW;x++) {
    if(alph!=NULL)
      cnt[0][x] = cnt[0][x-1] + ( alph[0][x] ? 1 : 0 );
    ii1[0][x] = ii1[0][x-1]+(II1)img[0][x];
    if(ii2!=NULL)
      ii2[0][x] = ii2[0][x-1]+(II2)img[0][x]*(II2)img[0][x];
  }
  for(y=1;y<imgH;y++)
    for(x=1;x<imgW;x++) {
      if(alph!=NULL)
        cnt[y][x] = cnt[y-1][x]+cnt[y][x-1]-cnt[y-1][x-1] + ( alph[y][x] ? 1 : 0 );
      ii1[y][x] = ii1[y-1][x]+ii1[y][x-1]-ii1[y-1][x-1]+(II1)img[y][x];
      if(ii2!=NULL)
        ii2[y][x] = ii2[y-1][x]+ii2[y][x-1]-ii2[y-1][x-1]+(II2)img[y][x]*(II2)img[y][x];
    }

  return EXIT_SUCCESS;
}

int cropsum_II(II1** ii1, int xmin, int ymin, int cropW, int cropH) {
  int xminm1 = xmin-1;
  int yminm1 = ymin-1;
  int xmax = xminm1+cropW;
  int ymax = yminm1+cropH;

  II1 S1 = ii1[ymax][xmax];
  if(yminm1>-1) {
    S1 -= ii1[yminm1][xmax];
    if(xminm1>-1)
      S1 += ii1[yminm1][xminm1]-ii1[ymax][xminm1];
  }
  else if(xminm1>-1)
    S1 -= ii1[ymax][xminm1];

  return S1;
}

int mean_II(II1** ii1, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _mean) {
  int xminm1 = xmin-1;
  int yminm1 = ymin-1;
  int xmax = xminm1+cropW;
  int ymax = yminm1+cropH;

  II1 S1 = ii1[ymax][xmax];
  if(yminm1>-1) {
    S1 -= ii1[yminm1][xmax];
    if(xminm1>-1)
      S1 += ii1[yminm1][xminm1]-ii1[ymax][xminm1];
  }
  else if(xminm1>-1)
    S1 -= ii1[ymax][xminm1];

  int numpix;
  if(cnt!=NULL) {
    numpix = cnt[ymax][xmax];
    if(yminm1>-1) {
      numpix -= cnt[yminm1][xmax];
      if(xminm1>-1)
        numpix += cnt[yminm1][xminm1]-cnt[ymax][xminm1];
    }
    else if(xminm1>-1)
      numpix -= cnt[ymax][xminm1];
  }
  else
    numpix = cropW*cropH;

  *_mean = (float)((int)S1)/((float)numpix);

  return EXIT_SUCCESS;
}

int sd_II(II1** ii1, II2** ii2, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _sd) {
  int xminm1 = xmin-1;
  int yminm1 = ymin-1;
  int xmax = xminm1+cropW;
  int ymax = yminm1+cropH;

  II1 S1 = ii1[ymax][xmax];
  II2 S2 = ii2[ymax][xmax];
  if(yminm1>-1) {
    S1 -= ii1[yminm1][xmax];
    S2 -= ii2[yminm1][xmax];
    if(xminm1>-1) {
      S1 += ii1[yminm1][xminm1]-ii1[ymax][xminm1];
      S2 += ii2[yminm1][xminm1]-ii2[ymax][xminm1];
    }
  }
  else if(xminm1>-1) {
    S1 -= ii1[ymax][xminm1];
    S2 -= ii2[ymax][xminm1];
  }

  int numpix;
  if(cnt!=NULL) {
    numpix = cnt[ymax][xmax];
    if(yminm1>-1) {
      numpix -= cnt[yminm1][xmax];
      if(xminm1>-1)
        numpix += cnt[yminm1][xminm1]-cnt[ymax][xminm1];
    }
    else if(xminm1>-1)
      numpix -= cnt[ymax][xminm1];
  }
  else
    numpix = cropW*cropH;

  float mean = (float)S1/(float)numpix;
  if(((float)S2/(float)numpix - mean*mean) <= 0.0)
    *_sd = 0;
  else
    *_sd = sqrt((float)S2/(float)numpix - mean*mean);
 
  return EXIT_SUCCESS;
}

int meanSd_II(II1** ii1, II2** ii2, II1** cnt, int xmin, int ymin, int cropW, int cropH, float* _mean, float* _sd) {
  int xminm1 = xmin-1;
  int yminm1 = ymin-1;
  int xmax = xminm1+cropW;
  int ymax = yminm1+cropH;

  II1 S1 = ii1[ymax][xmax];
  II2 S2 = ii2[ymax][xmax];
  if(yminm1>-1) {
    S1 -= ii1[yminm1][xmax];
    S2 -= ii2[yminm1][xmax];
    if(xminm1>-1) {
      S1 += ii1[yminm1][xminm1]-ii1[ymax][xminm1];
      S2 += ii2[yminm1][xminm1]-ii2[ymax][xminm1];
    }
  }
  else if(xminm1>-1) {
    S1 -= ii1[ymax][xminm1];
    S2 -= ii2[ymax][xminm1];
  }

  int numpix;
  if(cnt!=NULL) {
    numpix = cnt[ymax][xmax];
    if(yminm1>-1) {
      numpix -= cnt[yminm1][xmax];
      if(xminm1>-1)
        numpix += cnt[yminm1][xminm1]-cnt[ymax][xminm1];
    }
    else if(xminm1>-1)
      numpix -= cnt[ymax][xminm1];
  }
  else
    numpix = cropW*cropH;

  *_mean = (float)((int)S1)/((float)numpix);
  if(((float)S2/(float)numpix-(*_mean)*(*_mean)) <= 0.0)
    *_sd = 0;
  else
    *_sd = sqrt((float)S2/(float)numpix-(*_mean)*(*_mean));

  return EXIT_SUCCESS;
}

static inline void meanSdCW_II(int x, int y, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt, int winS, float* _mean, float* _sd) {
  int ymin = y-winS;
  ymin = ymin<0?0:ymin;
  int ymax = y+winS;
  ymax = ymax>=imgH?imgH-1:ymax;
  int xmin = x-winS;
  xmin = xmin<0?0:xmin;
  int xmax = x+winS;
  xmax = xmax>=imgW?imgW-1:xmax;
  meanSd_II(ii1,ii2,cnt,xmin,ymin,xmax-xmin+1,ymax-ymin+1,_mean,_sd);
}

/*int enhSauvola_pixelm(pixel** img, gray** alph, int imgW, int imgH, int winW, float prm, float slp) {
  gray** gimg = NULL;
  II1** cnt = NULL;
  II1** ii1 = NULL;
  II2** ii2 = NULL;

  int err = 0;
  err += malloc_graym(imgW,imgH,&gimg,FALSE);
  err += malloc_II1(imgW,imgH,&ii1,FALSE);
  err += malloc_II2(imgW,imgH,&ii2,FALSE);
  if(alph!=NULL)
    err += malloc_II1(imgW,imgH,&cnt,FALSE);

  if( err ) {
    fprintf(stderr,"enhSauvola_pixelm: error: unable to reserve memory\n");
    return EXIT_FAILURE;
  }

  int n;
  for( n=imgW*imgH-1; n>=0; n-- )
    gimg[0][n] = img[0][n].r;

  if( enhSauvola_graym(gimg,alph,imgW,imgH,&ii1,&ii2,&cnt,winW,prm,slp) )
    return EXIT_FAILURE;

  for( n=imgW*imgH-1; n>=0; n-- ) {
    img[0][n].r = gimg[0][n];
    gimg[0][n] = img[0][n].g;
  }

  if( enhSauvola_graym(gimg,alph,imgW,imgH,&ii1,&ii2,&cnt,winW,prm,slp) )
    return EXIT_FAILURE;

  for( n=imgW*imgH-1; n>=0; n-- ) {
    img[0][n].g = gimg[0][n];
    gimg[0][n] = img[0][n].b;
  }

  if( enhSauvola_graym(gimg,alph,imgW,imgH,&ii1,&ii2,&cnt,winW,prm,slp) )
    return EXIT_FAILURE;

  for( n=imgW*imgH-1; n>=0; n-- )
    img[0][n].b = gimg[0][n];

  free(gimg);
  free(ii1);
  free(ii2);
  if(alph!=NULL)
    free(cnt);

  return EXIT_SUCCESS;
}*/

static inline gray enhSauvola_single(int x, int y, gray** img, gray** alph, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt, int winS, float prm, float slp, float rng) {

  if(alph!=NULL && alph[y][x]==0)
    return (gray)255;

  float mu,sd;
  meanSdCW_II(x,y,imgW,imgH,ii1,ii2,cnt,winS,&mu,&sd);
  float thr = mu*(1+prm*((sd/rng)-1));
  if(slp==0.0) {
    if(img[y][x]>thr)
      return (gray)255;
    else
      return (gray)0;
  }
  else if(sd>1e-4) {
    float m = 255.0/(2*slp*sd);
    float c = 128-m*thr;
    return limit_gray(m*img[y][x]+c);
  }
  else {
    float fact=1.05;
    float W;
    for( W=fact*winS; W<2*imgW && W<2*imgH; W*=fact ) {
      int winS = (int)(W+0.5);
      float mu,sd;
      meanSdCW_II(x,y,imgW,imgH,ii1,ii2,cnt,winS,&mu,&sd);
      float thr = mu*(1+prm*((sd/rng)-1));
      if(sd>1e-4) {
        float m = 255.0/(2*slp*sd);
        float c = 128-m*thr;
        return limit_gray(m*img[y][x]+c);
      }
    }
  }

  return (gray)255;
}

static inline gray enhWolf_single(int x, int y, gray** img, gray** alph, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt, int winS, float prm, float slp, float rng, gray minv) {

  if(alph!=NULL && alph[y][x]==0)
    return (gray)255;

  float mu,sd;
  meanSdCW_II(x,y,imgW,imgH,ii1,ii2,cnt,winS,&mu,&sd);
  float thr = (1-prm)*mu+prm*minv+prm*(sd/rng)*(mu-minv);
  if(slp==0.0) {
    if(img[y][x]>thr)
      return (gray)255;
    else
      return (gray)0;
  }
  else if(sd>1e-4) {
    float m = 255.0/(2*slp*sd);
    float c = 128-m*thr;
    return limit_gray(m*img[y][x]+c);
  }
  else {
    float fact=1.05;
    float W;
    for( W=fact*winS; W<2*imgW && W<2*imgH; W*=fact ) {
      int winS = (int)(W+0.5);
      float mu,sd;
      meanSdCW_II(x,y,imgW,imgH,ii1,ii2,cnt,winS,&mu,&sd);
      float thr = (1-prm)*mu+prm*minv+prm*(sd/rng)*(mu-minv);
      if(sd>1e-4) {
        float m = 255.0/(2*slp*sd);
        float c = 128-m*thr;
        return limit_gray(m*img[y][x]+c);
      }
    }
  }

  return (gray)255;
}

static inline void minValmaxStd(gray** img, gray** alph, int imgW, int imgH, II1** ii1, II2** ii2, II1** cnt, int winS, gray* _minval, float* _maxstd) {

  gray minval = 255;
  float maxstd = 0.0;

  int y;
  for ( y=imgH-1; y>=0; y-- ) {
    gray *imgy = img[y];
    gray *alphy = alph!=NULL ? alph[y] : NULL;
    int x;
    for ( x=imgW-1; x>=0; x-- ) {
      if( alph==NULL || alphy[x]!=0 ) {
        float mu,sd;
        meanSdCW_II(x,y,imgW,imgH,ii1,ii2,cnt,winS,&mu,&sd);
        maxstd = maxstd < sd ? sd : maxstd ;
      }
      minval = minval > imgy[x] ? imgy[x] : minval ;
    }
  }

  *_minval = minval;
  *_maxstd = maxstd;
}

/*int enhSauvola_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float prm, float slp) {

  if(*_ii1==NULL || *_ii2==NULL) {
    int err = 0;
    err += malloc_II1(imgW,imgH,_ii1,FALSE);
    err += malloc_II2(imgW,imgH,_ii2,FALSE);
    if(alph!=NULL)
      err += malloc_II1(imgW,imgH,_cnt,FALSE);
    if(err) {
      fprintf(stderr,"enhSauvola_graym: error: unable to reserve memory\n");
      return EXIT_FAILURE;
    }
    _ii1[0][imgH-1][imgW-1]=0;
  }
  if(_ii1[0][imgH-1][imgW-1]==0)
    compII12_graym(img,alph,imgW,imgH,*_ii1,*_ii2,alph==NULL?NULL:*_cnt);

  winW = winW/2;

  II1 **cnt = alph==NULL ? NULL : *_cnt;

  gray minval;
  float maxstd;
  minValmaxStd(img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,&minval,&maxstd);

  //maxsd = 128.0;

  int y;
  for(y=imgH-1;y>=0;y--) {
    gray *imgy = img[y];
    int x;
    for(x=imgW-1;x>=0;x--)
      imgy[x] = enhSauvola_single(x,y,img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,prm,slp,maxstd);
  }

  return EXIT_SUCCESS;
}*/

int enhLocal_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float prm, float slp, int type) {

  if(*_ii1==NULL || *_ii2==NULL) {
    int err = 0;
    err += malloc_II1(imgW,imgH,_ii1,FALSE);
    err += malloc_II2(imgW,imgH,_ii2,FALSE);
    if(alph!=NULL)
      err += malloc_II1(imgW,imgH,_cnt,FALSE);
    if(err) {
      fprintf(stderr,"enhLocal_graym: error: unable to reserve memory\n");
      return EXIT_FAILURE;
    }
    _ii1[0][imgH-1][imgW-1]=0;
  }
  if(_ii1[0][imgH-1][imgW-1]==0)
    compII12_graym(img,alph,imgW,imgH,*_ii1,*_ii2,alph==NULL?NULL:*_cnt);

  winW = winW/2;

  II1 **cnt = alph==NULL ? NULL : *_cnt;

  gray minval = 0;
  float maxstd = 128.0;
  if ( type == ENH_SAUVOLA_SDMAX || type == ENH_WOLF )
    minValmaxStd(img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,&minval,&maxstd);

  int y;
  for(y=imgH-1;y>=0;y--) {
    gray *imgy = img[y];
    int x;
    for(x=imgW-1;x>=0;x--) {
      if ( type == ENH_SAUVOLA || type == ENH_SAUVOLA_SDMAX )
        imgy[x] = enhSauvola_single(x,y,img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,prm,slp,maxstd);
      else
        imgy[x] = enhWolf_single(x,y,img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,prm,slp,maxstd,minval);
    }
  }

  return EXIT_SUCCESS;
}


int enhSauvola_sample_prm_graym(gray** img, gray** alph, int imgW, int imgH, II1*** _ii1, II2*** _ii2, II1*** _cnt, int winW, float slp, double *_prm, int subsamp, float prmthr) {

  if(*_ii1==NULL || *_ii2==NULL) {
    int err = 0;
    err += malloc_II1(imgW,imgH,_ii1,FALSE);
    err += malloc_II2(imgW,imgH,_ii2,FALSE);
    if(alph!=NULL)
      err += malloc_II1(imgW,imgH,_cnt,FALSE);
    if(err) {
      fprintf(stderr,"enhSauvola_sample_prm_graym: error: unable to reserve memory\n");
      return EXIT_FAILURE;
    }
    _ii1[0][imgH-1][imgW-1]=0;
  }
  if(_ii1[0][imgH-1][imgW-1]==0)
    compII12_graym(img,alph,imgW,imgH,*_ii1,*_ii2,alph==NULL?NULL:*_cnt);

  winW = winW/2;

  II1 **cnt = alph==NULL ? NULL : *_cnt;

  static int Nprm = 10;
  static int bins = 32;
  static double histfact = 1.0/8.0;
  float prms[] = { 0.02, 0.04, 0.06, 0.08, 0.12, 0.16, 0.20, 0.24, 0.32, 0.48 };
  int hist[bins];
  float histvals[bins];

  //*_prm = 0.0;

  int n;
  for(n=0;n<Nprm;n++) {
    float prm = prms[n];

    int hcnt = 0;
    int y;
    for(y=bins-1;y>=0;y--)
      hist[y] = 0;

    for(y=imgH-1;y>=0;y-=subsamp) {
      int x;
      for(x=imgW-1;x>=0;x-=subsamp) {
        if(alph!=NULL && alph[y][x]==0)
          continue;
        gray v = enhSauvola_single(x,y,img,alph,imgW,imgH,*_ii1,*_ii2,cnt,winW,prm,slp,128.0);
        hist[(int)(v*histfact)]++;
        hcnt++;
      }
    }

    histvals[n] = hist[bins-2]/(double)hcnt;
    //fprintf(stderr,"histval[ %.2f ] -> %.6f\n",prm,histvals[n]);

    if( histvals[n] < prmthr ) {
      //fprintf(stderr,"stopping\n");
      //if(*_prm==0.0)
      *_prm = prm;
      return EXIT_SUCCESS;
    }
  }

  double vmax = 0.0;
  double vmin = 1.0;
  for(n=0;n<Nprm;n++) {
    if( vmax < histvals[n] )
      vmax = histvals[n];
    if( vmin > histvals[n] )
      vmin = histvals[n];
  }

  double thr = (1.0/Nprm)*(vmax-vmin)+vmin;

  for(n=0;n<Nprm-1;n++) {
    if( histvals[n] < thr ||
        histvals[n] == vmin ) {
      //fprintf(stderr,"stopping at %.2f\n",prms[n]);
      //if(*_prm==0.0)
      *_prm = prms[n];
      return EXIT_SUCCESS;
    }
  }

  return EXIT_FAILURE;
}

/*
int maskExtend_graym(gray **img, gray **alph, int imgW, int imgH, int winW, float prm, float seR) {
  gray **tmp;
  int err = clone_graym(img,imgW,imgH,&tmp);
  if(err) {
    fprintf(stderr,"maskExtend_graym: error: unable to reserve memory\n");
    return err;
  }

  II1** ii1 = NULL;
  II2** ii2 = NULL;
  err = enhSauvola_graym(tmp,NULL,imgW,imgH,&ii1,&ii2,NULL, winW, prm, 0);
  if(err!=EXIT_SUCCESS) {
    free(tmp);
    return err;
  }
  free(ii1);
  free(ii2);

  SE *se;
  //err = seCircle(seR,0,0,1,&se);
  err = seCircle(seR,TRUE,FALSE,&se,NULL);
  if(err) {
    free(tmp);
    return err;
  }
  err = morpherode_graym(tmp,imgW,imgH,se,tmp);
  if(err!=EXIT_SUCCESS) {
    free(tmp);
    return err;
  }
  free(se);

  int n;
  for(n=imgW*imgH-1;n>=0;n--)
    if(alph[0][n]==0 && tmp[0][n]!=0)
      alph[0][n] = 255;

  free(tmp);

  return EXIT_SUCCESS;
}
*/

/*
int rgbm2graym_winproj(pixel** img, int imgW, int imgH, II1*** _ii1, II2*** _ii2, gray** out, float* base, int D) {
  float min = base[0];
  float rang = base[1];
  int winW = base[D-1];
  pixel *imgv = img[0];
  gray *outv = out[0];
  base = base+3;
  D -= 3;
  winW /= 2;

  if(*_ii1==NULL || *_ii2==NULL) {
    int err = 0;
    err += malloc_II1(imgW,imgH,_ii1,0);
    err += malloc_II2(imgW,imgH,_ii2,0);
    if(err) {
      fprintf(stderr,"rgbm2graym_proj_musd: error: unable to reserve memory\n");
      return EXIT_FAILURE;
    }
    compII12_graym(img,NULL,imgW,imgH,*_ii1,*_ii2,NULL);
  }

  float gamma = 0;
  if( D==6 || D==21 ) {
    gamma = base[D-1];
    rang = 1/(rang-min);
    D --;
  }
  else
    rang = IMGIO_GRAYMAX/(rang-min);

  int x,y;
  float pv;
  switch(D) {
  case 5:
    for(y=imgH-1;y>=0;y--) {
      imgv = img[y];
      outv = out[y];
      int ymin = y-winW < 0 ? 0 : y-winW;
      int ymax = y+winW >= imgH-1 ? imgH-1 : y+winW;
      for(x=imgW-1;x>=0;x--) {
        pixel p = imgv[x];
        int xmin = x-winW < 0 ? 0 : x-winW;
        int xmax = x+winW >= imgW-1 ? imgW-1 : x+winW;
        float mu,sd;
        meanSd_II(*_ii1,*_ii2,NULL,xmin,ymin,xmax-xmin+1,ymax-ymin+1,&mu,&sd);
  //fprintf(file,"                    format 1: MIN MAX fR fG fB fM fS [GAM] WINW\n");
        pv =
          base[0]*p.r + base[1]*p.g + base[2]*p.b + base[3]*mu + base[4]*sd;
        if(gamma)
          outv[x] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(pv-min),gamma)+0.5 );
        else
          outv[x] = limit_gray( rang*(pv-min)+0.5 );
      }
    }
    break;

  case 20:
    for(y=imgH-1;y>=0;y--) {
      imgv = img[y];
      outv = out[y];
      int ymin = y-winW < 0 ? 0 : y-winW;
      int ymax = y+winW >= imgH-1 ? imgH-1 : y+winW;
      for(x=imgW-1;x>=0;x--) {
        pixel p = imgv[x];
        int xmin = x-winW < 0 ? 0 : x-winW;
        int xmax = x+winW >= imgW-1 ? imgW-1 : x+winW;
        float mu,sd;
        meanSd_II(*_ii1,*_ii2,NULL,xmin,ymin,xmax-xmin+1,ymax-ymin+1,&mu,&sd);
  //fprintf(file,"                    format 2: MIN MAX fR fG fB fRG fGB fRB fRR fGG fBB fM fS fMS fMR fMG fMB fSR fSG fSB fMM fSS [GAM] WINW\n");
        pv =
          base[0]*p.r     + base[1]*p.g     + base[2]*p.b +
          base[3]*p.r*p.g + base[4]*p.g*p.b + base[5]*p.r*p.b +
          base[6]*p.r*p.r + base[7]*p.g*p.g + base[8]*p.b*p.b +
          base[9]*mu      + base[10]*sd     + base[11]*mu*sd +
          base[12]*mu*p.r + base[13]*mu*p.g + base[14]*mu*p.b +
          base[15]*sd*p.r + base[16]*sd*p.g + base[17]*sd*p.b +
          base[18]*mu*mu  + base[19]*sd*sd;
        if(gamma)
          outv[x] = limit_gray( IMGIO_GRAYMAX*gamma_correct(rang*(pv-min),gamma)+0.5 );
        else
          outv[x] = limit_gray( rang*(pv-min)+0.5 );
      }
    }
    break;
  default:
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
*/
