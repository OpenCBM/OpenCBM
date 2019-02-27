#if HAVE_LIBUSB0
#include <usb.h>
#elif HAVE_LIBUSB1
#include <libusb.h>
#endif

/* vendor and product id */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

#include <stdint.h>

typedef struct {
        uint8_t FirmwareVersionMajor;
        uint8_t FirmwareVersionMinor;
        uint16_t Capabilities;
        uint8_t BiosVersionMajor;
        uint8_t BiosVersionMinor;

        uint8_t FirmwareVersionAvailable;
        uint8_t BiosVersionAvailable;
        uint8_t BootloaderMode;
} xu1541_device_info_t;

#if HAVE_LIBUSB0

/* some defines to make supporting libusb0 easier */

typedef usb_dev_handle libusb_device_handle;
typedef void           libusb_context;

#define libusb_error_name(_x)            usb_strerror()
#define libusb_init(_x)                  usb_init()
#define libusb_exit(_x)                  do { } while (0)
#define libusb_release_interface(_a, _b) usb_release_interface(_a, _b)
#define libusb_close(_h)                 usb_close(_h)
#define libusb_set_configuration(_h, _a) usb_set_configuration(_h, _a)
#define libusb_claim_interface(_h, _a)   usb_claim_interface(_h, _a)
#define libusb_reset_device(_h)          usb_reset(_h)

#define libusb_control_transfer(_handle, _req, _cmd, _v0, _v1, _buff, _buffsize, _to) \
        usb_control_msg(_handle, _req, _cmd, _v0, _v1, (char*) (_buff), _buffsize, _to)

#define LIBUSB_REQUEST_TYPE_VENDOR    USB_TYPE_VENDOR
#define LIBUSB_REQUEST_GET_DESCRIPTOR USB_REQ_GET_DESCRIPTOR

#define LIBUSB_RECIPIENT_DEVICE       USB_RECIP_DEVICE
#define LIBUSB_ENDPOINT_IN            USB_ENDPOINT_IN
#define LIBUSB_ENDPOINT_OUT           USB_ENDPOINT_OUT

#define LIBUSB_DT_STRING              USB_DT_STRING

#define LIBUSB_SUCCESS                0

#endif

extern libusb_device_handle * xu1541lib_find(void);
extern libusb_device_handle * xu1541lib_find_in_bootmode(unsigned int *p_soft_bootloader_mode);
extern void                   xu1541lib_close(libusb_device_handle *handle);
extern int                    xu1541lib_get_device_info(libusb_device_handle *handle, xu1541_device_info_t *device_info, unsigned int device_info_length);
extern void                   xu1541lib_display_device_info(libusb_device_handle *handle);
extern int                    xu1541lib_is_in_bootloader_mode(libusb_device_handle *handle);
extern void                   xu1541lib_wait(libusb_device_handle *handle);
extern int                    xu1541lib_set_to_boot_mode(libusb_device_handle *handle);
extern int                    xu1541lib_get_pagesize(libusb_device_handle *handle);
