/*
  xu1541_update.c

  xz1541 USB update tool, based on the fischl boot loader
*/

#include <stdio.h>
#include <string.h>

#include <usb.h>

#include "ihex.h"
#include "flash.h"

#include "../../firmware/xu1541_types.h"

/* vendor and product id (donated by ftdi) */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#define MSLEEP(a) Sleep(a)
#else
#define MSLEEP(a) usleep(a*1000)
#endif

/* usb bootloader constants */
#define USBBOOT_FUNC_LEAVE_BOOT 1
#define USBBOOT_FUNC_WRITE_PAGE 2
#define USBBOOT_FUNC_GET_PAGESIZE 3

#ifdef WIN
#define WINKEY { printf("Press return to quit\n"); getchar(); }
#else
#define WINKEY
#endif

static int  usb_get_string_ascii(usb_dev_handle *handle, int index, 
				 char *string, int size) {
  unsigned char buffer[64], *c;
  int len, i;
  
  if((len = usb_control_msg(handle, USB_ENDPOINT_IN, 
			    USB_REQ_GET_DESCRIPTOR, index, 0x0409, 
			    (char*)buffer, sizeof(buffer), 1000)) < 0)
    return 0;
  
  /* string is shorter than the number of bytes returned? */
  if(buffer[0] < len)
    len = buffer[0];

  /* not a string? */
  if(buffer[1] != USB_DT_STRING)
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

static void display_device_info(usb_dev_handle *handle) {
  int nBytes;
  unsigned char reply[6];

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   XU1541_INFO, 0, 0, (char*)reply, sizeof(reply), 1000);

  if(nBytes < 0) {
    fprintf(stderr, "USB request for XU1541 info failed: %s!\n", 
	    usb_strerror());
    return;
  }
  else if((nBytes != sizeof(reply)) && (nBytes != 4)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
    return;
  }

  if (nBytes > 4) {
    printf("Device reports BIOS version %x.%02x\n", reply[4], reply[5]);
  }

  printf("Device reports version %x.%02x\n", reply[0], reply[1]);
  printf("Device reports capabilities 0x%04x\n", *(unsigned short*)(reply+2));
}

/* try to set xu1541 into boot mode */
static int set_to_boot_mode(usb_dev_handle *handle)
{
  printf("Setting xu1541 into boot mode... \n");

  usb_control_msg(handle, 
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
        XU1541_FLASH, 0, 0, 0, 0, 1000);

  MSLEEP(3000); /* wait 3s */

  return 0;
}

/* find and open xu1541 device */
static usb_dev_handle *xu1541_find(unsigned int firstcall) {
  struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_dev_handle      *handle = 0;
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus=usb_get_busses(); bus && !handle; bus=bus->next){
    for(dev=bus->devices; dev && !handle; dev=dev->next){
      if( ( dev->descriptor.idVendor == XU1541_VID && 
	    dev->descriptor.idProduct == XU1541_PID ) ){
	char    string[32];

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

	if(strcmp(string, "xu1541boot") != 0) {
          if (firstcall)  {
            display_device_info(handle);
            /* try to set xu1541 into boot mode */
            set_to_boot_mode(handle);
          }
          else {
	    fprintf(stderr, "Error: Found %s device (version %u.%02u) not "
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
  
  if(!handle) {
    if (!firstcall)
      fprintf(stderr, "Could not find any xu1541 device in boot loader mode\n");
    return NULL;
  }

  if (usb_set_configuration(handle, 1) != 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    usb_close(handle);
    return NULL;
  }

  /* Get exclusive access to interface 0. */
  if (usb_claim_interface(handle, 0) != 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    usb_close(handle);
    return NULL;
  }
  
  return handle;
}

void xu1541_close(usb_dev_handle *handle) {
  /* release exclusive access to device */
  if(usb_release_interface(handle, 0))
    fprintf(stderr, "USB error: %s\n", usb_strerror());
  
  /* close usb device */
  usb_close(handle);

  WINKEY;
}

int xu1541_get_pagesize(usb_dev_handle *handle) {
  unsigned char retval[2];
  int nBytes;

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   USBBOOT_FUNC_GET_PAGESIZE, 0, 0, 
	   (char*)retval, sizeof(retval), 1000);

  if (nBytes != sizeof(retval)) {
    fprintf(stderr, "Error getting page size: %s\n", usb_strerror());
    return -1;
  }

  return 256 * retval[0] + retval[1];
}

int xu1541_start_application(usb_dev_handle *handle) {
  int nBytes;

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   USBBOOT_FUNC_LEAVE_BOOT, 0, 0, 
	   NULL, 0, 1000);

/*
 * This will always be an error, as the xu1541 just reboots, and does not answer!

  if (nBytes != 0) {
    fprintf(stderr, "Error starting application: %s\n", usb_strerror());
    return -1;
  }
*/
  usb_reset( handle ); /* re-enumerate that device */

  return 0;
}

int xu1541_write_page(usb_dev_handle *handle, 
		      char *data, int address, int len) {
  int nBytes;

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
	   USBBOOT_FUNC_WRITE_PAGE, address, 0, 
	   data, len, 1000);

  if(nBytes != len) {
    fprintf(stderr, "Error uploading flash: %s\n", usb_strerror());
    return -1;
  }
  return 0;
}


int main(int argc, char **argv) {
  ihex_file_t *ifile = NULL;
  usb_dev_handle *handle = NULL;
  int page_size, i;
  char *page = NULL;
  unsigned int soft_bootloader_mode = 0;

  printf("--        XU1541 flash updater        --\n");
  printf("--      (c) 2007 by Till Harbaum      --\n");
  printf("-- http://www.harbaum.org/till/xu1541 --\n");
  
  if(argc != 2) {
    fprintf(stderr, "Usage: xu1541_update <ihex_file>\n");
    WINKEY;
    exit(-1);
  }

  usb_init();

  /* find required usb device */
  if(!(handle = xu1541_find(1))) {
    soft_bootloader_mode = 1;
    if(!(handle = xu1541_find(0))) {
      WINKEY;
      exit(1);
    }
  }


  /* check page size */
  if((page_size = xu1541_get_pagesize(handle)) != FLASH_PAGE_SIZE) {
    fprintf(stderr, "Error: unexpected page size %d\n", page_size);
    xu1541_close(handle);
    exit(-1);
  }

  if(!(page = malloc(page_size))) {
    fprintf(stderr, "Error: Out of memory allocating page buffer\n");
    xu1541_close(handle);
    exit(-1);
  }

  /* load the file into memory */
  ifile = ihex_parse_file(argv[1]);

  if(ifile) {
    /* check if xu1541 memory limits are met */
    if(ihex_file_get_start_address(ifile) != 0) {
      fprintf(stderr, "ERROR: Image does not start at address $0\n");
      ihex_free_file(ifile);
      free(page);
      xu1541_close(handle);
      return -1;
    }
    
    /* xu1541 has 6k free flash since 2k are being used by the bootloader */
    if(ihex_file_get_end_address(ifile) >= 6*1024) {
      fprintf(stderr, "ERROR: Image too long by %d bytes\n", 
	      ihex_file_get_end_address(ifile) - 6*1024);
      ihex_free_file(ifile);
      free(page);
      xu1541_close(handle);
      return -1;
    }

    /* and flash it */
    printf("Uploading %d pages\n", flash_get_pages(ifile, page_size));

    for(i=0;i<flash_get_pages(ifile, page_size);i++) {

      /* fill page from ihex image */
      flash_get_page(ifile, i, page, page_size);

      /* and flash it */
      xu1541_write_page(handle, page, i*page_size, page_size);
  
      printf(".");
      fflush(stdout);
    }
      
    ihex_free_file(ifile);
  } else {
    fprintf(stderr, "ERROR: Failed to load hex image\n");
    free(page);
    xu1541_close(handle);
    return -1;
  }

  xu1541_start_application(handle);

  printf(" done\n"
	 "%s\n", 
         (soft_bootloader_mode
          ? "Rebooting the xu1541." 
          : "Please remove jumper switch and replug USB cable "
	    "to return to normal operation!"));

  free(page);
  xu1541_close(handle);  
  return 0;
}
