/**
 * Logging functions
 *
 * @version $Revision:: 165   $$Date:: 2015-02-23 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#include "log.h"

#include <stdarg.h>
#include <string.h>

void _proxy_log(FILE *log_file, int log_level, int level, const char *fmt, ...) {
  va_list arg;

  if( ( ! log_level ) || level > log_level )
    return;

  va_start(arg, fmt);
  vfprintf(log_file, fmt, arg);
  va_end(arg);
}

void _print_svn_rev( FILE *file, char *revnum, char *revdate, char *tool ) {
  char ver[128];
  char *p;
  sprintf( ver, "%s: revision %s", tool, revnum+11 );
  p = strchr( ver, '$' );
  sprintf( p, "(%s", revdate+7 );
  p = strchr( p+1, '(' );
  *(p-1) = ')';
  *p = '\0';
  fprintf( file, "%s\n", ver );
}
