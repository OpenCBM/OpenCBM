/*
  cbootloader.cpp - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25

  Parts are taken from the PowerSwitch project by Objective Development Software GmbH
*/

#include "cbootloader.h"

static int  getDescriptorString(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
  char    buffer[256];
  int     len, i;

  if((len = usb_control_msg(dev, USB_ENDPOINT_IN, 
			    USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 
			    langid, buffer, sizeof(buffer), 1000)) < 0)
    return len;
  
  if(buffer[1] != USB_DT_STRING)
    return 0;
  
  /* limit string len to embedded len */
  if((unsigned char)buffer[0] < len) len = (unsigned char)buffer[0];  
  len = len/2-1;  

  /* limit to buffer size */
  if(len > buflen) len = buflen;

  /* lossy conversion to ISO Latin1 */
  for(i=0;i<len;i++){
    buf[i] = buffer[2 + 2*i];
    if(buffer[3 + 2*i])
      buf[i] = '_';
  }
  buf[i] = 0;

  return i;
}

/* vendor and product id (donated by ftdi) */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

/* This project uses the free shared default VID/PID. If you want to see an
 * example device lookup where an individually reserved PID is used, see our
 * RemoteSensor reference implementation.
 */
static usb_dev_handle   *findDevice(void)
{
struct usb_bus      *bus;
struct usb_device   *dev;
usb_dev_handle      *handle = 0;

    usb_find_busses();
    usb_find_devices();

    for(bus=usb_get_busses(); bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == XU1541_VID && dev->descriptor.idProduct == XU1541_PID){
                char    string[256];
                int     len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    printf("Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }
                len = getDescriptorString(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                if(len < 0){
                    printf("warning: cannot query product name for device: %s\n", usb_strerror());
		    if(handle) usb_close(handle);
		    handle = NULL;
                }

		if(strcmp(string, "xu1541boot") != 0) {
		    printf("Error: Found %s device (version %u.%02u) not in boot loader\n"
			   "       mode, please install jumper switch and replug device!\n", 
			   string, dev->descriptor.bcdDevice >> 8, dev->descriptor.bcdDevice & 0xff);
		    
		    if(handle) usb_close(handle);
		    handle = NULL;		  
		}
		
		break;
            }
        }
        if(handle)
            break;
    }

    if(!handle)
        printf("Could not find any xu1541 device in boot loader mode\n");

    return handle;
}



CBootloader::CBootloader() {
  usb_init();
  if((usbhandle = findDevice()) == NULL) {
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }
}

CBootloader::~CBootloader() {
  usb_close(usbhandle);
}

unsigned int CBootloader::getPagesize() {
  char       buffer[8];
  int                 nBytes;

  nBytes = usb_control_msg(usbhandle, 
			   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			   3, 0, 0, 
			   buffer, sizeof(buffer), 
			   5000);

  if (nBytes != 2) {
    printf("Error: wrong response size in getPageSize: %d !\n", nBytes);
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }

  return (buffer[0] << 8) | buffer[1];
}

void CBootloader::startApplication() {
  char       buffer[8];
  int                 nBytes;

  nBytes = usb_control_msg(usbhandle, 
			   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
			   1, 0, 0, 
			   buffer, sizeof(buffer), 
			   5000);

  if (nBytes != 0) {
    printf("Error: wrong response size in startApplication: %d !\n", nBytes);
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }
}


void CBootloader::writePage(CPage* page) {

  int nBytes;

  nBytes = usb_control_msg(usbhandle, 
			   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
			   2, page->getPageaddress(), 0, 
			   (char*) page->getData(), page->getPagesize(), 
			   5000);

  if (nBytes != (signed)page->getPagesize()) {
    printf("Error: wrong byte count in writePage: %d !\n", nBytes);
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }
}

