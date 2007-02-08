#ifndef XU1541_H
#define XU1541_H

#include <stdio.h>
#include <usb.h>

#include "opencbm.h"

#if 0
#define FUNC_ENTER()  printf("CBMLIB: Entering %s\n", __func__)
#define DEBUGF(format, args...) printf("CBMLIB: " format "\n", ##args)
#else
#define FUNC_ENTER()
#define DEBUGF(format, args...)
#endif

/* time out one second after device itself times out */
/* so make sure we should normally never time out on usb */
#define USB_TIMEOUT ((XU1541_W4L_TIMEOUT+1) * 1000)

#include "../../xu1541/firmware/xu1541_types.h"

/* vendor and product id (donated by ftdi) */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

#define XU1541_CONTROL_CHUNK  128

extern usb_dev_handle *xu1541_handle;

/* calls required for standard io */
extern int xu1541_init(void);
extern void xu1541_close(void);
extern int xu1541_ioctl(__u_char cmd, __u_char addr, __u_char secaddr);
extern int xu1541_write(const void *data, size_t len);
extern int xu1541_read(void *data, size_t len);

/* calls for speeder supported modes */
extern int xu1541_special_write(int mode, const void *data, size_t size);
extern int xu1541_special_read(int mode, void *data, size_t size);

#endif // XU1541_H
