/*
 * xum1541 driver for bulk and control messages
 * Copyright 2009-2010 Nate Lawson <nate@root.org>
 * Copyright 2010 Spiro Trikaliotis
 * Copyright 2012 Arnd Menge
 *
 * Incorporates some code from the xu1541 driver by:
 * Copyright 2007 Till Harbaum <till@harbaum.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*! **************************************************************
** \file lib/plugin/xum1541/xum1541.c \n
** \author Nate Lawson \n
** \n
** \brief libusb-based xum1541 access routines
****************************************************************/

// This XUM1541 plugin has tape support.
#define TAPE_SUPPORT 1

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "opencbm.h"

#include "arch.h"
#include "dynlibusb.h"
#include "getpluginaddress.h"
#include "xum1541.h"

static int debug_level = -1; /*!< \internal \brief the debugging level for debugging output */

unsigned char DeviceDriveMode; // Temporary disk/tape mode hack until usb device handle context is there.

/*! \internal \brief Output debugging information for the xum1541

 \param level
   The output level; output will only be produced if this level is less or equal the debugging level

 \param msg
   The printf() style message to be output
*/
static void
xum1541_dbg(int level, char *msg, ...)
{
    va_list argp;

    /* determine debug mode if not yet known */
    if (debug_level == -1) {
        char *val = getenv("XUM1541_DEBUG");
        if (val)
            debug_level = atoi(val);
    }

    if (level <= debug_level) {
        fprintf(stderr, "[XUM1541] ");
        va_start(argp, msg);
        vfprintf(stderr, msg, argp);
        va_end(argp);
        fprintf(stderr, "\n");
    }
}

/*! \internal \brief Output (start of) transferred data for debugging

 \param level
   The output level; output will only be produced if this level is less or equal the debugging level

 \param op
   The operation performed ("read" or "write")

 \param data
   The data buffer

 \param len
   The length of the data in the buffer
*/
static void
xum1541_print_data(int level, const char *op, const unsigned char *data, unsigned int len)
{
    // optimize for no debug output
    if (level > debug_level) {
        return;
    }

    switch (len) {
    case 0:
        return;
    case 1:
        xum1541_dbg(level, "%s %d bytes (%02x)", op, len,
                    data[0]);
        break;
    case 2:
        xum1541_dbg(level, "%s %d bytes (%02x %02x)", op, len,
                    data[0], data[1]);
        break;
    case 3:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x)", op, len,
                    data[0], data[1], data[2]);
        break;
    case 4:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x)", op, len,
                    data[0], data[1], data[2], data[3]);
        break;
    case 5:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x %02x)", op, len,
                    data[0], data[1], data[2], data[3], data[4]);
        break;
    case 6:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x %02x %02x)", op, len,
                    data[0], data[1], data[2], data[3], data[4], data[5]);
        break;
    case 7:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x %02x %02x %02x)", op, len,
                    data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        break;
    case 8:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x %02x %02x %02x %02x)", op, len,
                    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
        break;
    default:
        xum1541_dbg(level, "%s %d bytes (%02x %02x %02x %02x %02x %02x %02x %02x ...)", op, len,
                    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
    }
}

// Cleanup after a failure
static void
xum1541_cleanup(struct opencbm_usb_handle *HandleXum1541, char *msg, ...)
{
    va_list args;

    if (msg != NULL) {
        va_start(args, msg);
        fprintf(stderr, msg, args);
        va_end(args);
    }
    if (HandleXum1541 != NULL) {
        usb.close(HandleXum1541->devh);
        HandleXum1541->devh = NULL;
    }
}

// USB bus enumeration
static int
xum1541_enumerate(struct opencbm_usb_handle *HandleXum1541, int PortNumber)
{
    static char xumProduct[] = "xum1541"; // Start of USB product string id
    static int prodLen = sizeof(xumProduct) - 1;
#if HAVE_LIBUSB0
    struct usb_bus *bus;
    struct usb_device *dev, *preferredDefaultHandle;
#elif HAVE_LIBUSB1
    libusb_device **list;
    struct opencbm_usb_handle found = { NULL, NULL };
    libusb_device *preferredDefaultHandle = NULL;
    struct libusb_device_descriptor descriptor;
    ssize_t cnt;
    ssize_t i = 0;
    int err = 0;
#endif
    int len, serialnum, leastserial;
    unsigned char string[256];

    if (PortNumber < 0 || PortNumber > MAX_ALLOWED_XUM1541_SERIALNUM) {
        // Normalise the Portnumber for invalid values
        PortNumber = 0;
    }

    xum1541_dbg(0, "scanning usb ...");

#if HAVE_LIBUSB0
    usb.init();
    usb.find_busses();
    usb.find_devices();

    /* usb.find_devices sets errno if some devices don't reply 100% correct. */
    /* make lib ignore this as this has nothing to do with our device */
    errno = 0;

    preferredDefaultHandle = NULL;
    leastserial = MAX_ALLOWED_XUM1541_SERIALNUM + 1;
    for (bus = usb.get_busses(); !HandleXum1541->devh && bus; bus = bus->next) {
        xum1541_dbg(1, "scanning bus %s", bus->dirname);
        for (dev = bus->devices; !HandleXum1541->devh && dev; dev = dev->next) {
            xum1541_dbg(1, "device %04x:%04x at %s",
                dev->descriptor.idVendor, dev->descriptor.idProduct,
                dev->filename);

            // First, find our vendor and product id
            if (dev->descriptor.idVendor != XUM1541_VID ||
                dev->descriptor.idProduct != XUM1541_PID)
                continue;

            xum1541_dbg(0, "found xu/xum1541 version %04x on bus %s, device %s",
                dev->descriptor.bcdDevice, bus->dirname, dev->filename);
            if ((HandleXum1541->devh = usb.open(dev)) == NULL) {
                fprintf(stderr, "error: Cannot open USB device: %s\n",
                    usb.strerror());
                continue;
            }

            // Get device product name and try to match against "xum1541".
            // If no match, it could be an xum1541 so don't report an error.
            len = usbGetStringAscii(HandleXum1541, dev->descriptor.iProduct,
                0x0409, string, sizeof(string) - 1);
            if (len < 0) {
                xum1541_cleanup(HandleXum1541,
                    "error: cannot query product name: %s\n", usb.strerror());
                continue;
            }
            string[len] = '\0';
            if (len < prodLen || strstr((char *)string, xumProduct) == NULL) {
                xum1541_cleanup(HandleXum1541, NULL);
                continue;
            }
            xum1541_dbg(0, "xum1541 name: %s", string);

            len = usbGetStringAscii(HandleXum1541,
                dev->descriptor.iSerialNumber, 0x0409,
                string, sizeof(string) - 1);
            if (len < 0 && PortNumber != 0){
                // we need the serial number, when PortNumber is not 0
                xum1541_cleanup(HandleXum1541,
                    "error: cannot query serial number: %s\n",
                    usb.strerror());
                continue;
            }
            serialnum = 0;
            if (len > 0 && len <=3 ) {
                string[len] = '\0';
                serialnum = atoi((char *)string);
            }
            if (PortNumber != serialnum) {
                // keep in mind the handle, if the device's
                // serial number is less than previous ones
                if(serialnum < leastserial) {
                    leastserial = serialnum;
                    preferredDefaultHandle = dev;
                }
                xum1541_cleanup(HandleXum1541, NULL);
                continue;
            }

            xum1541_dbg(0, "xum1541 serial number: %3u", serialnum);
            return 0;
        }
    }
    // if no default device was found because only specific devices were present,
    // determine the default device from the specific ones and open it
    if(preferredDefaultHandle != NULL) {
        if ((HandleXum1541->devh = usb.open(preferredDefaultHandle)) == NULL) {
            fprintf(stderr, "error: Cannot reopen USB device: %s\n",
                usb.strerror());
        }
    }
#elif HAVE_LIBUSB1
    // discover devices
    HandleXum1541->devh = NULL;
    leastserial = MAX_ALLOWED_XUM1541_SERIALNUM + 1;

    cnt = usb.get_device_list(HandleXum1541->ctx, &list);
    if (cnt < 0)
    {
        xum1541_dbg(0, "enumeration error: %s", usb.error_name((int)cnt));
        return -1;
    }

    for (i = 0; i < cnt; i++)
    {
        libusb_device *device = list[i];
        if (LIBUSB_SUCCESS != usb.get_device_descriptor(device, &descriptor))
            continue;

        xum1541_dbg(1, "device %04x:%04x", descriptor.idVendor, descriptor.idProduct);

        // First, find our vendor and product id
        if (descriptor.idVendor != XUM1541_VID || descriptor.idProduct != XUM1541_PID)
            continue;

        xum1541_dbg(0, "found xu/xum1541 version %04x on bus %d, device %d",
            descriptor.bcdDevice, usb.get_bus_number(device),
            usb.get_device_address(device));

        err = usb.open(device, &found.devh);
        if (LIBUSB_SUCCESS != err) {
            fprintf(stderr, "error: Cannot open USB device: %s\n",
               usb.error_name(err));
            continue;
        }

        // Get device product name and try to match against "xum1541".
        // If no match, it could be an xum1541 so don't report an error.
        len = usb.get_string_descriptor_ascii(found.devh, descriptor.iProduct,
            string, sizeof(string) - 1);
        if (len < 0) {
            xum1541_cleanup(&found, "error: cannot query product name: %s\n",
                usb.error_name(len));
            continue;
        }

        string[len] = '\0';
        if (len < prodLen || strstr((char *)string, xumProduct) == NULL) {
            xum1541_cleanup(&found, NULL);
            continue;
        }
        xum1541_dbg(0, "xum1541 name: %s", string);

        len = usb.get_string_descriptor_ascii(found.devh, descriptor.iSerialNumber,
            string, sizeof(string) - 1);
        if (len < 0 && PortNumber != 0) {
            // we need the serial number, when PortNumber is not 0
            xum1541_cleanup(&found, "error: cannot query serial number: %s\n",
            usb.error_name(len));
            continue;
        }

        serialnum = 0;
        if (len > 0 && len <= 3) {
            string[len] = '\0';
            serialnum = atoi((char *)string);
        }

        if (PortNumber == serialnum) {
            xum1541_dbg(0, "xum1541 serial number: %3u", serialnum);
            HandleXum1541->devh = found.devh;
            break;
        }

        // keep in mind the handle, if the device's
        // serial number is less than previous ones
        if(serialnum < leastserial) {
            leastserial = serialnum;
            preferredDefaultHandle = device;
        }
        xum1541_cleanup(&found, NULL);
    }

    // if no default device was found because only specific devices were present,
    // determine the default device from the specific ones and open it
    if (HandleXum1541->devh == NULL && preferredDefaultHandle != NULL) {
        err = usb.open(preferredDefaultHandle, &HandleXum1541->devh);
        if (LIBUSB_SUCCESS != err)
            fprintf(stderr, "error: Cannot open USB device: %s\n", usb.error_name(err));
    }

    usb.free_device_list(list, 1);
#endif

    return 0;
}

// Check for a firmware version compatible with this plugin
static int
xum1541_check_version(int version)
{
    xum1541_dbg(0, "firmware version %d, library version %d", version,
        XUM1541_VERSION);
    if (version < XUM1541_MINIMUM_COMPATIBLE_VERSION) {
        fprintf(stderr, "xum1541 firmware version too low (%d < %d)\n",
            version, XUM1541_MINIMUM_COMPATIBLE_VERSION);
        fprintf(stderr, "please update your xum1541 firmware\n");
        return -1;
    } else if (version > XUM1541_VERSION) {
        fprintf(stderr, "xum1541 firmware version too high (%d > %d)\n",
            version, XUM1541_VERSION);
        fprintf(stderr, "please update your OpenCBM plugin\n");
        return -1;
    }
    return 0;
}

/*! \brief Query unique identifier for the xum1541 device
  This function tries to find an unique identifier for the xum1541 device.

  \param PortNumber
   The device's serial number to search for also. It is not considered, if set to 0.

  \return
    0 on success, -1 on error. On error, the handle is cleaned up if it
    was already active.

  \remark
    On success, xum1541_handle contains a valid handle to the xum1541 device.
    In this case, the device configuration has been set and the interface
    been claimed. xum1541_close() should be called when the user is done
    with it.
*/
const char *
xum1541_device_path(int PortNumber)
{
#define XUM1541_PREFIX "libusb/xum1541:"

    struct opencbm_usb_handle HandleXum1541;
    static char dev_path[sizeof(XUM1541_PREFIX) + 3 + 1 + 3 + 1];

    HandleXum1541.devh = NULL;

#if HAVE_LIBUSB1
    usb.init(&HandleXum1541.ctx);
#endif

    arch_snprintf(dev_path, sizeof(dev_path), XUM1541_PREFIX);

    if (xum1541_enumerate(&HandleXum1541, PortNumber) < 0) {
        return NULL;
    }


    if (HandleXum1541.devh != NULL) {
#if HAVE_LIBUSB0
        struct usb_device * dev = usb.device(HandleXum1541.devh);
        if (dev != NULL) {
            strcpy(dev_path, dev->filename);
        }
#elif HAVE_LIBUSB1
        arch_snprintf(dev_path, sizeof(dev_path), XUM1541_PREFIX "%d/%d",
            usb.get_bus_number(usb.get_device(HandleXum1541.devh)),
            usb.get_device_address(usb.get_device(HandleXum1541.devh)));
#endif
        xum1541_close(&HandleXum1541);
    } else {
        fprintf(stderr, "error: no xum1541 device found\n");
    }

#if HAVE_LIBUSB1
    usb.exit(HandleXum1541.ctx);
#endif

    return dev_path;
}

static int
xum1541_clear_halt(struct opencbm_usb_handle *Xum1541Handle)
{
    int ret;

    ret = usb.clear_halt(Xum1541Handle->devh, XUM_BULK_IN_ENDPOINT | LIBUSB_ENDPOINT_IN);
    if (ret != 0) {
        fprintf(stderr, "USB clear halt request failed for in ep: %s\n",
            usb.error_name(ret));
        return -1;
    }
    ret = usb.clear_halt(Xum1541Handle->devh, XUM_BULK_OUT_ENDPOINT);
    if (ret != 0) {
        fprintf(stderr, "USB clear halt request failed for out ep: %s\n",
            usb.error_name(ret));
        return -1;
    }

#if HAVE_LIBUSB0

#ifdef __APPLE__
    /*
     * The Darwin libusb implementation calls ClearPipeStall() in
     * usb_clear_halt(). While that clears the host data toggle and resets
     * its endpoint, it does not send the CLEAR_FEATURE(halt) control
     * request to the device. The ClearPipeStallBothEnds() function does
     * do this.
     *
     * We manually send this control request here on Mac systems.
     */
    ret = usb.control_msg(Xum1541Handle->devh, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE,
        0, XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN, NULL, 0, USB_TIMEOUT);
    if (ret != 0) {
        fprintf(stderr, "USB clear control req failed for in ep: %s\n",
            usb.strerror());
        return -1;
    }
    ret = usb.control_msg(Xum1541Handle->devh, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE,
        0, XUM_BULK_OUT_ENDPOINT, NULL, 0, USB_TIMEOUT);
    if (ret != 0) {
        fprintf(stderr, "USB clear control req failed for out ep: %s\n",
            usb.strerror());
        return -1;
    }
#endif // __APPLE__

#endif

    return 0;
}

/*! \brief Initialize the xum1541 device
  This function tries to find and identify the xum1541 device.

  \param HandleXum1541
   Pointer to a XUM1541_HANDLE which will contain the file handle of the USB device.

  \param PortNumber
   The device's serial number to search for also. It is not considered, if set to 0.

  \return
    0 on success, -1 on error. On error, the handle is cleaned up if it
    was already active.

  \remark
    On success, xum1541_handle contains a valid handle to the xum1541 device.
    In this case, the device configuration has been set and the interface
    been claimed. xum1541_close() should be called when the user is done
    with it.
*/
int
xum1541_init(struct opencbm_usb_handle **HandleXum1541_p, int PortNumber)
{
    struct opencbm_usb_handle *HandleXum1541;
    unsigned char devInfo[XUM_DEVINFO_SIZE], devStatus;
    int len, ret;
    int interface_claimed = 0;
    int success = 0;

    if (HandleXum1541_p == NULL) {
        perror("xum1541_init: HandleXum1541_p is NULL");
        return -1;
    }

    // Place after "opencbm_usb_handle" allocation:
    /*uh->*/DeviceDriveMode = DeviceDriveMode_Uninit;

    *HandleXum1541_p = HandleXum1541 = malloc(sizeof(struct opencbm_usb_handle));
    if (HandleXum1541 == NULL) {
        perror("xum1541_init: malloc failed");
        return -1;
    }
    HandleXum1541->devh = NULL;

#if HAVE_LIBUSB1
    usb.init(&HandleXum1541->ctx);
#endif

    if (xum1541_enumerate(HandleXum1541, PortNumber) < 0) {
        fprintf(stderr, "error: no xum1541 device found\n");
        return -1;
    }

    if (HandleXum1541->devh == NULL) {
        fprintf(stderr, "error: no xum1541 device found\n");
#if HAVE_LIBUSB1
        usb.exit(HandleXum1541->ctx);
#endif
        free(HandleXum1541);
        HandleXum1541 = NULL;
        return -1;
    }

    do {
        // Select first and only device configuration.
        ret = usb.set_configuration(HandleXum1541->devh, 1);
        if (ret != LIBUSB_SUCCESS) {
            xum1541_cleanup(HandleXum1541, "USB error: %s\n", usb.error_name(ret));
            break;
        }

        /*
         * Get exclusive access to interface 0.
         * After this point, do cleanup using xum1541_close() instead of
         * xum1541_cleanup().
         */
        ret = usb.claim_interface(HandleXum1541->devh, 0);
        if (ret != LIBUSB_SUCCESS) {
            xum1541_cleanup(HandleXum1541, "USB error: %s\n", usb.error_name(ret));
            break;
        }

#if HAVE_LIBUSB1
        interface_claimed = 1;
#endif

        // Check the basic device info message for firmware version
        memset(devInfo, 0, sizeof(devInfo));
#if HAVE_LIBUSB0
        len = usb.control_msg(HandleXum1541->devh, USB_TYPE_CLASS | USB_ENDPOINT_IN,
            XUM1541_INIT, 0, 0, (char*)devInfo, sizeof(devInfo), USB_TIMEOUT);
#elif HAVE_LIBUSB1
        len = usb.control_transfer(HandleXum1541->devh, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
            XUM1541_INIT, 0, 0, devInfo, sizeof(devInfo), USB_TIMEOUT);
#endif
        if (len < 2) {
            fprintf(stderr, "USB request for XUM1541 info failed: %s\n",
                usb.error_name(len));
            break;
        }
        if (xum1541_check_version(devInfo[0]) != 0) {
            break;
        }
        if (len >= 4) {
            xum1541_dbg(0, "device capabilities %02x status %02x",
                devInfo[1], devInfo[2]);
        }

        // Check for the xum1541's current status. (Not the drive.)
        devStatus = devInfo[2];
        if ((devStatus & XUM1541_DOING_RESET) != 0) {
            fprintf(stderr, "previous command was interrupted, resetting\n");
            // Clear the stalls on both endpoints
            if (xum1541_clear_halt(HandleXum1541) < 0) {
                break;
            }
        }

        //  Enable disk or tape mode.
        if (devInfo[1] & XUM1541_CAP_TAP) {
            if (devInfo[2] & XUM1541_TAPE_PRESENT) {
                /*uh->*/DeviceDriveMode = DeviceDriveMode_Tape;
                xum1541_dbg(1, "[xum1541_init] Tape supported, tape mode entered.");
            }
            else
            {
                /*uh->*/DeviceDriveMode = DeviceDriveMode_Disk;
                xum1541_dbg(1, "[xum1541_init] Tape supported, disk mode entered.");
            }
        }
        else
        {
            DeviceDriveMode = (unsigned char) DeviceDriveMode_NoTapeSupport;
            xum1541_dbg(1, "[xum1541_init] No tape support.");
        }

        success = 1;

    } while (0);

    /* error cleanup */
    if (!success) {
        if (interface_claimed) {
            usb.release_interface(HandleXum1541->devh, 0);
        }

        xum1541_close(HandleXum1541);
    }

    return success ? 0 : -1;
}
/*! \brief close the xum1541 device

 \param HandleXum1541
   Pointer to a XUM1541_HANDLE which will contain the file handle of the USB device.

 \remark
    This function releases the interface and closes the xum1541 handle.
*/
void
xum1541_close(struct opencbm_usb_handle *HandleXum1541)
{
    int ret;

    xum1541_dbg(0, "Closing USB link");

#if HAVE_LIBUSB0
    ret = usb.control_msg(HandleXum1541->devh, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        XUM1541_SHUTDOWN, 0, 0, NULL, 0, 1000);
#elif HAVE_LIBUSB1
    ret = usb.control_transfer(HandleXum1541->devh, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
        XUM1541_SHUTDOWN, 0, 0, NULL, 0, 1000);
#endif
    if (ret < 0) {
        fprintf(stderr,
            "USB request for XUM1541 close failed, continuing: %s\n",
            usb.error_name(ret));
    }
    ret = usb.release_interface(HandleXum1541->devh, 0);
    if (ret != LIBUSB_SUCCESS)
        fprintf(stderr, "USB release intf error: %s\n", usb.error_name(ret));

    ret = usb.set_configuration(HandleXum1541->devh, -1);
    if (ret != LIBUSB_SUCCESS) {
        fprintf(stderr, "USB deconfig device error: %d %s\n", ret, usb.error_name(ret));
    }
#if HAVE_LIBUSB0
    if (usb.close(HandleXum1541->devh) != LIBUSB_SUCCESS)
        fprintf(stderr, "USB close error: %s\n", usb.strerror());
#elif HAVE_LIBUSB1
    usb.close(HandleXum1541->devh);
    usb.exit(HandleXum1541->ctx);
#endif

    free(HandleXum1541);
}

/*! \brief  Handle synchronous USB control messages, e.g. for RESET.
    xum1541_ioctl() is used for bulk messages.

 \param HandleXum1541
   A XUM1541_HANDLE which contains the file handle of the USB device.

 \param cmd
   The command to run.

 \return
   Returns the value the USB device sent back.
*/
int
xum1541_control_msg(struct opencbm_usb_handle *HandleXum1541, unsigned int cmd)
{
    int nBytes;

    xum1541_dbg(1, "control msg %d", cmd);

#if HAVE_LIBUSB0
    nBytes = usb.control_msg(HandleXum1541->devh, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        cmd, 0, 0, NULL, 0, USB_TIMEOUT);
#elif HAVE_LIBUSB1
    nBytes = usb.control_transfer(HandleXum1541->devh, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
        (uint8_t) cmd, 0, 0, NULL, 0, USB_TIMEOUT);
#endif
    if (nBytes < 0) {
        fprintf(stderr, "USB error in xum1541_control_msg: %s\n",
            usb.error_name(nBytes));
        exit(-1); /** \todo WHY? */
    }

    return nBytes;
}

static int
xum1541_wait_status(struct opencbm_usb_handle *HandleXum1541)
{
    int nBytes, deviceBusy, ret=0;
    unsigned char statusBuf[XUM_STATUSBUF_SIZE];

    xum1541_dbg(2, "xum1541_wait_status checking for status");
    deviceBusy = 1;
    while (deviceBusy) {
#if HAVE_LIBUSB0
        nBytes = usb.bulk_read(HandleXum1541->devh,
            XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char*)statusBuf, XUM_STATUSBUF_SIZE, LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
        nBytes = 0;
        ret = usb.bulk_transfer(HandleXum1541->devh,
            XUM_BULK_IN_ENDPOINT | LIBUSB_ENDPOINT_IN,
            statusBuf, XUM_STATUSBUF_SIZE, &nBytes, LIBUSB_NO_TIMEOUT);
#endif
        if (nBytes == XUM_STATUSBUF_SIZE) {
            switch (XUM_GET_STATUS(statusBuf)) {
            case XUM1541_IO_BUSY:
                xum1541_dbg(2, "device busy, waiting");
                break;
            case XUM1541_IO_ERROR:
                fprintf(stderr, "device reports error\n");
                /* FALLTHROUGH */
            case XUM1541_IO_READY:
                deviceBusy = 0;
                break;
            default:
                fprintf(stderr, "unknown status value: %d\n",
                    XUM_GET_STATUS(statusBuf));
                exit(-1);
            }
        } else {
            fprintf(stderr, "USB error in xum1541_wait_status: %s\n",
                usb.error_name(ret));
            exit(-1); /** \todo WHY? */
        }
    }

    // Once we have a valid response (done ok), get extended status
    if (XUM_GET_STATUS(statusBuf) == XUM1541_IO_READY)
        ret = XUM_GET_STATUS_VAL(statusBuf);
    else
        ret = -1;

    xum1541_dbg(2, "return val = %x", ret);
    return ret;
}

// Macro to enforce disk/tape mode.
// Checks if xum1541_ioctl/xum1541_read/xum1541_write command is allowed in currently set disk/tape mode.
#define RefuseToWorkInWrongMode \
    {                                                                                                    \
        if (/*uh->*/DeviceDriveMode == DeviceDriveMode_Uninit)                                               \
        {                                                                                                \
            xum1541_dbg(1, "[RefuseToWorkInWrongMode] cmd blocked - No disk or tape mode set.");         \
            return XUM1541_Error_NoDiskTapeMode;                                                         \
        }                                                                                                \
                                                                                                         \
        if (isTapeCmd)                                                                                   \
        {                                                                                                \
            if (/*uh->*/DeviceDriveMode == DeviceDriveMode_NoTapeSupport)                                \
            {                                                                                            \
                xum1541_dbg(1, "[RefuseToWorkInWrongMode] cmd blocked - Firmware has no tape support."); \
                return XUM1541_Error_NoTapeSupport;                                                      \
            }                                                                                            \
                                                                                                         \
            if (/*uh->*/DeviceDriveMode == DeviceDriveMode_Disk)                                             \
            {                                                                                            \
                xum1541_dbg(1, "[RefuseToWorkInWrongMode] cmd blocked - Tape cmd in disk mode.");        \
                return XUM1541_Error_TapeCmdInDiskMode;                                                  \
            }                                                                                            \
        }                                                                                                \
        else /*isDiskCmd*/                                                                               \
        {                                                                                                \
            if (/*uh->*/DeviceDriveMode == DeviceDriveMode_Tape)                                             \
            {                                                                                            \
                xum1541_dbg(1, "[RefuseToWorkInWrongMode] cmd blocked - Disk cmd in tape mode.");        \
                return XUM1541_Error_DiskCmdInTapeMode;                                                  \
            }                                                                                            \
        }                                                                                                \
    }

/*! \brief Perform an ioctl on the xum1541, which is any command other than
    read/write or special device management commands such as INIT and RESET.

 \param HandleXum1541
   A XUM1541_HANDLE which contains the file handle of the USB device.

 \param cmd
   The command to run.

 \param addr
   The IEC device to use or 0 if not needed.

 \param secaddr
   The IEC secondary address to use or 0 if not needed.

 \return
   Returns the device status byte, which is 0 if the command does not
   have a return value. For some commands, the status byte gives
   info from the device such as the active IEC lines.
*/
int
xum1541_ioctl(struct opencbm_usb_handle *HandleXum1541, unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
    int nBytes, ret = 0;
    unsigned char cmdBuf[XUM_CMDBUF_SIZE];
    BOOL isTapeCmd = ((XUM1541_TAP_MOTOR_ON <= cmd) && (cmd <= XUM1541_TAP_MOTOR_OFF));

    xum1541_dbg(1, "ioctl %d for device %d, sub %d", cmd, addr, secaddr);

    RefuseToWorkInWrongMode; // Check if command allowed in current disk/tape mode.

    cmdBuf[0] = (unsigned char)cmd;
    cmdBuf[1] = (unsigned char)addr;
    cmdBuf[2] = (unsigned char)secaddr;
    cmdBuf[3] = 0;

    // Send the 4-byte command block
#if HAVE_LIBUSB0
    nBytes = usb.bulk_write(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        (char *)cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
    ret = usb.bulk_transfer(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | LIBUSB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), &nBytes, LIBUSB_NO_TIMEOUT);
#endif

#if HAVE_LIBUSB0
    if (nBytes < 0) {
#elif HAVE_LIBUSB1
    if (ret != LIBUSB_SUCCESS) {
#endif
        fprintf(stderr, "USB error in xum1541_ioctl cmd: %s\n",
            usb.error_name(ret));
        exit(-1);
    }

    // If we have a valid response, return extended status
    ret = xum1541_wait_status(HandleXum1541);
    xum1541_dbg(2, "return val = %x", ret);
    return ret;
}

/*! \brief Send tape operations abort command to the xum1541 device

 \param HandleXum1541
   A XUM1541_HANDLE which contains the file handle of the USB device.

 \return
   Returns the value the USB device sent back.
*/
int
xum1541_tap_break(struct opencbm_usb_handle *HandleXum1541)
{
    BOOL isTapeCmd = TRUE;
    RefuseToWorkInWrongMode; // Check if command allowed in current disk/tape mode.

    xum1541_dbg(1, "[xum1541_tap_break] Sending tape break command.");

    return xum1541_control_msg(HandleXum1541, XUM1541_TAP_BREAK);
}

/*! \brief Write data to the xum1541 device

 \param HandleXum1541
   A XUM1541_HANDLE which contains the file handle of the USB device.

 \param mode
    Drive protocol to use to read the data from the device (e.g,
    XUM1541_CBM is normal IEC wire protocol).

 \param data
    Pointer to buffer which contains the data to be written to the xum1541

 \param size
    The number of bytes to write to the xum1541

 \return
    The number of bytes actually written, 0 on device error. If there is a
    fatal error, returns -1.
*/
int
xum1541_write(struct opencbm_usb_handle *HandleXum1541, unsigned char modeFlags, const unsigned char *data, size_t size)
{
    int wr, mode, ret=0;
    size_t bytesWritten, bytes2write;
    unsigned char cmdBuf[XUM_CMDBUF_SIZE];
    BOOL isTapeCmd = ((modeFlags == XUM1541_TAP) || (modeFlags == XUM1541_TAP_CONFIG));

    mode = modeFlags & 0xf0;
    xum1541_dbg(1, "write %d %d bytes from address %p flags %x",
        mode, size, data, modeFlags & 0x0f);

    RefuseToWorkInWrongMode; // Check if command allowed in current disk/tape mode.

    // Send the write command
    cmdBuf[0] = XUM1541_WRITE;
    cmdBuf[1] = modeFlags;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
#if HAVE_LIBUSB0
    wr = usb.bulk_write(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        (char *)cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
    ret = usb.bulk_transfer(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | LIBUSB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), &wr, LIBUSB_NO_TIMEOUT);
#endif

#if HAVE_LIBUSB0
    if (wr < 0) {
#elif HAVE_LIBUSB1
    if (ret != LIBUSB_SUCCESS) {
#endif
        fprintf(stderr, "USB error in write cmd: %s\n",
            usb.error_name(ret));
        return -1;
    }

    bytesWritten = 0;
    while (bytesWritten < size) {
        bytes2write = size - bytesWritten;
        if (bytes2write > XUM_MAX_XFER_SIZE)
            bytes2write = XUM_MAX_XFER_SIZE;
#if HAVE_LIBUSB0
        wr = usb.bulk_write(HandleXum1541->devh,
            XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
            (char *)data, bytes2write, LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
        wr = 0;
        ret = usb.bulk_transfer(HandleXum1541->devh,
            XUM_BULK_OUT_ENDPOINT | LIBUSB_ENDPOINT_OUT,
            (unsigned char *)data, bytes2write, &wr, LIBUSB_NO_TIMEOUT);
#endif

#if HAVE_LIBUSB0
        if (wr < 0) {
#elif HAVE_LIBUSB1
        if (ret != LIBUSB_SUCCESS) {
#endif
            if (isTapeCmd)
            {
#if HAVE_LIBUSB0
                if (usb.resetep(HandleXum1541->devh, XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT) < 0) {
#elif HAVE_LIBUSB1
                ret = usb.clear_halt(HandleXum1541->devh, XUM_BULK_OUT_ENDPOINT | LIBUSB_ENDPOINT_OUT);
                if (ret < 0) {
#endif
                    fprintf(stderr, "USB reset ep request failed for out ep (tape stall): %s\n", usb.error_name(ret));
                }
#if HAVE_LIBUSB0
                if (usb.control_msg(HandleXum1541->devh, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE, 0, XUM_BULK_OUT_ENDPOINT, NULL, 0, USB_TIMEOUT) < 0) {
#elif HAVE_LIBUSB1
                ret = usb.control_transfer(HandleXum1541->devh, LIBUSB_RECIPIENT_ENDPOINT, LIBUSB_REQUEST_CLEAR_FEATURE, 0, XUM_BULK_OUT_ENDPOINT, NULL, 0, USB_TIMEOUT); /** \todo */
                if (ret < 0) {

#endif
                    fprintf(stderr, "USB error in xum1541_control_msg (tape stall): %s\n", usb.error_name(ret));
                }
                return bytesWritten;
            }
            fprintf(stderr, "USB error in write data: %s\n",
                usb.error_name(ret));
            return -1;
        }

        xum1541_print_data(2, "wrote", data, wr);

        data += wr;
        bytesWritten += wr;

        /*
         * If we wrote less than we requested (or 0), the transfer is done
         * even if we had more data to write still.
         */
        if (wr < (int)bytes2write)
            break;
    }

    // If this is the CBM protocol, wait for the status message.
    if (mode == XUM1541_CBM) {
        ret = xum1541_wait_status(HandleXum1541);
        if (ret >= 0)
            xum1541_dbg(2, "wait done, extended status %d", ret);
        else
            xum1541_dbg(2, "wait done with error");
        bytesWritten = ret;
    }

    xum1541_dbg(2, "write done, got %d bytes", bytesWritten);
    return bytesWritten;
}

/*! \brief Wrapper for xum1541_write() forcing xum1541_wait_status(), with additional parameters:

 \param Status
   The return status.

 \param BytesWritten
   The number of bytes written.

 \return
     1 : Finished successfully.
    <0 : Fatal error.
*/

int
xum1541_write_ext(struct opencbm_usb_handle *HandleXum1541, unsigned char modeFlags, const unsigned char *data, size_t size, int *Status, int *BytesWritten)
{
    xum1541_dbg(1, "[xum1541_write_ext]");
    *BytesWritten = xum1541_write(HandleXum1541, modeFlags, data, size);
    if (*BytesWritten < 0)
        return *BytesWritten;
    xum1541_dbg(2, "[xum1541_write_ext] BytesWritten = %d", *BytesWritten);
    *Status = xum1541_wait_status(HandleXum1541);
    xum1541_dbg(2, "[xum1541_write_ext] Status = %d", *Status);
    return 1;
}

/*! \brief Wrapper for xum1541_read() forcing xum1541_wait_status(), with additional parameters:

 \param Status
   The return status.

 \param BytesRead
   The number of bytes read.

 \return
     1 : Finished successfully.
    <0 : Fatal error.
*/

int
xum1541_read_ext(struct opencbm_usb_handle *HandleXum1541, unsigned char mode, unsigned char *data, size_t size, int *Status, int *BytesRead)
{
    xum1541_dbg(1, "[xum1541_read_ext]");
    *BytesRead = xum1541_read(HandleXum1541, mode, data, size);
    if (*BytesRead < 0)
        return *BytesRead;
    xum1541_dbg(2, "[xum1541_read_ext] BytesRead = %d", *BytesRead);
    *Status = xum1541_wait_status(HandleXum1541);
    xum1541_dbg(2, "[xum1541_read_ext] Status = %d", *Status);
    return 1;
}

/*! \brief Read data from the xum1541 device

 \param HandleXum1541
   A XUM1541_HANDLE which contains the file handle of the USB device.

 \param mode
    Drive protocol to use to read the data from the device (e.g,
    XUM1541_CBM is normal IEC wire protocol).

 \param data
    Pointer to a buffer which will contain the data read from the xum1541

 \param size
    The number of bytes to read from the xum1541

 \return
    The number of bytes actually read, 0 on device error. If there is a
    fatal error, returns -1.
*/
int
xum1541_read(struct opencbm_usb_handle *HandleXum1541, unsigned char mode, unsigned char *data, size_t size)
{
    int rd, ret;
    size_t bytesRead, bytes2read;
    unsigned char cmdBuf[XUM_CMDBUF_SIZE];
    BOOL isTapeCmd = ((mode == XUM1541_TAP) || (mode == XUM1541_TAP_CONFIG));

    xum1541_dbg(1, "read %d %d bytes to address %p",
               mode, size, data);

    RefuseToWorkInWrongMode; // Check if command allowed in current disk/tape mode.

    // Send the read command
    cmdBuf[0] = XUM1541_READ;
    cmdBuf[1] = mode;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
#if HAVE_LIBUSB0
    ret = 0;
    rd = usb.bulk_write(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        (char *)cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
    ret = usb.bulk_transfer(HandleXum1541->devh,
        XUM_BULK_OUT_ENDPOINT | LIBUSB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), &rd, LIBUSB_NO_TIMEOUT);
#endif
#if HAVE_LIBUSB0
    if (rd < 0) {
#elif HAVE_LIBUSB1
    if (ret != LIBUSB_SUCCESS) {
#endif
        fprintf(stderr, "USB error in read cmd: %s\n",
            usb.error_name(ret));
        return -1;
    }

    // Read the actual data now that it's ready.
    bytesRead = 0;
    while (bytesRead < size) {
        bytes2read = size - bytesRead;
        if (bytes2read > XUM_MAX_XFER_SIZE)
            bytes2read = XUM_MAX_XFER_SIZE;
#if HAVE_LIBUSB0
        rd = usb.bulk_read(HandleXum1541->devh,
            XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char *)data, bytes2read, LIBUSB_NO_TIMEOUT);
#elif HAVE_LIBUSB1
        ret = usb.bulk_transfer(HandleXum1541->devh,
            XUM_BULK_IN_ENDPOINT | LIBUSB_ENDPOINT_IN,
            data, bytes2read, &rd, LIBUSB_NO_TIMEOUT);
#endif
#if HAVE_LIBUSB0
        if (rd < 0) {
#elif HAVE_LIBUSB1
        if (ret != LIBUSB_SUCCESS) {
#endif
            fprintf(stderr, "USB error in read data(%p, %d): %s\n",
               data, (int)size, usb.error_name(ret));
            return -1;
        }

        xum1541_print_data(2, "read", data, rd);

        data += rd;
        bytesRead += rd;

        /*
         * If we read less than we requested (or 0), the transfer is done
         * even if we had more data to read still.
         */
        if (rd < (int)bytes2read)
            break;
    }

    xum1541_dbg(2, "read done, got %d bytes", bytesRead);
    return bytesRead;
}
