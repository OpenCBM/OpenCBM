/*
 * xum1541 driver for bulk and control messages
 * Copyright 2009-2010 Nate Lawson <nate@root.org>
 * Copyright 2010 Spiro Trikaliotis
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
** \version $Id: xum1541.c,v 1.10 2010-08-09 19:35:34 wmsr Exp $ \n
** \n
** \brief libusb-based xum1541 access routines
****************************************************************/

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
/* not static anymore
static usb_dev_handle *xum1541_handle = NULL; /*!< \internal \brief handle to the xum1541 device */

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

/*! \internal \brief Get a char* string from the device's Unicode descriptors
    Some data will be lost in this conversion, but we are ok with that.

 \param dev
    libusb device handle

 \param index
    Descriptor string index

 \param langid
    Language code

 \param buf
    Where to store the string. The result is nul-terminated.

 \param buflen
    Length of the output buffer.

 \return
    Returns the length of the string read or 0 on error.
*/
static int
usbGetStringAscii(usb_dev_handle *dev, int index, int langid,
    char *buf, int buflen)
{
    char buffer[256];
    int rval, i;

    rval = usb.control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
        (USB_DT_STRING << 8) + index, langid,
        buffer, sizeof(buffer), 1000);
    if (rval < 0)
        return rval;

    if (buffer[1] != USB_DT_STRING)
        return 0;
    if ((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];

    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for (i = 1; i < rval; i++) {
        if (i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if (buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i - 1;
}

// Cleanup after a failure
static void
xum1541_cleanup(XUM1541_HANDLE *HandleXum1541, char *msg, ...)
{
    va_list args;

    if (msg != NULL) {
        va_start(args, msg);
        fprintf(stderr, msg, args);
        va_end(args);
    }
    if (*HandleXum1541 != NULL)
        usb.close(*HandleXum1541);
    *HandleXum1541 = NULL;
}

// Check for a firmware version compatible with this plugin
static int
xum1541_check_version(int version)
{
    xum1541_dbg(0, "firmware version %d, library version %d", version,
        XUM1541_VERSION);
    if (version < XUM1541_VERSION) {
        fprintf(stderr, "xum1541 firmware version too low (%d < %d)\n",
            version, XUM1541_VERSION);
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

/*! \brief Initialize the xum1541 device
  This function tries to find and identify the xum1541 device.

 \param HandleXum1541  
   Pointer to a XUM1541_HANDLE which will contain the file handle of the USB device.

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
xum1541_init(XUM1541_HANDLE *HandleXum1541)
{
    static char xumProduct[] = "xum1541"; // Start of USB product string id
    static int prodLen = sizeof(xumProduct) - 1;
    struct usb_bus *bus;
    struct usb_device *dev;
    char string[256];
    unsigned char devInfo[XUM_DEVINFO_SIZE], devStatus;
    int len;

    xum1541_dbg(0, "scanning usb ...");

    usb.init();
    usb.find_busses();
    usb.find_devices();

    /* usb.find_devices sets errno if some devices don't reply 100% correct. */
    /* make lib ignore this as this has nothing to do with our device */
    errno = 0;

    for (bus = usb.get_busses(); !*HandleXum1541 && bus; bus = bus->next) {
        xum1541_dbg(1, "scanning bus %s", bus->dirname);
        for (dev = bus->devices; !*HandleXum1541 && dev; dev = dev->next) {
            xum1541_dbg(1, "device %04x:%04x at %s",
                dev->descriptor.idVendor, dev->descriptor.idProduct,
                dev->filename);

            // First, find our vendor and product id
            if (dev->descriptor.idVendor != XUM1541_VID &&
                dev->descriptor.idProduct != XUM1541_PID)
                continue;

            xum1541_dbg(0, "found xu/xum1541 version %04x on bus %s, device %s",
                dev->descriptor.bcdDevice, bus->dirname, dev->filename);
            if ((*HandleXum1541 = usb.open(dev)) == NULL) {
                fprintf(stderr, "error: Cannot open USB device: %s\n",
                    usb.strerror());
                continue;
            }

            // Get device product name and try to match against "xum1541".
            // If no match, it could be an xum1541 so don't report an error.
            len = usbGetStringAscii(*HandleXum1541, dev->descriptor.iProduct,
                0x0409, string, sizeof(string) - 1);
            if (len < 0) {
                xum1541_cleanup(HandleXum1541, "error: cannot query product name: %s\n",
                    usb.strerror());
                continue;
            }
            string[len] = '\0';
            if (len < prodLen || strstr(string, xumProduct) == NULL) {
                xum1541_cleanup(HandleXum1541, NULL);
                continue;
            }
            xum1541_dbg(0, "xum1541 name: %s", string);
            goto done;
        }
    }

done:
    if (*HandleXum1541 == NULL) {
        fprintf(stderr, "error: no xum1541 device found\n");
        return -1;
    }

    // Select first and only device configuration.
    if (usb.set_configuration(*HandleXum1541, 1) != 0) {
        xum1541_cleanup(HandleXum1541, "USB error: %s\n", usb.strerror());
        return -1;
    }

    /*
     * Get exclusive access to interface 0.
     * After this point, do cleanup using xum1541_close() instead of
     * xum1541_cleanup().
     */
    if (usb.claim_interface(*HandleXum1541, 0) != 0) {
        xum1541_cleanup(HandleXum1541, "USB error: %s\n", usb.strerror());
        return -1;
    }

    // Check the basic device info message for firmware version
    memset(devInfo, 0, sizeof(devInfo));
    len = usb.control_msg(*HandleXum1541, USB_TYPE_CLASS | USB_ENDPOINT_IN,
        XUM1541_INIT, 0, 0, (char*)devInfo, sizeof(devInfo), USB_TIMEOUT);
    if (len < 2) {
        fprintf(stderr, "USB request for XUM1541 info failed: %s\n",
            usb.strerror());
        xum1541_close(HandleXum1541);
        return -1;
    }
    if (xum1541_check_version(devInfo[0]) != 0) {
        xum1541_close(HandleXum1541);
        return -1;
    }
    if (len >= 4) {
        xum1541_dbg(0, "device capabilities %02x status %02x",
            devInfo[1], devInfo[2]);
    }

    // Check for the xum1541's current status. (Not the drive.)
    devStatus = devInfo[2];
    if ((devStatus & XUM1541_DOING_RESET) != 0) {
        fprintf(stderr, "previous command was interrupted, resetting\n");
        usb.clear_halt(*HandleXum1541, XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN);
        usb.clear_halt(*HandleXum1541, XUM_BULK_OUT_ENDPOINT);
    }

    return 0;
}

/*! \brief close the xum1541 device

 \param HandleXum1541  
   Pointer to a XUM1541_HANDLE which will contain the file handle of the USB device.

 \remark
    This function releases the interface and closes the xum1541 handle.
*/
void
xum1541_close(XUM1541_HANDLE *HandleXum1541)
{
    int ret;

    xum1541_dbg(0, "Closing USB link");

    ret = usb.control_msg(*HandleXum1541, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        XUM1541_SHUTDOWN, 0, 0, NULL, 0, 1000);
    if (ret < 0) {
        fprintf(stderr,
            "USB request for XUM1541 close failed, continuing: %s\n",
            usb.strerror());
    }
    if (usb.release_interface(*HandleXum1541, 0) != 0)
        fprintf(stderr, "USB release intf error: %s\n", usb.strerror());

    if (usb.close(*HandleXum1541) != 0)
        fprintf(stderr, "USB close error: %s\n", usb.strerror());
    *HandleXum1541 = NULL;
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
xum1541_control_msg(XUM1541_HANDLE HandleXum1541, unsigned int cmd)
{
    int nBytes;

    xum1541_dbg(1, "control msg %d", cmd);

    nBytes = usb.control_msg(HandleXum1541, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        cmd, 0, 0, NULL, 0, USB_TIMEOUT);
    if (nBytes < 0) {
        fprintf(stderr, "USB error in xum1541_control_msg: %s\n",
            usb.strerror());
        exit(-1);
    }

    return nBytes;
}

static int
xum1541_wait_status(XUM1541_HANDLE HandleXum1541)
{
    int nBytes, deviceBusy, ret;
    unsigned char statusBuf[XUM_STATUSBUF_SIZE];

    xum1541_dbg(2, "xum1541_wait_status checking for status");
    deviceBusy = 1;
    while (deviceBusy) {
        nBytes = usb.bulk_read(HandleXum1541,
            XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char*)statusBuf, XUM_STATUSBUF_SIZE, LIBUSB_NO_TIMEOUT);
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
                usb.strerror());
            exit(-1);
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
xum1541_ioctl(XUM1541_HANDLE HandleXum1541, unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
    int nBytes, ret;
    unsigned char cmdBuf[XUM_CMDBUF_SIZE];

    xum1541_dbg(1, "ioctl %d for device %d, sub %d", cmd, addr, secaddr);
    cmdBuf[0] = (unsigned char)cmd;
    cmdBuf[1] = (unsigned char)addr;
    cmdBuf[2] = (unsigned char)secaddr;
    cmdBuf[3] = 0;

    // Send the 4-byte command block
    nBytes = usb.bulk_write(HandleXum1541,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
    if (nBytes < 0) {
        fprintf(stderr, "USB error in xum1541_ioctl cmd: %s\n",
            usb.strerror());
        exit(-1);
    }

    // If we have a valid response, return extended status
    ret = xum1541_wait_status(HandleXum1541);
    xum1541_dbg(2, "return val = %x", ret);
    return ret;
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
xum1541_write(XUM1541_HANDLE HandleXum1541, __u_char modeFlags, const __u_char *data, size_t size)
{
    int wr, mode, ret;
    size_t bytesWritten, bytes2write;
    __u_char cmdBuf[XUM_CMDBUF_SIZE];

    mode = modeFlags & 0xf0;
    xum1541_dbg(1, "write %d %d bytes from address %p flags %x",
        mode, size, data, modeFlags & 0x0f);

    // Send the write command
    cmdBuf[0] = XUM1541_WRITE;
    cmdBuf[1] = modeFlags;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
    wr = usb.bulk_write(HandleXum1541,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
    if (wr < 0) {
        fprintf(stderr, "USB error in write cmd: %s\n",
            usb.strerror());
        return -1;
    }

    bytesWritten = 0;
    while (bytesWritten < size) {
        bytes2write = size - bytesWritten;
        if (bytes2write > XUM_MAX_XFER_SIZE)
            bytes2write = XUM_MAX_XFER_SIZE;
        wr = usb.bulk_write(HandleXum1541,
            XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
            (char *)data, bytes2write, LIBUSB_NO_TIMEOUT);
        if (wr < 0) {
            fprintf(stderr, "USB error in write data: %s\n",
                usb.strerror());
            return -1;
        } else if (wr > 0)
            xum1541_dbg(2, "wrote %d bytes", wr);

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
xum1541_read(XUM1541_HANDLE HandleXum1541, __u_char mode, __u_char *data, size_t size)
{
    int rd, ret;
    size_t bytesRead, bytes2read;
    unsigned char cmdBuf[XUM_CMDBUF_SIZE];
    unsigned char statusBuf[XUM_STATUSBUF_SIZE];

    xum1541_dbg(1, "read %d %d bytes to address %p",
               mode, size, data);

    // Send the read command
    cmdBuf[0] = XUM1541_READ;
    cmdBuf[1] = mode;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
    rd = usb.bulk_write(HandleXum1541,
        XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);
    if (rd < 0) {
        fprintf(stderr, "USB error in read cmd: %s\n",
            usb.strerror());
        return -1;
    }

    // Read the actual data now that it's ready.
    bytesRead = 0;
    while (bytesRead < size) {
        bytes2read = size - bytesRead;
        if (bytes2read > XUM_MAX_XFER_SIZE)
            bytes2read = XUM_MAX_XFER_SIZE;
        rd = usb.bulk_read(HandleXum1541,
            XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char *)data, bytes2read, LIBUSB_NO_TIMEOUT);
        if (rd < 0) {
            fprintf(stderr, "USB error in read data(%p, %d): %s\n",
               data, size, usb.strerror());
            return -1;
        } else if (rd > 0)
            xum1541_dbg(2, "read %d bytes", rd);

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
