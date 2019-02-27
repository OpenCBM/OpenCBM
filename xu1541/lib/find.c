#include "xu1541lib.h"
#include "xu1541_types.h"

#include <stdio.h>
#include <string.h>

#include "arch.h"

/* do not recognize other xu1541-BIOS adopted firmwares */
/* comment out to enable "foreign" firmwares            */
#define RECOGNIZE_TRUE_XU1541_ONLY

const static struct recognized_usb_devices_t {
  unsigned short vid;
  unsigned short pid;
} recognized_usb_devices[] = {
  { XU1541_VID, XU1541_PID }   /* xu1541 vendor and product id (donated by ftdi) */
#if !defined(RECOGNIZE_TRUE_XU1541_ONLY)
  , { 0x16c0, 0x05dc } /* USBasp vendor and product id */
#endif
};

static int usb_get_string_ascii(libusb_device_handle *handle, int index,
                                 char *string, int size) {
  unsigned char buffer[64], *c;
  int len, i;

  if((len = libusb_control_transfer(handle, LIBUSB_ENDPOINT_IN,
                                    LIBUSB_REQUEST_GET_DESCRIPTOR, index, 0x0409,
                                    buffer, sizeof(buffer), 1000)) < 0)
    return 0;

  /* string is shorter than the number of bytes returned? */
  if(buffer[0] < len)
    len = buffer[0];

  /* not a string? */
  if(buffer[1] != LIBUSB_DT_STRING)
    return 0;

  /* length is in unicode 16 bit chars and includes 2 byte header */
  len = (len-2)/2;

  /* string is longer than buffer provided? */
  if(len > size-1)
    len = size-1;

  /* take only lower byte for simple unicode to ascii conversion */
  for(i=0,c = buffer+2;i<len;i++,c+=2){
    if(!(*(c+1)))
      string[i] = *c;
    else
      string[i] = '_';
  }

  /* terminate string and return length */
  string[i] = 0;
  return 1;
}

/* check for known firmwares with xu1541 BIOS abilities */
static int is_xu1541bios_device(unsigned short vid, unsigned short pid) {
  int i;
  for(i=0;i<sizeof(recognized_usb_devices);++i) {
    if( recognized_usb_devices[i].vid == vid &&
        recognized_usb_devices[i].pid == pid ) {
      return 1;
    }
  }
  return 0;
}

/* find and open xu1541 device */
static libusb_device_handle *find_internal(unsigned int displaydeviceinfo, unsigned int in_bootmode, unsigned int print_error) {
  libusb_device_handle *handle = 0;
  int                   ret;

#if HAVE_LIBUSB0
  struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_find_busses();
  usb_find_devices();

  for(bus=usb_get_busses(); bus && !handle; bus=bus->next){
    for(dev=bus->devices; dev && !handle; dev=dev->next){
      if(is_xu1541bios_device(dev->descriptor.idVendor,
                              dev->descriptor.idProduct)) {
        char string[32];

        /* we need to open the device in order to query strings */
        handle = usb_open(dev);
        if(!handle){
          fprintf(stderr, "Warning: cannot open USB device: %s\n",
                  usb_strerror());
          continue;
        }

        if(!usb_get_string_ascii(handle,
                 (USB_DT_STRING << 8) | dev->descriptor.iProduct,
                 string, sizeof(string))) {
          fprintf(stderr, "Error: Cannot query product name "
                  "for device: %s\n", usb_strerror());
          if(handle) usb_close(handle);
          handle = NULL;
        }

        if (displaydeviceinfo) {
          xu1541lib_display_device_info(handle);
        }

        if(in_bootmode && strcmp(string, "xu1541boot") != 0 && ! xu1541lib_is_in_bootloader_mode(handle)) {
          if (in_bootmode)  {
            /* try to set xu1541 into boot mode */
            xu1541lib_set_to_boot_mode(handle);
          }
          else {
            fprintf(stderr, "Error: Found %s device (version %x.%02x) not "
                  "in boot loader\n"
                  "       mode, please install jumper switch "
                  "and replug device!\n",
                  string, dev->descriptor.bcdDevice >> 8,
                  dev->descriptor.bcdDevice & 0xff);
          }

          if(handle) usb_close(handle);
          handle = NULL;
        }
      }
    }
  }
#elif HAVE_LIBUSB1
  libusb_device **list;
  ssize_t cnt;
  ssize_t i = 0;
  int err;

  cnt = libusb_get_device_list(NULL, &list);
  if (cnt < 0) {
    fprintf(stderr, "enumeration error: %s", libusb_error_name(cnt));
    return NULL;
  }

  for (i = 0; i < cnt; i++) {
    libusb_device *device = list[i];
    struct libusb_device_descriptor descriptor;

    if (libusb_get_device_descriptor(device, &descriptor) != LIBUSB_SUCCESS)
      continue;

    fprintf(stderr, "Device %04x:%04x\n", descriptor.idVendor, descriptor.idProduct);

    // First, find our vendor and product id
    if (descriptor.idVendor != XU1541_VID || descriptor.idProduct != XU1541_PID)
      continue;

    fprintf(stderr, "Found xu1541 device on bus %u device %u.\n",
      libusb_get_bus_number(device), libusb_get_device_address(device));

    /* open device */
    err = libusb_open(device, &handle);
    if (err != LIBUSB_SUCCESS) {
      fprintf(stderr, "error: Cannot open USB device: %s\n", libusb_error_name(err));
      continue;
    }

#if 0
    /* get device name and make sure the name is "xu1541" meaning */
    /* that the device is not in boot loader mode */
    len = usb.get_string_descriptor_ascii(HandleXu1541->devh, descriptor.iProduct,
                            string, sizeof(string) - 1);
    if (len < 0) {
      fprintf(stderr, "warning: cannot query product "
              "name for device: %s\n", usb.error_name(len));
      if (HandleXu1541->devh)
        usb.close(HandleXu1541->devh);
      HandleXu1541->devh = NULL;
    }

    /* make sure the name matches what we expect */
    if (strcmp((char*)string, "xu1541") != 0) {
      fprintf(stderr, "Error: Found xu1541 in unexpected state,"
              " please make sure device is _not_ in bootloader mode!\n");

      if (HandleXu1541->devh)
        usb.close(HandleXu1541->devh);
      HandleXu1541->devh = NULL;
    }
#else
    if(is_xu1541bios_device(descriptor.idVendor,
                            descriptor.idProduct)) {
        char string[32];

        if(!usb_get_string_ascii(handle,
                 (LIBUSB_DT_STRING << 8) | descriptor.iProduct,
                 string, sizeof(string))) {
          fprintf(stderr, "Error: Cannot query product name "
                  "for device: %s\n", libusb_error_name(ret));
          if(handle) libusb_close(handle);
          handle = NULL;
        }

        if (displaydeviceinfo) {
          xu1541lib_display_device_info(handle);
        }

        if(in_bootmode && strcmp(string, "xu1541boot") != 0 && ! xu1541lib_is_in_bootloader_mode(handle)) {
          if (in_bootmode)  {
            /* try to set xu1541 into boot mode */
            xu1541lib_set_to_boot_mode(handle);
          }
          else {
            fprintf(stderr, "Error: Found %s device (version %x.%02x) not "
                  "in boot loader\n"
                  "       mode, please install jumper switch "
                  "and replug device!\n",
                  string, descriptor.bcdDevice >> 8,
                  descriptor.bcdDevice & 0xff);
          }

          if(handle) libusb_close(handle);
          handle = NULL;
        }
    }
#endif
  }
#endif

  if(!handle) {
    if (print_error)
      fprintf(stderr, "Could not find any xu1541 device%s!\n", in_bootmode ? " in boot loader mode" : "");
    return NULL;
  }

  ret = libusb_set_configuration(handle, 1);
  if (ret != LIBUSB_SUCCESS) {
    fprintf(stderr, "USB error: %s\n", libusb_error_name(ret));
    libusb_close(handle);
    return NULL;
  }

  /* Get exclusive access to interface 0. */
  ret = libusb_claim_interface(handle, 0);
  if (ret != LIBUSB_SUCCESS) {
    fprintf(stderr, "USB error: %s\n", libusb_error_name(ret));
    libusb_close(handle);
    return NULL;
  }

  return handle;
}

libusb_device_handle *xu1541lib_find(void) {
  return find_internal(0, 0, 1);
}

libusb_device_handle *xu1541lib_find_in_bootmode(unsigned int *p_soft_bootloader_mode) {
  libusb_device_handle *handle;

  if(!(handle = find_internal(1,1,0))) {
    if (p_soft_bootloader_mode)
      *p_soft_bootloader_mode = 1;

    handle = find_internal(0,1,1);
  }

  return handle;
}
