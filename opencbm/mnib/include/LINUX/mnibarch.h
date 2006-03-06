#include <opencbm.h>
#include <unistd.h>

#define delay(x)  usleep((x) * 1000)
#define msleep(x) delay(x)

#define ARCH_MAINDECL
#define ARCH_SIGNALDECL

typedef unsigned char BYTE;
