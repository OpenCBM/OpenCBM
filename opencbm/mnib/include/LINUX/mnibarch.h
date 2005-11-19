#include <opencbm.h>
#include "arch.h"
#define delay(x)  arch_usleep(x*1000)
#define msleep(x) delay(x)
