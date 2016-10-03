/**
 * Logging functions
 *
 * @version $Revision:: 198   $$Date:: 2015-10-13 #$
 * @copyright Copyright (c) 2004 to the present, Mauricio Villegas <mauvilsa@upv.es>
 */

#ifndef __MV_LOG_H__
#define __MV_LOG_H__

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

#include <stdio.h>

void _proxy_log(FILE *log_file, int log_level, int level, const char *fmt, ...)
    __attribute__((format (printf, 4, 5)));
#define proxy_log(log_file, log_level, level, fmt, ...) _proxy_log(log_file, log_level, level, fmt"\n", ##__VA_ARGS__)

#define logger( level, fmt, ... ) proxy_log( logfile, verbosity, level, "%s: " fmt, tool, ##__VA_ARGS__ )
#define die( fmt, ... ) { proxy_log( logfile, verbosity, 0, "%s: " fmt, tool, ##__VA_ARGS__ ); return FAILURE; }

void _print_svn_rev( FILE *file, char *revnum, char *revdate, char *tool );
#define print_svn_rev(file) _print_svn_rev(file,revnum,revdate,tool);

#define strbool(cond) ( (cond) ? "true" : "false" )

#endif
