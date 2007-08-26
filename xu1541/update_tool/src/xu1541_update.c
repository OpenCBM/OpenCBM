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

/* do not recognize other xu1541-BIOS adopted firmwares */
/* comment out to enable "foreign" firmwares            */
#define RECOGNIZE_TRUE_XU1541_ONLY

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

int xu1541_write_page(usb_dev_handle *handle, 
		      char *data, int address, int len);

const static struct recognized_usb_devices_t {
  unsigned short vid;
  unsigned short pid;
} recognized_usb_devices[] = {
  { 0x0403, 0xc632 }   /* xu1541 vendor and product id (donated by ftdi) */
#if !defined(RECOGNIZE_TRUE_XU1541_ONLY)
  , { 0x16c0, 0x05dc } /* USBasp vendor and product id */
#endif
};

static int usb_was_resetted = 0;

static unsigned char *page_for_address_0 = NULL;
static unsigned int   page_for_address_0_is_valid = 0;

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

/* wait for the xu1541 to react again */
static void xu1541_wait(usb_dev_handle *handle)
{
  /* TODO: currently a dummy only */

  MSLEEP(3000); /* wait 3s */
}

/* try to set xu1541 into boot mode */
static int xu1541_set_to_boot_mode(usb_dev_handle *handle)
{
  printf("Setting xu1541 into boot mode... \n");

  usb_control_msg(handle, 
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
        XU1541_FLASH, 0, 0, 0, 0, 1000);

  xu1541_wait(handle);

  return 0;
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
static usb_dev_handle *xu1541_find(unsigned int firstcall) {
  struct usb_bus      *bus;
  struct usb_device   *dev;
  usb_dev_handle      *handle = 0;
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus=usb_get_busses(); bus && !handle; bus=bus->next){
    for(dev=bus->devices; dev && !handle; dev=dev->next){
      if(is_xu1541bios_device(dev->descriptor.idVendor,
                              dev->descriptor.idProduct)) {
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
            xu1541_set_to_boot_mode(handle);
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
  if(!usb_was_resetted && usb_release_interface(handle, 0))
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

int xu1541_start_application(usb_dev_handle *handle, int page_size) {
  int nBytes;

  if (page_for_address_0_is_valid) {
    /* page 0 was flashed 'invalidly', correct it now as last step */
    /* before rebooting. */
    xu1541_write_page(handle, (char*)page_for_address_0, 0, page_size);
  }

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   USBBOOT_FUNC_LEAVE_BOOT, 0, 0, 
	   NULL, 0, 1000);

/*
 * This will always be an error, as the xu1541 just reboots, and does 
 * not answer!

  if (nBytes != 0) {
    fprintf(stderr, "Error starting application: %s\n", usb_strerror());
    return -1;
  }
*/
  usb_reset( handle ); /* re-enumerate that device */
  usb_was_resetted = 1;

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
  int start;
  char *page = NULL;
  unsigned int soft_bootloader_mode = 0;

  printf("--        XU1541 flash updater        --\n");
  printf("--    (c) 2007 by the opencbm team    --\n");
  printf("-- http://www.harbaum.org/till/xu1541 --\n");
  
  if(argc < 2) {
    fprintf(stderr, "Usage: xu1541_update [-o OFFSET] <ihex_file> [[-o OFFSET] <ihex_file2> ...]\n");
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

  if(!(page_for_address_0 = malloc(page_size))) {
    fprintf(stderr, "Error: Out of memory allocating page buffer\n");
    xu1541_close(handle);
    exit(-1);
  }

  do {
          unsigned int flash_offset = 0;

          /* process -o (offset) parameter */
          if (strncmp(argv[1], "-o", 2) == 0) {
            char *pos = argv[1]+2;

            if (*pos == '=')
              ++pos;

            flash_offset = strtol(pos, &pos, 0);
            printf("An offset of 0x%04x is specified!\n", flash_offset);

            if (*pos != 0) {
              fprintf(stderr, "ERROR: extra input '%s' after offset of 0x%04x.\n", pos, flash_offset);
              xu1541_close(handle);
              return -1;
            }

            /* proceed to next argument */
            ++argv;
            --argc;
          }

          /* process -R (reboot) parameter */
          if (strcmp(argv[1], "-R") == 0) {
            printf("Starting application...\n");
            xu1541_start_application(handle, page_size);
            printf("Waiting for reboot...\n");
            xu1541_wait(handle);
            xu1541_wait(handle); /* TODO: remove that and replace the xu1541_wait with a better approach */

            /* find required usb device */
            printf("Find xu1541 again...\n");
            if(!(handle = xu1541_find(1))) {
              if(!(handle = xu1541_find(0))) {
                WINKEY;
                exit(1);
              }
            }

            /* proceed to next argument */
            ++argv;
            --argc;
          }

          /* load the file into memory */

          ifile = ihex_parse_file(argv[1]);

          if(ifile) {
            /* flash the file */
            printf("Uploading %d pages ", flash_get_pages(ifile, page_size, &start));
            printf("starting from 0x%04x", start);

            if (flash_offset != 0) {
                    start -= flash_offset;
                    printf(", moved to 0x%04x", start);
            }
            printf("\n");

            for(i=0;i<flash_get_pages(ifile, page_size, NULL);i++) {

              /* fill page from ihex image */
              flash_get_page(ifile, i, page, page_size);

              /* special handling of page 0: */

              if (i * page_size + start == 0) {
                /* remember the data to write */
                memcpy(page_for_address_0, page, page_size);
                page_for_address_0_is_valid = 1;

                /* mark the page address is invalid application */
                page[0] = -1;
                page[1] = -1;
              }

              /* and flash it */
              xu1541_write_page(handle, page, i*page_size + start, page_size);
          
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

          /* proceed to next file */
          if (argc > 2)
             printf(" done\n");
          --argc;
          ++argv;

  } while (argc >= 2);

  xu1541_start_application(handle, page_size);

  printf(" done\n"
	 "%s\n", 
         (soft_bootloader_mode
          ? "Rebooting the xu1541." 
          : "If you had installed a jumper switch, please remove it and replug the USB cable\n"
	    "to return to normal operation!\n"
            "If not, the xu1541 will reboot itself automatically.\n"));

  free(page);
  xu1541_close(handle);  
  return 0;
}
