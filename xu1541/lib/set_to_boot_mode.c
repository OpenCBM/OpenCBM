#include "xu1541lib.h"
#include "xu1541_types.h"

#include <stdio.h>
#include <string.h>

#include "arch.h"

/* try to set xu1541 into boot mode */
int xu1541lib_set_to_boot_mode(libusb_device_handle *handle)
{
  printf("Setting xu1541 into boot mode... \n");

  libusb_control_transfer(handle,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
        XU1541_FLASH, 0, 0, 0, 0, 1000);

  xu1541lib_wait(handle);

  return 0;
}
