#include <stdio.h>
#include "Error.h"
/*
 * printError - Signal an error. /Stegu
 */
void printError( const char *errtype, const char *errmsg ) {
    fprintf( stderr, "%s: %s\n", errtype, errmsg );
}
