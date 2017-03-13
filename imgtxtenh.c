/**
 * Tool for enhancing noisy scanned text images
 *
 * @version $Revision: 242 $$Date:: 2017-03-13 #$
 * @copyright Copyright (c) 2012-present, Mauricio Villegas <mauvilsa@upv.es>
 * @link https://github.com/mauvilsa/imgtxtenh
 * @license MIT License
 */

/*** Includes *****************************************************************/
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "imgio_magick.h"
#include "imgconv.h"
#include "intimg.h"
#include "htrprep.h"
#include "objs.h"
#include "log.h"

/*** Definitions **************************************************************/
static char tool[] = "imgtxtenh";
static char revnum[] = "$Revision: 242 $";
static char revdate[] = "$Date: 2017-03-13 17:09:15 +0100 (Mon, 13 Mar 2017) $";

//#define __NO_PIX_UNITS__

char *ifn = NULL;
char *ofn = "png:-";
//char *gb_units = "mm";
char *gb_units = "pixels";

//double gb_winW = 10;
double gb_winW = 80;
double gb_prm = 0.2;
double gb_slp = 0.5;
int gb_enhtype = ENH_SAUVOLA;

int gb_autosubsamp = 2;
double gb_prmthr = 0.004;

int gb_regstretch = FALSE;
double gb_satu = -1;
//double gb_seR = 0.2;
//double gb_seR = 0;

//double gb_small = 0.16;
//double gb_rlsa[4] = { 0.4, 0.4, 0.4, 0.4 }; // "-|/\"
double gb_small = 6;
double gb_rlsa[4] = { 2.4, 2.4, 2.4, 2.4 }; // "-|/\"

char gb_autoprm = FALSE;
//char gb_extmask = FALSE;
char gb_alph = TRUE;
char gb_savealph = FALSE;
char gb_negate = FALSE;
char gb_procimgs = FALSE;
double gb_density = FALSE;

FILE *logfile = NULL;
int verbosity = 1;

/*** Functions ****************************************************************/
void print_usage( FILE *file ) {
  char vsatu[20];
  sprintf(vsatu,"%g%%",100*gb_satu);

  fprintf( file, "Usage: %s [options] <in_img> [out_img (def.=%s)]\n", tool, ofn );
  fprintf( file, "\n" );
  fprintf( file, " -u (mm|pixels)  Units for ALL distance parameters (def.=%s)\n", gb_units );
  fprintf( file, " -d density      Specify the image density in pixels per cm (def.=%s)\n", strbool(gb_density) );
  fprintf( file, " -t type:string  Local enhancement type: sauvola, sauvolaSdMax, wilson (def.=sauvola)\n" );
  fprintf( file, " -w width:float  Window width for local enhancement (def.=%g%s)\n", gb_winW, gb_units );
  fprintf( file, " -k mfct:float   Local enhancement mean threshold factor (def.=%g)\n", gb_prm );
  //fprintf( file, " -K subs,thr     Automatically select mfct, use -K - for default %d,%g (def.=%s)\n", gb_autosubsamp, gb_prmthr, strbool(gb_autoprm) );
  fprintf( file, " -s sfct:float   Local enhancement threshold slope factor (def.=%g)\n", gb_slp );

  fprintf( file, " -S [r]sat:float Contrast stretch [optionally per region] saturating sat%% of pixels (def.=%s)\n", (gb_satu>=0?vsatu:"false") );
  fprintf( file, " -A              Use input image alpha channel as processing mask (def.=%s)\n", strbool(gb_alph) );
  fprintf( file, " -M mfile        Read separate processing mask image (def.=false)\n" );
  //fprintf( file, " -e seR:float    Processing mask extended by radius seR (def.=%g%s)\n", gb_seR, gb_units );
  fprintf( file, " -P pfile        Post-processing mask image (def.=false)\n" );
  fprintf( file, " -N              Negate mask images (def.=%s)\n", strbool(gb_negate) );
  fprintf( file, " -a              Save processing mask as alpha channel in output image (def.=%s)\n", strbool(gb_savealph) );

  fprintf( file, " -r small:float  Area limit for small component removal (def.=%g%s^2)\n", gb_small, gb_units );
  fprintf( file, " -R lengths      Protect faint strokes by 4-directional RLSA: '-', '|', '/' and '\\' (def.=%g,%g,%g,%g %s)\n", gb_rlsa[0], gb_rlsa[1], gb_rlsa[2], gb_rlsa[3], gb_units );
  fprintf( file, "                 lengths are 1 to 4 floats separated by commas\n" );

  fprintf( file, " -p              Save images %s_*.png of processing steps (def.=%s)\n", tool, strbool(gb_procimgs) );
  fprintf( file, " -l lfile        Logging to 'lfile' (def.=stderr)\n" );
  fprintf( file, " -V (-|+|level)  Set verbosity level (def.=%d)\n", verbosity );
  fprintf( file, " -h              Print this usage information and exit\n" );
  fprintf( file, " -v              Print version and exit\n" );
  fprintf( file, "\n");
  print_svn_rev( file );
}

/*** Program ******************************************************************/
int main( int argc, char *argv[] ) {
  logfile = stderr;
  char *mfn = NULL;
  char *pfn = NULL;
  int err = 0;

  char procimg[32], procnum = 0;

  MagickCoreGenesis( (char*)NULL, MagickFalse );

  /// Parse input arguments ///
  if( ! strncmp( "--", argc>1 ? argv[1] : "", 2 ) ) {
    print_svn_rev( logfile );
    return SUCCESS;
  }

  char *optstr = "u:d:t:w:k:K:s:S:AM:e:P:Nar:R:pl:V:hv";
  while( getopt(argc,argv,optstr) != -1 );
  int nopts = optind;
  optind = 1;

  int n;
  while( ( n = getopt(argc,argv,optstr) ) != -1 )
    switch( n ) {
    case 'u':
#ifdef __NO_PIX_UNITS__
      if( ! strcmp(optarg,"pixels") )
        die( "error: pixels units disabled to prevent malpractice" );
#endif
      if( ! strcmp(optarg,"mm") || ! strcmp(optarg,"pixels") )
        gb_units = optarg;
      else
        die( "error: units can only be mm or pixels" );
      break;
    case 'd':
      gb_density = atof(optarg);
      break;
    case 't':
      if ( ! strcmp(optarg,"sauvola") )
        gb_enhtype = ENH_SAUVOLA;
      else if ( ! strcmp(optarg,"sauvolaSdMax") )
        gb_enhtype = ENH_SAUVOLA_SDMAX;
      else if ( ! strcmp(optarg,"wolf") )
        gb_enhtype = ENH_WOLF;
      else {
        logger( 0, "error: incorrect input argument (-%c %s)", n, optarg );
        print_usage( logfile );
        return FAILURE;
      }
      break;
    case 'w':
      gb_winW = atoi(optarg);
      break;
    case 'k':
      gb_prm = atof(optarg);
      break;
    case 'K':
      gb_autoprm = TRUE;
      if( optarg[0] != '-' ) {
        gb_autosubsamp = atoi(optarg);
        if( strchr(optarg,',') != NULL )
          gb_prmthr = atof(strchr(optarg,',')+1);
      }
      break;
    case 's':
      gb_slp = atof(optarg);
      break;
    case 'S':
      if( optarg[0] != 'r' )
        gb_satu = atof(optarg)/100.0;
      else {
        gb_regstretch = TRUE;
        gb_satu = atof(optarg+1)/100.0;
      }
      break;
    case 'A':
      gb_alph = !gb_alph;
      break;
    case 'M':
      mfn = optarg;
      gb_alph = FALSE;
      break;
    //case 'e':
    //  gb_seR = atof(optarg);
    //  break;
    case 'P':
      pfn = optarg;
      gb_alph = FALSE;
      break;
    case 'N':
      gb_negate = !gb_negate;
      break;
    case 'a':
      gb_savealph = !gb_savealph;
      break;
    case 'r':
      gb_small = atof(optarg);
      break;
    case 'R':
      if( strchr(optarg,',') == NULL )
        gb_rlsa[3] = gb_rlsa[2] = gb_rlsa[1] = gb_rlsa[0] = atof(optarg);
      else {
        char *p = optarg;
        int m = 0;
        do {
          gb_rlsa[m++] = atof( p );
          p = strchr(p,',');
          p = p == NULL ? p : p+1 ;
        } while( p != NULL );
      }
      break;
    case 'p':
      gb_procimgs = !gb_procimgs;
      break;
    case 'l':
      if( ! strcmp( optarg, "-" ) ||
          ! strcmp( optarg, "stdout" ) ||
          ! strcmp( optarg, "/dev/stdout" ) )
        logfile = stdout;
      else if( (logfile = fopen(optarg,"ab")) == NULL )
        die( "error: unable to %s file %s", "open log", optarg );
      break;
    case 'V':
      if( optarg[0] == '+' )
        verbosity ++;
      else if( optarg[0] == '-' )
        verbosity --;
      else
        verbosity = atoi( optarg );
      verbosity = verbosity < 0 ? 0 : verbosity;
      break;
    default:
      logger( 0, "error: incorrect input argument (-%c)", n );
      err = FAILURE;
    case 'h':
      print_usage( logfile );
      return err;
    case 'v':
      print_svn_rev( logfile );
      return err;
    }

  if( argc - nopts > 2 ) {
    logger( 0, "error: expected at most two non-option arguments" );
    print_usage( logfile );
    return FAILURE;
  }

  if( argc - nopts > 0 )
    ifn = argv[optind++];
  if( argc - nopts > 1 )
    ofn = argv[optind++];

  if( ifn == NULL ) {
    logger( 0, "error: input image required" );
    print_usage( logfile );
    return FAILURE;
  }

  logger( 2, "input image: %s", ifn );
  logger( 2, "output image: %s", ofn );

  clock_t time_ini = clock();

  /// Read image ///
  Img* img = NULL;
  if( readimg_magick( ifn, &img, logfile ) )
    die( "error: unable to read image %s", ifn );

  if( gb_density ) {
    set_density_magick( img, gb_density );
    logger( 2, "read image of size %dx%d pixels and provided density %g ppc", img->width, img->height, gb_density );
  }
  else if( img->res_x == 0 )
    logger( 2, "read image of size %dx%d pixels and undefined density", img->width, img->height );
  else
    logger( 2, "read image of size %dx%d pixels and density %gx%g ppc", img->width, img->height, img->res_x, img->res_y );

  /// Convert physical units to pixels ///
  if( ! strcmp(gb_units,"mm") ) {
    if( img->res_x == 0 )
      die( "error: image does not specify density which is required for mm units" );
    if( img->res_x != img->res_y )
      die( "error: expected image density to be the same for vertical and horizontal" );
    double fact = img->res_x / 10.0;
    gb_winW *= fact;
    //gb_seR *= fact;
    gb_small *= fact*fact;
    for( n=0; n<4; n++ )
      gb_rlsa[n] *= fact;
  }
  //else
  //  logger( 0, "warning: it is discouraged to provide parameters in pixels" );

  /// Read processing mask ///
  gray **msk = NULL;
  if( ! gb_alph && mfn != NULL ) {
    Img *imsk = NULL;
    if( readimg_magick( mfn, &imsk, logfile ) )
      die( "error: unable to read image %s", mfn );
    if( img->width != imsk->width || img->height != imsk->height )
      die( "error: mask must have the same size as the image: %s", mfn );
    if( ! imsk->is_gray )
      if( togray_magick(imsk) )
        die( "error: problems converting to grayscale: %s", mfn );
    if( gb_negate )
      if( ! NegateImage( imsk->image, MagickTrue ) )
        die( "error: problems negating image: %s", mfn );
    //BilevelImage( imsk->image, 1 );
    if( malloc_graym( img->width, img->height, &msk, FALSE ) )
      die( "error: unable to reserve memory" );
    if( getpixels_magick_graym( imsk, msk[0] ) )
      die( "error: unable to get pixels" );
    if( gb_procimgs ) {
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "procmask" );
      if( writeimg_magick( procimg, imsk, logfile ) )
        die( "error: unable to write image %s", procimg );
    }
    free_Img( imsk );
    logger( 3, "read processing mask: %s", mfn );
  }

  else if( ! img->is_opaque ) {
    if( malloc_graym( img->width, img->height, &msk, FALSE ) )
      die( "error: unable to reserve memory" );
    if( getalpha_magick_graym( img, msk[0] ) )
      die( "error: unable to separate alpha channel" );
    //if( gb_procimgs ) {
    //  if( setpixels_magick_graym( img, msk[0] ) )
    //    die( "error: unable to set pixels" );
    //  sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "procmask" );
    //  if( writeimg_magick( procimg, img, logfile ) )
    //    die( "error: unable to write image %s", procimg );
    //}
  }

  /// Read post-processing mask ///
  gray **pmsk = NULL;
  if( pfn != NULL ) {
    Img *imsk = NULL;
    if( readimg_magick( pfn, &imsk, logfile ) )
      die( "error: unable to read image %s", pfn );
    if( img->width != imsk->width || img->height != imsk->height )
      die( "error: mask must have the same size as the image: %s", pfn );
    if( ! imsk->is_gray )
      if( togray_magick(imsk) )
        die( "error: problems converting to grayscale: %s", pfn );
    if( gb_negate )
      if( ! NegateImage( imsk->image, MagickTrue ) )
        die( "error: problems negating image: %s", pfn );
    //BilevelImage( imsk->image, 1 );
    if( malloc_graym( img->width, img->height, &pmsk, FALSE ) )
      die( "error: unable to reserve memory" );
    if( getpixels_magick_graym( imsk, pmsk[0] ) )
      die( "error: unable to get pixels" );
    if( gb_procimgs ) {
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "postmask" );
      if( writeimg_magick( procimg, imsk, logfile ) )
        die( "error: unable to write image %s", procimg );
    }
    free_Img( imsk );
    logger( 3, "read post-processing mask: %s", mfn );
  }

  double time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
  logger( 4, "time: reading images (ms): %g", time_end );

  /// Convert image to grayscale if different colorspace ///
  if( ! img->is_gray ) {
    //DestroyImageProperties(img->image);
    //SetImageProperty(img->image,"colorspace","RGB");
    //img->info->colorspace = RGBColorspace;

    //writeimg_magick( "tmp_gamma.png", img, logfile );

    //char *p = NULL;
    //ResetImagePropertyIterator( img->image );
    //while( ( p = GetNextImageProperty(img->image) ) != NULL )
    //  printf("prop: %s => %s\n",p,GetImageProperty(img->image,p));

    //ResetImageProfileIterator( img->image );
    //while( ( p = GetNextImageProfile(img->image) ) != NULL )
    //  printf("prof: %s\n",p);

    if( togray_magick(img) )
      die( "error: problems converting to grayscale" );
    logger( 3, "converted image to grayscale" );

    ProfileImage( img->image, "*", NULL, 0, MagickFalse );

    if( gb_procimgs ) {
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "gray" );
      if( writeimg_magick( procimg, img, logfile ) )
        die( "error: unable to write image %s", procimg );
    }
  }


  gray **gimg = NULL;
  if( malloc_graym( img->width, img->height, &gimg, FALSE ) )
    die( "error: unable to reserve memory" );
  if( getpixels_magick_graym( img, gimg[0] ) )
    die( "error: unable to get pixels" );

//FILE *f = fopen("in_img_new.txt","w");
//for( n=img->width*img->height-1; n>=0; n-- )
//  fprintf(f,"%d\n",gimg[0][n]);
//fclose(f);

  /// Contrast stretch ///
  if( gb_satu >= 0 ) {
    time_ini = clock();

    if( gb_regstretch && msk == NULL )
      die( "error: region-based contrast stretch requires a mask" );

    logger( 2, "stretching contrast saturating %g%%", gb_satu );

    if( gb_regstretch )
      regstretch_graym( gimg, msk, img->width, img->height, gb_satu/200 );
    else
      stretch_graym( gimg, msk, img->width, img->height, gb_satu/200 );

    if( gb_procimgs ) {
      if( setpixels_magick_graym( img, gimg[0] ) )
        die( "error: unable to set pixels" );
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "stretch" );
      if( writeimg_magick( procimg, img, logfile ) )
        die( "error: unable to write image %s", procimg );
    }

    time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
    logger( 4, "time: contrast stretch (ms): %g", time_end );
  }

  /// Extend processing mask ///
  /*gray **tmsk = msk;
  if( msk != NULL && gb_seR >= 0.5 ) {
    time_ini = clock();

    if( gb_procimgs ) {
      if( setpixels_magick_graym( img, msk[0] ) )
        die( "error: unable to set pixels" );
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "mask" );
      if( writeimg_magick( procimg, img, logfile ) )
        die( "error: unable to write image %s", procimg );
    }

    if( clone_graym( tmsk, img->width, img->height, &msk ) )
      die( "error: unable to reserve memory" );
    logger( 2, "extending processing mask with radius %.1g pixels", gb_seR );
    if( gb_autoprm ) {
      II1** ii1 = NULL;
      II2** ii2 = NULL;
      II1** cnt = NULL;
      if( enhSauvola_sample_prm_graym( gimg, msk, img->width, img->height, &ii1, &ii2, &cnt, gb_winW, gb_slp, &gb_prm, gb_autosubsamp, gb_prmthr ) )
        die( "error: unable to select the Sauvola mean threshold automatically" );
      free(ii1);
      free(ii2);
      free(cnt);
    }
    maskExtend_graym( gimg, msk, img->width, img->height, gb_winW, gb_prm, gb_seR );

    if( gb_procimgs ) {
      if( setpixels_magick_graym( img, msk[0] ) )
        die( "error: unable to set pixels" );
      sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "extmask" );
      if( writeimg_magick( procimg, img, logfile ) )
        die( "error: unable to write image %s", procimg );
    }

    time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
    logger( 4, "time: extending processing mask (ms): %g", time_end );
  }*/

  /// Enhance image ///
  time_ini = clock();

  II1** ii1 = NULL;
  II2** ii2 = NULL;
  II1** cnt = NULL;

  if( gb_autoprm ) {
    logger( 2, "automatic selection of mfct: %d, %g", gb_autosubsamp, gb_prmthr );
    if( enhSauvola_sample_prm_graym( gimg, msk, img->width, img->height, &ii1, &ii2, &cnt, gb_winW, gb_slp, &gb_prm, gb_autosubsamp, gb_prmthr ) )
      die( "error: unable to select the Sauvola mean threshold automatically" );
  }

  logger( 1, "enhancing by Sauvola: width=%.0fpixels, mfct=%g%s, sfct=%g", gb_winW, gb_prm, gb_autoprm?"(auto)":"", gb_slp );
  //enhSauvola_graym( gimg, msk, img->width, img->height, &ii1, &ii2, &cnt, gb_winW, gb_prm, gb_slp );
  enhLocal_graym( gimg, msk, img->width, img->height, &ii1, &ii2, &cnt, gb_winW, gb_prm, gb_slp, gb_enhtype );

  time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
  logger( 4, "time: enhancement (ms): %g", time_end );

  /// Set masked zones to white ///
  /*if( msk != NULL && tmsk != msk ) {
    free(msk);
    msk = tmsk;
  }*/
  if( pmsk == NULL )
    pmsk = msk;
  if( pmsk != NULL ) {
    for( n=img->width*img->height-1; n>=0; n-- )
      if( pmsk[0][n] == 0 )
        gimg[0][n] = 255;
  }

  /// Small component removal ///
  int minarea = (int)(gb_small+0.5);
  if( gb_small > 0 && minarea <= 0 )
    logger( 2, "skipping small component removal due to low pixel density" );
  else if( minarea > 0 ) {
    time_ini = clock();

    gray **bmap = NULL;
    int **objs = NULL;
    int *area = NULL;
    if( malloc_graym( img->width, img->height, &bmap, FALSE ) ||
        malloc_I1m( img->width, img->height, &objs, FALSE ) )
      die( "error: unable to reserve memory" );

    for( n=img->width*img->height-1; n>=0; n-- )
    //  bmap[0][n] = gimg[0][n] > 127 ? 255 : 0;
      bmap[0][n] = gimg[0][n] == 255 ? 255 : 0;

    /// Use RLSA to protect faint strokes ///
    int irlsa[4];
    for( n=0; n<4; n++ )
      irlsa[n] = gb_rlsa[n] < 0 ? 0 : (int)(gb_rlsa[n]+0.5) ;

    char rlsa_op = 0;
    rlsa_op = rlsa_op | ( irlsa[0] ? 0x1 : 0 );
    rlsa_op = rlsa_op | ( irlsa[1] ? 0x2 : 0 );
    rlsa_op = rlsa_op | ( irlsa[2] ? 0x4 : 0 );
    rlsa_op = rlsa_op | ( irlsa[3] ? 0x8 : 0 );

    if( ! rlsa_op )
      logger( 2, "RLSA not used" );

    else {
      if( gb_procimgs ) {
        if( setpixels_magick_graym( img, bmap[0] ) )
          die( "error: unable to set pixels" );
        sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "prerlsa" );
        if( writeimg_magick( procimg, img, logfile ) )
          die( "error: unable to write image %s", procimg );
      }

      logger( 2, "applying RLSA to protect faint strokes: %d,%d,%d,%d pixels", irlsa[0], irlsa[1], irlsa[2], irlsa[3] );
      if( rlsa4_graym( bmap, img->width, img->height, rlsa_op, irlsa, FALSE, bmap ) )
        die( "error: problems applying RLSA" );

      if( gb_procimgs ) {
        if( setpixels_magick_graym( img, bmap[0] ) )
          die( "error: unable to set pixels" );
        sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "rlsa" );
        if( writeimg_magick( procimg, img, logfile ) )
          die( "error: unable to write image %s", procimg );
      }
    }

    for( n=img->width*img->height-1; n>=0; n-- )
      bmap[0][n] = bmap[0][n] == 255 ? 0 : 255;

    /// Get connected components ///
    int nobjs = 0;
    if( objlabel8( bmap, img->width, img->height, objs, &nobjs ) )
      die( "error: problems obtaining connected components" );

    if( nobjs <= 0 ) 
      logger( 2, "warning: encountered an all white image" );

    else {
      if( gb_procimgs ) {
        if( setpixels_magick_graym( img, gimg[0] ) )
          die( "error: unable to set pixels" );
        sprintf( procimg, "%s_%d_%s.png", tool, procnum++, "enh" );
        if( writeimg_magick( procimg, img, logfile ) )
          die( "error: unable to write image %s", procimg );
      }

      if( malloc_I1v( 2*(nobjs+1), &area, TRUE ) )
        die( "error: unable to reserve memory" );

      /// Compute connected component areas ///
      objinfo( objs, img->width, img->height, nobjs, NULL, area, NULL );

      int *rmobj = area+nobjs+1;

      /// Mark small components for removal ///
      int nrm = 0;
      rmobj[0] = 0;
      for( n=1; n<=nobjs; n++ ) {
        rmobj[n] = area[n]<minarea ? 1 : 0;
        nrm += rmobj[n];
      }

      /// Remove marked components from image ///
      for( n=img->width*img->height-1; n>=0; n-- )
        if( rmobj[objs[0][n]] == 1 )
          gimg[0][n] = 255;

      logger( 1, "removed %d small components from a total of %d (min. area: %d pixels^2)", nrm, nobjs, minarea );
    }

    time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
    logger( 4, "time: small component removal (ms): %g", time_end );

    free( bmap );
    free( objs );
    if( area != NULL )
      free( area );
  }

  /// Write output image ///
  time_ini = clock();

  if( msk == NULL || ! gb_savealph ?
        setpixels_magick_graym( img, gimg[0] ) :
        setpixels_magick_grayalphm( img, gimg[0], msk[0] ) )
    die( "error: unable to set pixels" );
  if( writeimg_magick( ofn, img, logfile ) )
    die( "error: unable to write image %s", ofn );

  time_end = 1000.0*((double)clock()-time_ini)/((double)CLOCKS_PER_SEC);
  logger( 4, "time: writing output image (ms): %g", time_end );

  /// Release resources ///
  free_Img( img );
  free( gimg );
  free( ii1 );
  free( ii2 );
  if( msk != NULL )
    free( msk );
  if( cnt != NULL )
    free( cnt );
  if( pmsk != NULL && pmsk != msk )
    free( pmsk );

  if( logfile != stdout && logfile != stderr )
    fclose( logfile );

  MagickCoreTerminus();

  return SUCCESS;
}
