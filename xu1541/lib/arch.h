#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#define MSLEEP(a) Sleep(a)
#else
#include <unistd.h>
#define MSLEEP(a) usleep(a*1000)
#endif
