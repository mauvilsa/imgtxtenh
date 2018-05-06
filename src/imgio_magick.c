/**
 * Functions for reading and writing images using MagickCore
 *
 * @version $Revision: 207 $$Date:: 2015-11-24 #$
 * @copyright Copyright (c) 2014 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "imgio_magick.h"

#define CM_PER_IN 2.54
#define IN_PER_CM 0.39370078740157480314

void free_Img( Img *img ) {
  DestroyImageInfo( img->info );
  DestroyImage( img->image );
  free(img);
}

Img* create_img( ImageInfo* image_info, Image* image ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  unsigned char is_gray = IsGrayImage( image, exception );
  unsigned char is_opaque = IsOpaqueImage( image, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  //IdentifyImage(image,stderr,MagickTrue);

  Img* img = (Img*)calloc( 1, sizeof(Img) );
  if( img == NULL )
    return NULL;

  /* @todo convert to true color sRGB if not gray and not true color sRGB
     @todo change orientation to 0 if different */

  //if( image->orientation != UndefinedOrientation )

  if( image->units == PixelsPerInchResolution ) {
    image->units = PixelsPerCentimeterResolution;
    image_info->units = PixelsPerCentimeterResolution;
    image->x_resolution *= IN_PER_CM;
    image->y_resolution *= IN_PER_CM;
  }

  img->width = image->columns;
  img->height = image->rows;
  img->res_x = image->units != UndefinedResolution ? image->x_resolution : 0;
  img->res_y = image->units != UndefinedResolution ? image->y_resolution : 0;
  img->is_gray = is_gray;
  img->is_opaque = is_opaque;
  img->info = image_info;
  img->image = image;

  return img;
}

int readimg_magick( char const* fname, Img** _img, FILE* logfile ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  ImageInfo *image_info = CloneImageInfo( NULL );
  CopyMagickString( image_info->filename, fname, MaxTextExtent );
  Image *image = ReadImage( image_info, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  if( image == NULL ) {
    DestroyImageInfo( image_info );
    fprintf( logfile, "%s: error: unable to read image: %s\n", "readimg_magick", fname );
    return FAILURE;
  }

  *_img = create_img( image_info, image );

  if( *_img == NULL ) {
    DestroyImageInfo( image_info );
    DestroyImage( image );
    fprintf( logfile, "%s: error: problems creating Img structure\n", "readimg_magick" );
    return FAILURE;
  }

  return SUCCESS;
}

int scanimg_magick( FILE* file, Img** _img, FILE* logfile ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  ImageInfo *image_info = CloneImageInfo( (ImageInfo*)NULL );
  image_info->file = file;
  Image *image = ReadImage( image_info, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  if( image == (Image*)NULL ) {
    DestroyImageInfo( image_info );
    fprintf( logfile, "%s: error: unable to read image\n", "scanimg_magick" );
    return FAILURE;
  }

  *_img = create_img( image_info, image );

  if( *_img == NULL ) {
    DestroyImageInfo( image_info );
    DestroyImage( image );
    fprintf( logfile, "%s: error: problems creating Img structure\n", "scanimg_magick" );
    return FAILURE;
  }

  return SUCCESS;
}

int writeimg_magick( char const* fname, Img* img, FILE* logfile ) {
  CopyMagickString( img->image->filename, fname, MaxTextExtent );
  if( WriteImage( img->info, img->image ) == MagickFalse ) {
    fprintf( logfile, "%s: error: unable to write image: %s\n", "writeimg_magick", fname );
    return FAILURE;
  }

  return SUCCESS;
}

int printimg_magick( FILE* file, char const* format, Img* img, FILE* logfile ) {
  char out[16];
  sprintf(out,"%s:out",format);

  img->info->file = file;
  CopyMagickString( img->image->filename, out, MaxTextExtent );

  if( WriteImage( img->info, img->image ) == MagickFalse ) {
    fprintf( logfile, "%s: error: unable to write image\n", "printimg_magick" );
    return FAILURE;
  }

  return SUCCESS;
}

int getalpha_magick_graym( Img* img, gray* gimg ) {
  if( img->is_opaque )
    return FALSE;

  ExceptionInfo *exception = AcquireExceptionInfo();
  const PixelPacket *pixs =
    GetVirtualPixels( img->image, 0, 0, img->width, img->height, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  int n;
  for( n=img->width*img->height-1; n>=0; n-- )
#if MAGICKCORE_QUANTUM_DEPTH == 16
    gimg[n] = GetPixelAlpha(pixs+n) >> 8;
#elif MAGICKCORE_QUANTUM_DEPTH == 8
    gimg[n] = GetPixelAlpha(pixs+n);
#endif

  return SUCCESS;
}

int getpixels_magick_graym( Img* img, gray* gimg ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  const PixelPacket *pixs =
    GetVirtualPixels( img->image, 0, 0, img->width, img->height, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  int n;
  for( n=img->width*img->height-1; n>=0; n-- )
#if MAGICKCORE_QUANTUM_DEPTH == 16
    gimg[n] = GetPixelGray(pixs+n) >> 8;
#elif MAGICKCORE_QUANTUM_DEPTH == 8
    gimg[n] = GetPixelGray(pixs+n);
#endif

  return SUCCESS;
}

int getpixels_magick_pixelm( Img* img, pixel* cimg ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  const PixelPacket *pixs =
    GetVirtualPixels( img->image, 0, 0, img->width, img->height, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  int n;
  for( n=img->width*img->height-1; n>=0; n-- ) {
#if MAGICKCORE_QUANTUM_DEPTH == 16
    cimg[n].r = GetPixelRed(pixs+n) >> 8;
    cimg[n].g = GetPixelGreen(pixs+n) >> 8;
    cimg[n].b = GetPixelBlue(pixs+n) >> 8;
#elif MAGICKCORE_QUANTUM_DEPTH == 8
    cimg[n].r = GetPixelRed(pixs+n);
    cimg[n].g = GetPixelGreen(pixs+n);
    cimg[n].b = GetPixelBlue(pixs+n);
#endif
  }

  return SUCCESS;
}

int setpixels_magick_graym( Img* img, gray* gimg ) {
  ExceptionInfo *exception = AcquireExceptionInfo();

  Image *pimage = img->image;

  img->image = ConstituteImage( img->width, img->height, "I", CharPixel, gimg, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  if( pimage != NULL ) {
    CloneImageProperties( img->image, pimage );
    CloneImageProfiles( img->image, pimage );
    DestroyImage( pimage );
  }

  return SUCCESS;
}

int setpixels_magick_grayalphm( Img* img, gray* gimg, gray* alph ) {
  ExceptionInfo *exception = AcquireExceptionInfo();

  Image *pimage = img->image;

  int n, WH = img->width*img->height;

  gray* gaimg = NULL;
  if( malloc_grayv( WH+WH, &gaimg, FALSE ) )
    return FAILURE;

  for( n=0; n<WH; n++ ) {
    gaimg[n+n] = gimg[n];
    gaimg[n+n+1] = alph[n];
  }

  img->image = ConstituteImage( img->width, img->height, "IA", CharPixel, gaimg, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );
  free( gaimg );

  if( pimage != NULL ) {
    CloneImageProperties( img->image, pimage );
    CloneImageProfiles( img->image, pimage );
    DestroyImage( pimage );
  }

  return SUCCESS;
}

int setpixels_magick_pixelm( Img* img, pixel* cimg ) {
  ExceptionInfo *exception = AcquireExceptionInfo();

  Image *pimage = img->image;

  img->image = ConstituteImage( img->width, img->height, "RGB", CharPixel, cimg, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  if( pimage != NULL ) {
    CloneImageProperties( img->image, pimage );
    CloneImageProfiles( img->image, pimage );
    DestroyImage( pimage );
  }

  return SUCCESS;
}

int togray_magick( Img* img ) {
  gray *gimg = NULL;
  if( malloc_grayv( img->width*img->height, &gimg, FALSE ) )
    return FAILURE;

  ExceptionInfo *exception = AcquireExceptionInfo();
  const PixelPacket *pixs =
    GetVirtualPixels( img->image, 0, 0, img->width, img->height, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  int n;
  for( n=img->width*img->height-1; n>=0; n-- )
#if MAGICKCORE_QUANTUM_DEPTH == 16
    gimg[n] = (gray)( 0.5 +
              0.299*(GetPixelRed(pixs+n)>>8) +
              0.587*(GetPixelGreen(pixs+n)>>8) +
              0.114*(GetPixelBlue(pixs+n)>>8) );
#elif MAGICKCORE_QUANTUM_DEPTH == 8
    gimg[n] = (gray)( 0.5 +
              0.299*(GetPixelRed(pixs+n)) +
              0.587*(GetPixelGreen(pixs+n)) +
              0.114*(GetPixelBlue(pixs+n)) );
#endif

  int err = setpixels_magick_graym( img, gimg );

  SetImageProperty(img->image,"colorspace","Gray");
  img->info->colorspace = GRAYColorspace;
  img->is_gray = IsGrayImage( img->image, exception );
  //printf( "image %s in grayscale\n", img->is_gray ? "is" : "is NOT" );

  free(gimg);

  return err;
}

/*int togray_magick( Img* img ) {
  if( img->is_gray )
    return SUCCESS;

  TransformImageColorspace( img->image, GRAYColorspace );

  ExceptionInfo *exception = AcquireExceptionInfo();
  img->is_gray = IsGrayImage( img->image, exception );

  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  return img->is_gray ? SUCCESS : FAILURE;
}*/

void set_density_magick( Img* img, double density ) {
  img->res_x = density;
  img->res_y = density;
  img->image->x_resolution = density;
  img->image->y_resolution = density;
  img->image->units = PixelsPerCentimeterResolution;
  img->info->units = PixelsPerCentimeterResolution;
}

int add_border_magick( Img* img, size_t size, PixelPacket color ) {
  ExceptionInfo *exception = AcquireExceptionInfo();
  const RectangleInfo bord = { size, size, 0, 0 };

  img->image->border_color = color;

  Image *bimg = BorderImage( img->image, &bord, exception );
  if( exception->severity != UndefinedException )
    CatchException( exception );
  DestroyExceptionInfo( exception );

  DestroyImage( img->image );
  img->image = bimg;
  img->width = bimg->columns;
  img->height = bimg->rows;

  return SUCCESS;
}
