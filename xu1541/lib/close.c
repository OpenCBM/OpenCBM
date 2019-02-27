#include "xu1541lib.h"
#include "xu1541_types.h"

#include <stdio.h>
#include <string.h>

#include "arch.h"

void xu1541lib_close(libusb_device_handle *handle) {
  /* release exclusive access to device */
  int ret = libusb_release_interface(handle, 0);
  if (ret != LIBUSB_SUCCESS) {
    fprintf(stderr, "USB error: %s\n", libusb_error_name(ret));
  }

  /* close usb device */
  libusb_close(handle);
}
