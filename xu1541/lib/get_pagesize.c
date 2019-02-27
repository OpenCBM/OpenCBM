#include "xu1541lib.h"
#include "xu1541_types.h"

#include <stdio.h>
#include <string.h>

#include "arch.h"

int xu1541lib_get_pagesize(libusb_device_handle *handle) {
  unsigned char retval[2];
  int nBytes;

  nBytes = libusb_control_transfer(handle,
           LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
           USBBOOT_FUNC_GET_PAGESIZE, 0, 0,
           retval, sizeof(retval), 1000);

  if (nBytes != sizeof(retval)) {
    fprintf(stderr, "Error getting page size: %s\n", libusb_error_name(nBytes));
    return -1;
  }

  return 256 * retval[0] + retval[1];
}
