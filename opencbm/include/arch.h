#ifndef CBM_ARCH_H
#define CBM_ARCH_H

#ifdef WIN32
# include "version.h"

# define VERSION CBM4WIN_VERSION_STRING
# define ARCH_CBM_LINUX_WIN( _linux, _win) _win

# include <stdio.h>
# include <io.h>
# include <fcntl.h>

#else

# define ARCH_CBM_LINUX_WIN( _linux, _win) _linux

# include <unistd.h>
# include <errno.h>
# include <error.h>

#endif

#include <string.h>

#define arch_strcasecmp(_x,_y)     ARCH_CBM_LINUX_WIN(strcasecmp(_x,_y), _stricmp(_x,_y))
#define arch_strncasecmp(_x,_y,_z) ARCH_CBM_LINUX_WIN(strncasecmp(_x,_y,_z), _strnicmp(_x,_y,_z))

#define arch_sleep(_x)  ARCH_CBM_LINUX_WIN(sleep(_x), Sleep((_x) * 1000))
#define arch_usleep(_x) ARCH_CBM_LINUX_WIN(usleep(_x), Sleep((_x) / 1000))

#ifdef WIN32
 extern void arch_error(int AUnused, unsigned int ErrorCode, const char *format, ...);
 extern char *arch_strerror(unsigned int ErrorCode);
#else
# define arch_error error
# define arch_strerror strerror
#endif

/*! set errno variable */
#define arch_set_errno(_x) ARCH_CBM_LINUX_WIN((errno = (_x)), SetLastError(_x))
#define arch_get_errno()   ARCH_CBM_LINUX_WIN((errno), GetLastError())

#define arch_atoc(_x) ((unsigned char) atoi(_x))

/* dummys for compiling */

#ifdef WIN32
# include <stdlib.h> /* for getenv */
/* Make sure that getenv() will not be defined with a prototype
   in arch/windows/getopt.c, which would result in a compiler error 
   "error C2373: 'getenv' : redefinition; different type modifiers".
*/
# define getenv getenv
#endif

#define arch_unlink(_x) ARCH_CBM_LINUX_WIN(unlink(_x), _unlink(_x))

int arch_filesize(const char *Filename, size_t *Filesize);

#define arch_strdup(_x) ARCH_CBM_LINUX_WIN(strdup(_x), _strdup(_x))

#define arch_fileno(_x) ARCH_CBM_LINUX_WIN(fileno(_x), _fileno(_x))

#define arch_setbinmode(_x) ARCH_CBM_LINUX_WIN(/* already in bin mode */, _setmode(_x, _O_BINARY))

#define arch_ftruncate(_x, _y) ARCH_CBM_LINUX_WIN(ftruncate(_x, _y), _chsize(_x, _y))

#define arch_fdopen(_x, _y) ARCH_CBM_LINUX_WIN(fdopen(_x, _y), _fdopen(_x, _y))

#define ARCH_MAINDECL   ARCH_CBM_LINUX_WIN(/* no decl needed */, __cdecl)
#define ARCH_SIGNALDECL ARCH_CBM_LINUX_WIN(/* no decl needed */, __cdecl)

#endif /* #ifndef CBM_ARCH_H */
