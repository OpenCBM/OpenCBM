#include "xu1541lib.h"
#include "xu1541_types.h"

#include <stdio.h>
#include <string.h>

#include "arch.h"

/* wait for the xu1541 to react again */
void xu1541lib_wait(libusb_device_handle *handle)
{
  /* TODO: currently a dummy only */

  MSLEEP(3000); /* wait 3s */
}
