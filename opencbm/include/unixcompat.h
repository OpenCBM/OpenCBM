#include "version.h"
#define VERSION CBM4WIN_VERSION_STRING

#include <string.h>

#define strcasecmp(_x,_y) _stricmp(_x,_y)
#define strncasecmp(_x,_y,_z) _strnicmp(_x,_y,_z)

#define sleep(_x) Sleep((_x) * 1000)
#define usleep(_x) Sleep((_x) / 1000)

extern void unix_error(int a, unsigned int ErrorCode, const char *format, ...);
extern char *unix_strerror(unsigned int ErrorCode);

/*! set errno variable */
#define set_errno(_x) SetLastError(_x)
#define get_errno()   GetLastError()

/* dummys for compiling */

#include <stdlib.h> /* for getenv */
/* Make sure that getenv() will not be defined with a prototype
   in unixcompat/getopt.c, which would result in a compiler error 
   "error C2373: 'getenv' : redefinition; different type modifiers".
*/
#define getenv getenv

#define unlink(_x) _unlink(_x)

#include <sys/stat.h>
#define stat(_x, _y) _stat(_x, _y)

#define strdup(_x) _strdup(_x)

#include <stdio.h>
#undef fileno
#define fileno(_x) _fileno(_x)

#include <io.h>
#define ftruncate(_x, _y) _chsize(_x, _y)
