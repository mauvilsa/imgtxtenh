/**
 * Connected component functions for images
 *
 * @version $Revision: 172 $$Date:: 2015-03-09 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "objs.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

static inline int ROOT_NODE(int *mfset,int neighbor1) {
  if(mfset[neighbor1]==neighbor1)
    return neighbor1;
  else
    mfset[neighbor1] = ROOT_NODE(mfset,mfset[neighbor1]);
  return mfset[neighbor1];
}

static inline void MERGE(int *mfset,int neighbor1,int neighbor2) {
  int n1 = ROOT_NODE(mfset,neighbor1);
  int n2 = ROOT_NODE(mfset,neighbor2);
  if(n1<n2)
    mfset[n2] = n1;
  else
    mfset[n1] = n2;
}

int objlabel4(gray** img,int imgW,int imgH,int** out,int* _obj)
{
  int i,j,n=0,obj=0;
  int **tmp,*T1,*T2;

  if( (T1=(int*)malloc(imgW*imgH*sizeof(int)))==NULL ||
      bcalloc_intm(imgW,imgH,1,&tmp) )
    return EXIT_FAILURE;

  T2 = T1+(imgW*imgH/2);

  for(i=0;i<imgH;i++)
    for(j=0;j<imgW;j++)
      if(img[i][j]) {
        int neigh1=tmp[i-1][j];
        int neigh2=tmp[i][j-1];
        if(neigh1 && neigh2 && T1[neigh1]!=T1[neigh2]) {
          MERGE(T1,neigh1,neigh2);
          tmp[i][j]=T1[neigh1];
        }
        else if(neigh1)
          tmp[i][j] = T1[neigh1];
        else if(neigh2)
          tmp[i][j] = T1[neigh2];
        else {
          n++;
          tmp[i][j]=n;
          T1[n]=n;
        }
      }
  T2[0]=0;
  for(i=1;i<=n;i++) {
    if(T1[i]!=i)
      T2[i] = T2[ROOT_NODE(T1,i)];
    else {
      obj++;
      T2[i]=obj;
    }
  }
  for(i=imgH;i--;)
    for(j=imgW;j--;)
      out[i][j]=T2[tmp[i][j]];

  *_obj=obj;

  free(T1);
  bfree(tmp,1);
  return EXIT_SUCCESS;
}

int objlabel8(gray** img,int imgW,int imgH,int** out,int* _obj)
{
  int i,j,n=0,obj=0;
  int **tmp,*T1,*T2;

  if( (T1=(int*)malloc(imgW*imgH*sizeof(int)/2))==NULL ||
      bcalloc_intm(imgW,imgH,1,&tmp) )
    return EXIT_FAILURE;

  T2 = T1+(imgW*imgH/4);

  for(i=0;i<imgH;i++)
    for(j=0;j<imgW;j++)
      if(img[i][j]) {
        int neigh[4];
        neigh[0] = tmp[i][j-1];
        neigh[1] = tmp[i-1][j-1];
        neigh[2] = tmp[i-1][j];
        neigh[3] = tmp[i-1][j+1];
        if(!(neigh[0] || neigh[1] || neigh[2] || neigh[3])) {
          n++;
          tmp[i][j]=n;
          T1[n]=n;
        }
        else {
          int nn = 0;
          while(!neigh[nn])
            nn++;
          int mm;
          for(mm=nn+1;mm<4;mm++)
            if(neigh[mm] && T1[neigh[nn]]!=T1[neigh[mm]])
              MERGE(T1,neigh[nn],neigh[mm]);
          tmp[i][j]=T1[neigh[nn]];
        }
      }
  T2[0]=0;
  for(i=1;i<=n;i++) {
    if(T1[i]!=i)
      T2[i] = T2[ROOT_NODE(T1,i)];
    else {
      obj++;
      T2[i]=obj;
    }
  }
  for(i=imgH;i--;)
    for(j=imgW;j--;)
      out[i][j]=T2[tmp[i][j]];

  *_obj=obj;

  free(T1);
  bfree(tmp,1);
  return EXIT_SUCCESS;
}

int objinfo(int** img,int imgW,int imgH,int obj,int** bbox,int* area,float** cent)
{
  int n,x,y,*minX=NULL,*maxX=NULL,*minY=NULL,*maxY=NULL;
  float *cX=NULL,*cY=NULL;

  if(bbox!=NULL) {
    minX = bbox[0];
    maxX = bbox[1];
    minY = bbox[2];
    maxY = bbox[3];
    for(n=obj;n>=0;n--) {
      minX[n] = -1;
      maxX[n] = -1;
      minY[n] = -1;
      maxY[n] = -1;
    }
  }
  if(area!=NULL)
    memset(area,0,(obj+1)*sizeof(int));
  if(cent!=NULL) {
    if(area==NULL)
      return EXIT_FAILURE;
    cX = cent[0];
    cY = cent[1];
    memset(cX,0,(obj+1)*sizeof(float));
    memset(cY,0,(obj+1)*sizeof(float));
  }

  if(bbox!=NULL || area!=NULL)
    for(y=imgH-1;y>=0;y--)
      for(x=imgW-1;x>=0;x--) {
        n = img[y][x];
        if(bbox!=NULL) {
          if(minX[n]==-1 || minX[n]>x) minX[n] = x;
          if(minY[n]==-1 || minY[n]>y) minY[n] = y;
          if(maxX[n]<x) maxX[n] = x;
          if(maxY[n]<y) maxY[n] = y;
        }
        if(area!=NULL)
          area[n]++;
        if(cent!=NULL) {
          cX[n] += x;
          cY[n] += y;
        }
      }

  if(cent!=NULL)
    for(n=obj;n>=0;n--)
      if(area[n]!=0) {
        cX[n] /= area[n];
        cY[n] /= area[n];
      }

  return EXIT_SUCCESS;
}



int objmeanWH(int** img,int imgW,int imgH,int obj,int** meanWH) {
  int n, x, y, *meanW=NULL, *meanH=NULL;

  if( meanWH != NULL ) {
    meanW = meanWH[0];
    meanH = meanWH[1];
    memset(meanW,0,(obj+1)*sizeof(int));
    memset(meanH,0,(obj+1)*sizeof(int));
  }

  int *mn = NULL;
  if( malloc_I1v(3*(obj+1),&mn,FALSE) )
    return FAILURE;

  int *mx = mn+obj+1;
  int *cnt = mx+obj+1;

  memset(cnt,0,(obj+1)*sizeof(int));
  for( x=imgW-1; x>=0; x-- ) {
    for( n=obj; n>=0; n-- ) {
      mn[n] = -1;
      mx[n] = -1;
    }
    for( y=imgH-1; y>=0; y-- ) {
      n = img[y][x];
      if( mn[n] == -1 || mn[n] > y )
        mn[n] = y;
      if( mx[n] < y )
        mx[n] = y;
    }
    for( n=obj; n>=0; n-- )
      if( mn[n] != -1 && mx[n] != -1 ) {
        meanH[n] += mx[n]-mn[n];
        cnt[n] ++;
      }
  }

  for( n=obj; n>=0; n-- )
    meanH[n] = (int)(0.5+(double)meanH[n]/cnt[n]);

  memset(cnt,0,(obj+1)*sizeof(int));
  for( y=imgH-1; y>=0; y-- ) {
    for( n=obj; n>=0; n-- ) {
      mn[n] = -1;
      mx[n] = -1;
    }
    for( x=imgW-1; x>=0; x-- ) {
      n = img[y][x];
      if( mn[n] == -1 || mn[n] > x )
        mn[n] = x;
      if( mx[n] < x )
        mx[n] = x;
    }
    for( n=obj; n>=0; n-- )
      if( mn[n] != -1 && mx[n] != -1 ) {
        meanW[n] += mx[n]-mn[n];
        cnt[n] ++;
      }
  }

  for( n=obj; n>=0; n-- )
    meanW[n] = (int)(0.5+(double)meanW[n]/cnt[n]);

  free(mn);

  return EXIT_SUCCESS;
}



/*int objmodeWH(int** img,int imgW,int imgH,int obj,int** modeWH) {
  int n, x, y, *modeW=NULL, *modeH=NULL;

  fprintf(stderr,"error: this function is not working\n");
  if( stderr!=NULL )
    EXIT_FAILURE;

  modeW = modeWH[0];
  modeH = modeWH[1];
  memset(modeW,0,(obj+1)*sizeof(int));
  memset(modeH,0,(obj+1)*sizeof(int));

  int sz = imgW > imgH ? imgW : imgH;
  int **mo = NULL;
  int *mn = NULL;
  if( malloc_I1m(sz,obj+1,&mo,FALSE) ||
      malloc_I1v(2*(obj+1),&mn,FALSE) )
    return FAILURE;
  int *mx = mn+obj+1;

  for( x=imgW-1; x>=0; x-- ) {
    memset(mo[0],0,sz*(obj+1)*sizeof(int));
    for( n=obj; n>=0; n-- ) {
      mn[n] = -1;
      mx[n] = -1;
    }
    for( y=imgH-1; y>=0; y-- ) {
      n = img[y][x];
      if( mn[n] == -1 || mn[n] > y )
        mn[n] = y;
      if( mx[n] < y )
        mx[n] = y;
    }
    for( n=obj; n>=0; n-- )
      if( mn[n] != -1 && mx[n] != -1 )
        mo[n][mx[n]-mn[n]] ++;
  }

  for( n=obj; n>=0; n-- ) {
    int idx = imgH-1;
    int max = mo[n][idx];
    for( y=imgH-2; y>=0; y-- )
      if( max < mo[n][y] ) {
        idx = y;
        max = mo[n][idx];
      }

    modeH[n] = idx;
  }

  for( y=imgH-1; y>=0; y-- ) {
    memset(mo[0],0,sz*(obj+1)*sizeof(int));
    for( n=obj; n>=0; n-- ) {
      mn[n] = -1;
      mx[n] = -1;
    }
    for( x=imgW-1; x>=0; x-- ) {
      n = img[y][x];
      if( mn[n] == -1 || mn[n] > x )
        mn[n] = x;
      if( mx[n] < x )
        mx[n] = x;
    }
    for( n=obj; n>=0; n-- )
      if( mn[n] != -1 && mx[n] != -1 )
        mo[n][mx[n]-mn[n]] ++;
  }

  for( n=obj; n>=0; n-- ) {
    int idx = imgW-1;
    int max = mo[n][idx];
    for( x=imgW-2; x>=0; x-- )
      if( max < mo[n][x] ) {
        idx = x;
        max = mo[n][idx];
      }

    modeW[n] = idx;
  }

  free(mo);
  free(mn);

  return EXIT_SUCCESS;
}*/





/*void objarea(int** img,int imgW,int imgH,int obj,int* _area)
{
  int i,j,a=0;

  for(i=imgH;i--;)
    for(j=imgW;j--;)
      if(img[i][j]==obj)
        a++;

  *_area=a;
}*/

/*void objperim(int** img,int imgW,int imgH,int obj,int* _perim)
{
  int i,j,p=0;

  for(i=0;i<imgH;i++) {
    if(img[i][0]==obj)
      p++;
    if(img[i][imgW-1]==obj)
      p++;
  }
  for(j=1;j<imgW-1;j++) {
    if(img[0][j]==obj)
      p++;
    if(img[imgH-1][j]==obj)
      p++;
  }
  for(i=1;i<imgH-1;i++)
    for(j=1;j<imgW-1;j++)
      if(img[i][j]==obj)
        if( img[i-1][j-1]!=obj || img[i-1][j]!=obj || img[i-1][j+1]!=obj || img[i][j-1]!=obj || img[i][j+1]!=obj || img[i+1][j-1]!=obj || img[i+1][j]!=obj || img[i+1][j+1]!=obj)
          p++;

  *_perim=p;
}*/

/*void objcompactness(int** img,int imgW,int imgH,int obj,double* _comp)
{
  int a,p;

  objarea(img,imgW,imgH,obj,&a);
  objperim(img,imgW,imgH,obj,&p);

  *_comp=(double)a/(p*p);
}*/

/*int objbbox(int** img,int imgW,int imgH,int obj,int* _minX,int* _maxX,int* _minY,int* _maxY)
{
  int i,j,minX=0,minY=0,maxX=imgW-1,maxY=imgH-1;

  for(i=0;i<imgH;i++)
    for(j=imgW;j--;)
      if(img[i][j]==obj) {
        minY=i;
        i=imgH;
        break;
      }
  for(i=imgH;i--;)
    for(j=imgW;j--;)
      if(img[i][j]==obj) {
        maxY=i;
        i=0;
        break;
      }
  for(j=0;j<imgW;j++)
    for(i=imgH;i--;)
      if(img[i][j]==obj) {
        minX=j;
        j=imgW;
        break;
      }
  for(j=imgW;j--;)
    for(i=imgH;i--;)
      if(img[i][j]==obj) {
        maxX=j;
        j=0;
        break;
      }

  if(i==-1 && j==-1)
    return EXIT_FAILURE;

  *_minY=minY;
  *_maxY=maxY;
  *_minX=minX;
  *_maxX=maxX;

  return EXIT_SUCCESS;
}*/

/*void objcentroid(int** img,int imgW,int imgH,int obj,double* _cX,double* _cY)
{
  int i,j,a=0,cX=0,cY=0;

  for(i=imgH;i--;)
    for(j=imgW;j--;)
      if(img[i][j]==obj) {
        cY+=i;
        cX+=j;
        a++;
      }

  *_cY=(double)cY/a;
  *_cX=(double)cX/a;
}*/

/*void objorient(int** img,int imgW,int imgH,int obj,double* _orient)
{
  int i,j;
  double cX,cY,mu11=0.0,mu20=0.0,mu02=0.0;

  objcentroid(img,imgW,imgH,obj,&cX,&cY);

  for(i=imgH;i--;)
    for(j=imgW;j--;)
      if(img[i][j]==obj) {
        mu11+=(i-cY)*(j-cX);
        mu20+=(j-cX)*(j-cX);
        mu02+=(i-cY)*(i-cY);
      }

  *_orient=0.5*atan(2*mu11/(mu20-mu02));
}*/
