/*
 * usb_echo_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xu1541lib.h"

#include "arch.h"

#include "xu1541_types.h"

libusb_device_handle *handle = NULL;

#ifdef WIN
#define QUIT_KEY  { printf("Press return to quit\n"); getchar(); }
#else
#define QUIT_KEY
#endif

// Linux: newer usb.h does not have USB_LE16_TO_CPU() macro anymore
#ifndef USB_LE16_TO_CPU
#if defined(__linux__) || defined(__FreeBSD__)
#ifdef __linux__
#include <endian.h>
#else
#include <sys/endian.h>
#endif
#define USB_LE16_TO_CPU(x) x=le16toh(x);
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define USB_LE16_TO_CPU(x) x=OSSwapLittleToHostInt16(x);
#endif
#endif

/* send a number of 16 bit words to the xu1541 interface */
/* and verify that they are correctly returned by the echo */
/* command. This may be used to check the reliability of */
/* the usb interfacing */
#define ECHO_NUM 256

void usb_echo(void) {

  int i, nBytes, errors=0;
  unsigned short val[2], ret[2];

  printf("=== Running standard echo test ===\n");

  for(i=0;i<ECHO_NUM;i++) {
    val[0] = rand() & 0xffff;
    val[1] = rand() & 0xffff;

    nBytes = libusb_control_transfer(handle,
                 LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
                 XU1541_ECHO, val[0], val[1],
                 (unsigned char*)ret, sizeof(ret), 1000);

    /* the val[x] values are sent through a control message which */
    /* causes libusb to convert their endianess to little endian */
    /* if necessary (read: on big endian machines). since the */
    /* return values ret[x] are just returned as a bytes stream */
    /* and are thus not converted. They have to be explicitely */
    /* converted using the appropriate macros. */
    USB_LE16_TO_CPU(ret[0]);
    USB_LE16_TO_CPU(ret[1]);

    if(nBytes < 0) {
      fprintf(stderr, "USB request failed: %s!\n", libusb_error_name(nBytes));
      return;
    } else if(nBytes != sizeof(ret)) {
      fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
      errors++;
    } else if((val[0] != ret[0]) || (val[1] != ret[1])) {
      fprintf(stderr, "Echo payload mismatch (%x/%x -> %x/%x)\n",
              val[0], val[1], ret[0], ret[1]);
      errors++;
    }
  }

  if(errors)
    fprintf(stderr, "ERROR: %d out of %d echo transfers failed!\n",
            errors, ECHO_NUM);
  else
    printf("%d echo test transmissions successful!\n", ECHO_NUM);
}

void usb_no_irq(void) {

  int i, nBytes, errors=0, tos=0, recovered=0, failed=0;
  unsigned short val[2], ret[2];

  printf("=== Running irq disabled echo test ===\n");

  /* disable IRQs for one second (100 * 10ms) */
  nBytes = libusb_control_transfer(handle,
           LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
           XU1541_IRQ_PAUSE, 100, 0, NULL, 0, 1000);

  if(nBytes < 0) {
    fprintf(stderr, "ERROR: %s!\n", libusb_error_name(nBytes));
  } else if(nBytes == 0) {
    fprintf(stderr, "GOOD: No error sending control message.\n");
  }

  printf("USB errors may (and even should) be reported in the "
         "following lines.\n");

  /* now wait for the device to repond again */

  for(i=0;i<10;i++) {
    val[0] = rand() & 0xffff;
    val[1] = rand() & 0xffff;

    nBytes = libusb_control_transfer(handle,
           LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
                             XU1541_ECHO, val[0], val[1],
                             (unsigned char*)ret, sizeof(ret), 1000);

    USB_LE16_TO_CPU(ret[0]);
    USB_LE16_TO_CPU(ret[1]);

    if(nBytes < 0) {
      fprintf(stderr, "Expected error: %s!\n", libusb_error_name(nBytes));
      failed = 1;
      tos++;
    } else if(nBytes != sizeof(ret)) {
      fprintf(stderr, "Expected error: Wrong number of %d bytes returned, "
              "expected %d\n", nBytes, (int)sizeof(ret));
      failed = 1;
      tos++;
    } else if((val[0] != ret[0]) || (val[1] != ret[1])) {
      fprintf(stderr, "Echo payload mismatch (%x/%x -> %x/%x)\n",
              val[0], val[1], ret[0], ret[1]);
      errors++;
    } else {
      if(failed) recovered = 1;

      printf("Echo successful\n");
    }

    MSLEEP(200); /* wait 200ms */
  }

  fprintf(stderr, "USB timeout states: %d\n", tos);

  if(!failed)
    fprintf(stderr, "ERROR: Device did not enter IRQ PAUSE state\n");
  else {
    if(!recovered)
      fprintf(stderr, "ERROR: Device did not recover!!!\n");
    else {
      printf("GOOD: Device/USB link successfully recovered from disabled "
             "target irq\n");
    }
  }

  if(errors)
    fprintf(stderr, "ERROR: %d echo transfers failed!\n", errors);
}

int main(int argc, char *argv[]) {
  libusb_context * usbContext;

  printf("--      XU1541 USB test application        --\n"
         "--     (c) 2007, 2019 the opencbm team     --\n"
         "--       https://github.com/OpenCBM        --\n"
         "-- http://sourceforge.net/projects/opencbm --\n\n");

  libusb_init(&usbContext);

  if(!(handle=xu1541lib_find())) {
    return 1;
  }

  xu1541lib_display_device_info(handle);

  /* make xu1541 interface return some bytes to */
  /* test transfer reliability */
  usb_echo();
  usb_no_irq();

  xu1541lib_close(handle);

  libusb_exit(usbContext);

  return 0;
}
