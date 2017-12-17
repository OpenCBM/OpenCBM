/*
 * xuac driver for bulk and control messages
 * Copyright 2009-2010 Nate Lawson <nate@root.org>
 * Copyright 2010 Spiro Trikaliotis
 * Copyright 2012-2017 Arnd Menge
 *
 * Incorporates some code from the xu1541 driver by:
 * Copyright 2007 Till Harbaum <till@harbaum.org>
 *
 * Incorporates content from the xum1541 driver by:
 * Copyright 2009-2010 Nate Lawson <nate@root.org>
 * Copyright 2010 Spiro Trikaliotis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*! **************************************************************
** \file lib/plugin/xuac/xuac.c \n
** \author Nate Lawson \n
** \n
** \brief libusb-based xuac access routines
****************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "opencbm.h"

#include "arch.h"
#include "dynlibusb.h"
#include "getpluginaddress.h"
#include "xuac.h"

// XXX Fix for Linux/Mac build, should be moved
#ifndef LIBUSB_PATH_MAX
#define LIBUSB_PATH_MAX 512
#endif

static int debug_level = -1; /*!< \internal \brief the debugging level for debugging output */

/*! \internal \brief Output debugging information for the xuac

 \param level
   The output level; output will only be produced if this level is less or equal the debugging level

 \param msg
   The printf() style message to be output
*/
static void
xuac_dbg(int level, char *msg, ...)
{
    va_list argp;

    /* determine debug mode if not yet known */
    if (debug_level == -1) {
        char *val = getenv("XUAC_DEBUG");
        if (val)
            debug_level = atoi(val);
    }

    if (level <= debug_level) {
        fprintf(stderr, "[XUAC] ");
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
xuac_cleanup(usb_dev_handle **hDev, char *msg, ...)
{
    va_list args;

    if (msg != NULL) {
        va_start(args, msg);
        fprintf(stderr, msg, args);
        va_end(args);
    }
    if (*hDev != NULL)
        usb.close(*hDev);
    *hDev = NULL;
}

// USB bus enumeration
static void
xuac_enumerate(usb_dev_handle **hDev, int PortNumber)
{
    static char xuacProduct[] = "xuac"; // Start of USB product string id
    static int prodLen = sizeof(xuacProduct) - 1;
    struct usb_bus *bus;
    struct usb_device *dev, *preferredDefaultHandle;
    char string[256];
    int len, serialnum, leastserial;

    if (PortNumber < 0 || PortNumber > MAX_ALLOWED_XUAC_SERIALNUM) {
        // Normalise the Portnumber for invalid values
        PortNumber = 0;
    }

    xuac_dbg(0, "scanning usb ...");

    usb.init();
    usb.find_busses();
    usb.find_devices();

    /* usb.find_devices sets errno if some devices don't reply 100% correct. */
    /* make lib ignore this as this has nothing to do with our device */
    errno = 0;

    *hDev = NULL;
    preferredDefaultHandle = NULL;
    leastserial = MAX_ALLOWED_XUAC_SERIALNUM + 1;
    for (bus = usb.get_busses(); !*hDev && bus; bus = bus->next) {
        xuac_dbg(1, "scanning bus %s", bus->dirname);
        for (dev = bus->devices; !*hDev && dev; dev = dev->next) {
            xuac_dbg(1, "device %04x:%04x at %s",
                dev->descriptor.idVendor, dev->descriptor.idProduct,
                dev->filename);

            // First, find our vendor and product id
            if (dev->descriptor.idVendor != XUAC_VID ||
                dev->descriptor.idProduct != XUAC_PID)
                continue;

            xuac_dbg(0, "found xu/xum1541/xuac version %04x on bus %s, device %s",
                dev->descriptor.bcdDevice, bus->dirname, dev->filename);
            if ((*hDev = usb.open(dev)) == NULL) {
                fprintf(stderr, "error: Cannot open USB device: %s\n",
                    usb.strerror());
                continue;
            }

            // Get device product name and try to match against "xuac".
            // If no match, it could be an xum1541 so don't report an error.
            len = usbGetStringAscii(*hDev, dev->descriptor.iProduct,
                0x0409, string, sizeof(string) - 1);
            if (len < 0) {
                xuac_cleanup(hDev,
                    "error: cannot query product name: %s\n", usb.strerror());
                continue;
            }
            string[len] = '\0';
            if (len < prodLen || strstr(string, xuacProduct) == NULL) {
                xuac_cleanup(hDev, NULL);
                continue;
            }
            xuac_dbg(0, "xuac name: %s", string);

            len = usbGetStringAscii(*hDev,
                dev->descriptor.iSerialNumber, 0x0409,
                string, sizeof(string) - 1);
            if (len < 0 && PortNumber != 0){
                // we need the serial number, when PortNumber is not 0
                xuac_cleanup(hDev,
                    "error: cannot query serial number: %s\n",
                    usb.strerror());
                continue;
            }
            serialnum = 0;
            if (len > 0 && len <=3 ) {
                string[len] = '\0';
                serialnum = atoi(string);
            }
            if (PortNumber != serialnum) {
                // keep in mind the handle, if the device's
                // serial number is less than previous ones
                if(serialnum < leastserial) {
                    leastserial = serialnum;
                    preferredDefaultHandle = dev;
                }
                xuac_cleanup(hDev, NULL);
                continue;
            }

            xuac_dbg(0, "xuac serial number: %3u", serialnum);
            return;
        }
    }
    // if no default device was found because only specific devices were present,
    // determine the default device from the specific ones and open it
    if(preferredDefaultHandle != NULL) {
        if ((*hDev = usb.open(preferredDefaultHandle)) == NULL) {
            fprintf(stderr, "error: Cannot reopen USB device: %s\n",
                usb.strerror());
        }
    }
}

// Check for a firmware version compatible with this plugin
static int
xuac_check_version(int version)
{
    xuac_dbg(0, "firmware version %d, library version %d", version,
        XUAC_VERSION);
    if (version < XUAC_VERSION) {
        fprintf(stderr, "xuac firmware version too low (%d < %d)\n",
            version, XUAC_VERSION);
        fprintf(stderr, "please update your xuac firmware\n");
        return -1;
    } else if (version > XUAC_VERSION) {
        fprintf(stderr, "xuac firmware version too high (%d > %d)\n",
            version, XUAC_VERSION);
        fprintf(stderr, "please update your OpenCBM plugin\n");
        return -1;
    }
    return 0;
}

/*! \brief Query unique identifier for the xuac device
  This function tries to find an unique identifier for the xuac device.

  \param PortNumber
   The device's serial number to search for also. It is not considered, if set to 0.

  \return
    0 on success, -1 on error. On error, the handle is cleaned up if it
    was already active.

  \remark
    On success, xuac_handle contains a valid handle to the xuac device.
    In this case, the device configuration has been set and the interface
    been claimed. xuac_close() should be called when the user is done
    with it.
*/
const char *
xuac_device_path(int PortNumber)
{
#define PREFIX_OFFSET   (sizeof("libusb/xuac:") - 1)
    usb_dev_handle *hDev;
    static char dev_path[PREFIX_OFFSET + LIBUSB_PATH_MAX] = "libusb/xuac:";

    dev_path[PREFIX_OFFSET + 1] = '\0';
    xuac_enumerate(&hDev, PortNumber);

    if (hDev != NULL) {
        strcpy(dev_path, (usb.device(hDev))->filename);
        xuac_close(hDev);
    } else {
        fprintf(stderr, "error: no xuac device found\n");
    }

    return dev_path;
}
#undef PREFIX_OFFSET

static int
xuac_clear_halt(usb_dev_handle *handle)
{
    int ret;

    ret = usb.clear_halt(handle, XUAC_BULK_IN_ENDPOINT | USB_ENDPOINT_IN);
    if (ret != 0) {
        fprintf(stderr, "USB clear halt request failed for in ep: %s\n",
            usb.strerror());
        return -1;
    }
    ret = usb.clear_halt(handle, XUAC_BULK_OUT_ENDPOINT);
    if (ret != 0) {
        fprintf(stderr, "USB clear halt request failed for out ep: %s\n",
            usb.strerror());
        return -1;
    }

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
    ret = usb.control_msg(handle, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE,
        0, XUAC_BULK_IN_ENDPOINT | USB_ENDPOINT_IN, NULL, 0, USB_TIMEOUT);
    if (ret != 0) {
        fprintf(stderr, "USB clear control req failed for in ep: %s\n",
            usb.strerror());
        return -1;
    }
    ret = usb.control_msg(handle, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE,
        0, XUAC_BULK_OUT_ENDPOINT, NULL, 0, USB_TIMEOUT);
    if (ret != 0) {
        fprintf(stderr, "USB clear control req failed for out ep: %s\n",
            usb.strerror());
        return -1;
    }
#endif // __APPLE__

    return 0;
}

/*! \brief Initialize the xuac device
  This function tries to find and identify the xuac device.

  \param hDev
   Pointer to a XUAC_HANDLE which will contain the file handle of the USB device.

  \param PortNumber
   The device's serial number to search for also. It is not considered, if set to 0.

  \return
    0 on success, -1 on error. On error, the handle is cleaned up if it
    was already active.

  \remark
    On success, xuac_handle contains a valid handle to the xuac device.
    In this case, the device configuration has been set and the interface
    been claimed. xuac_close() should be called when the user is done
    with it.
*/
int
xuac_init(usb_dev_handle **hDev, int PortNumber)
{
    unsigned char devInfo[XUAC_DEVINFO_SIZE], devStatus;
    int len;

    xuac_enumerate(hDev, PortNumber);

    if (*hDev == NULL) {
        fprintf(stderr, "error: no xuac device found\n");
        return -1;
    }

    // Select first and only device configuration.
    if (usb.set_configuration(*hDev, 1) != 0) {
        xuac_cleanup(hDev, "USB error: %s\n", usb.strerror());
        return -1;
    }

    /*
     * Get exclusive access to interface 0.
     * After this point, do cleanup using xuac_close() instead of
     * xuac_cleanup().
     */
    if (usb.claim_interface(*hDev, 0) != 0) {
        xuac_cleanup(hDev, "USB error: %s\n", usb.strerror());
        return -1;
    }

    // Check the basic device info message for firmware version
    memset(devInfo, 0, sizeof(devInfo));
    len = usb.control_msg(*hDev, USB_TYPE_CLASS | USB_ENDPOINT_IN,
        XUAC_INIT, 0, 0, (char*)devInfo, sizeof(devInfo), USB_TIMEOUT);
    if (len < 2) {
        fprintf(stderr, "USB request for XUAC info failed: %s\n",
            usb.strerror());
        xuac_close(*hDev);
        return -1;
    }
    if (xuac_check_version(devInfo[0]) != 0) {
        xuac_close(*hDev);
        return -1;
    }
    if (len >= 4) {
        xuac_dbg(0, "device capabilities %02x status %02x",
            devInfo[1], devInfo[2]);
    }

    // Check for the xuac's current status. (Not the drive.)
    devStatus = devInfo[2];
    if ((devStatus & XUAC_DOING_RESET) != 0) {
        fprintf(stderr, "previous command was interrupted, resetting\n");
        // Clear the stalls on both endpoints
        if (xuac_clear_halt(*hDev) < 0) {
            xuac_close(*hDev);
            return -1;
        }
    }

    //  Enable disk or tape mode.
	if (devInfo[1] & BOARD_CAPABILITY_TAPE)
	{
        xuac_dbg(1, "[xuac_init] Tape support.");
	}
	else
	{
        xuac_dbg(1, "[xuac_init] No tape support.");
	}

    return 0;
}
/*! \brief close the xuac device

 \param hDev
   Pointer to a XUAC_HANDLE which will contain the file handle of the USB device.

 \remark
    This function releases the interface and closes the xuac handle.
*/
void
xuac_close(usb_dev_handle *hDev)
{
    int ret;

    xuac_dbg(0, "Closing USB link");

    ret = usb.control_msg(hDev, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        XUAC_SHUTDOWN, 0, 0, NULL, 0, 1000);
    if (ret < 0) {
        fprintf(stderr,
            "USB request for XUAC close failed, continuing: %s\n",
            usb.strerror());
    }
    if (usb.release_interface(hDev, 0) != 0)
        fprintf(stderr, "USB release intf error: %s\n", usb.strerror());

    if (usb.close(hDev) != 0)
        fprintf(stderr, "USB close error: %s\n", usb.strerror());
}

/*! \brief  Handle synchronous USB control messages, e.g. for RESET.
    xuac_ioctl() is used for bulk messages.

 \param hDev
   A XUAC_HANDLE which contains the file handle of the USB device.

 \param cmd
   The command to run.

 \return
   Returns the value the USB device sent back.
*/
int
xuac_control_msg(usb_dev_handle *hDev, unsigned int cmd)
{
    int nBytes;

    xuac_dbg(1, "control msg %d", cmd);

    nBytes = usb.control_msg(hDev, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        cmd, 0, 0, NULL, 0, USB_TIMEOUT);
    if (nBytes < 0) {
        fprintf(stderr, "USB error in xuac_control_msg: %s\n",
            usb.strerror());
        exit(-1);
    }

    return nBytes;
}

/*! \brief Send tape operations abort command to the xuac device

 \param hDev
   A XUAC_HANDLE which contains the file handle of the USB device.

 \return
   Returns the value the USB device sent back.
*/
int
Device_tap_break(usb_dev_handle *hDev)
{
    xuac_dbg(1, "[Device_tap_break] Sending tape break command.");

    return xuac_control_msg(hDev, XUAC_TAP_BREAK);
}

/*! \brief Receive bulk data from USB device (internal function).

 \param hDev
    A XUAC_HANDLE containing the handle of the USB device.

 \param isTapeCmd
    Boolean specifying whether this bulk transmission is part of a tape operation.

 \param data
    Pointer to a buffer for the data to be received from the USB device.
    Must not be NULL.

 \param size
    The number of bytes to be read from the USB device.

 \return
    >=0 : Finished successfully. Number of bytes read from USB device.
    -1  : Fatal error. Check console output and debug messages.
*/
int
Device_read_buffer(usb_dev_handle *hDev, BOOL isTapeCmd, unsigned char *data, size_t size)
{
    int rd;
    size_t bytesRead, bytes2read;

    UNREFERENCED_PARAMETER(isTapeCmd);

    xuac_dbg(1, "[Device_read_buffer] device -> 0x%p (%d bytes)", data, size);

    bytesRead = 0;

    while (bytesRead < size) {

        // Determine data packet size.
        bytes2read = size - bytesRead;
        if (bytes2read > XUAC_MAX_XFER_SIZE) {
            bytes2read = XUAC_MAX_XFER_SIZE;
        }

        // Receive data packet from device.
        rd = usb.bulk_read(hDev,
            XUAC_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char *)data, bytes2read, LIBUSB_NO_TIMEOUT);

        if (rd < 0) {
            fprintf(stderr, "USB error in read data(%p, %d): %s\n",
                data, (int)size, usb.strerror());
            return -1;
        } else if (rd >= 0) {
            xuac_dbg(2, "[R:%d]", rd);
        }

        // Update data pointer & counter.
        data += rd;
        bytesRead += rd;

        // The transfer is done if less bytes were read than requested (or 0).
        // Even if still data left to be read.
        if (rd < (int)bytes2read) {
            xuac_dbg(1, "[Device_read_buffer] short read: %d<%d", rd, (int)bytes2read);
            break;
        }
    }

    xuac_dbg(1, "[Device_read_buffer] read %d bytes", bytesRead);

    return bytesRead;
}

/*! \brief Send bulk data to USB device (internal function).

 \param hDev
    A XUAC_HANDLE containing the handle of the USB device.

 \param isTapeCmd
    Boolean specifying whether this bulk transmission is part of a tape operation.

 \param data
    Pointer to a buffer containing the data to be written to the USB device.
    Must not be NULL.

 \param size
    The number of bytes to be written to the USB device.

 \return
    >=0 : Finished successfully. Number of bytes sent to USB device.
    -1  : Fatal error. Check console output and debug messages.
*/
int
Device_write_buffer(usb_dev_handle *hDev, BOOL isTapeCmd, const unsigned char *data, size_t size)
{
    int wr, ret;
    size_t bytesWritten, bytes2write;

    xuac_dbg(1, "[Device_write_buffer] 0x%p -> device (%d bytes)", data, size);

    bytesWritten = 0;

    while (bytesWritten < size) {

        // Determine data packet size.
        bytes2write = size - bytesWritten;
        if (bytes2write > XUAC_MAX_XFER_SIZE) {
            bytes2write = XUAC_MAX_XFER_SIZE;
        }

        // Send data packet to device.
        wr = usb.bulk_write(hDev,
            XUAC_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
            (char *)data, bytes2write, LIBUSB_NO_TIMEOUT);

        if (wr < 0) {
            if (isTapeCmd)
            {
                if (usb.resetep(hDev, XUAC_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT) < 0)
                    fprintf(stderr, "USB reset request failed for out ep (tape stall): %s\n", usb.strerror());
                if (usb.control_msg(hDev, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE, 0, XUAC_BULK_OUT_ENDPOINT, NULL, 0, USB_TIMEOUT) < 0)
                    fprintf(stderr, "USB error in xuac_control_msg (tape stall): %s\n", usb.strerror());
                return bytesWritten;
            }
            fprintf(stderr, "USB error in write data(%p, %d): %s\n",
                data, (int)size, usb.strerror());
            return -1;
        }
        else if (wr >= 0) {
            xuac_dbg(2, "[W:%d]", wr);
        }

        // Update data pointer & counter.
        data += wr;
        bytesWritten += wr;

        // The transfer is done if less bytes were written than requested (or 0).
        // Even if still data left to be written.
        if (wr < (int)bytes2write) {
            xuac_dbg(1, "[Device_write_buffer] short write: %d<%d", wr, (int)bytes2write);
            break;
        }
    }

    xuac_dbg(1, "[Device_write_buffer] wrote %d bytes", bytesWritten);

    return bytesWritten;
}

/*! \brief Send bulk command to USB device (internal function).

 \param hDev
    A XUAC_HANDLE containing the handle of the USB device.

 \param cmd
    Service request (command) for the USB device.

 \param subcmd
    USB function subrequest ID.
    A service request (command) may optionally have a subcommand.

 \param data
    Data value sent along with service request (command)
    to the USB device.

 \return
    >=0 : Finished successfully.
    <0  : Fatal error. Check console output and debug messages.
*/
int
Device_SendCommand(usb_dev_handle *hDev,
                   unsigned char cmd, unsigned char subcmd,
                   unsigned __int32 data)
{
    int res;
    unsigned char cmdBuf[XUAC_CMD_LEN];

    xuac_dbg(1, "[Device_SendCommand] %u:%u:%u", cmd, subcmd, data);

    // Send 4-byte command to device

    cmdBuf[0] = cmd;
    cmdBuf[1] = subcmd;
    cmdBuf[2] = data & 0xFF;
    cmdBuf[3] = (data >>  8) & 0xFF;
    cmdBuf[4] = (data >> 16) & 0xFF;
    cmdBuf[5] = (data >> 24) & 0xFF;

    res = usb.bulk_write(hDev,
        XUAC_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
        (char *)cmdBuf, sizeof(cmdBuf), LIBUSB_NO_TIMEOUT);

    if (res < 0) {
        fprintf(stderr, "USB error in SendCommand: %s", usb.strerror());
        //return -1; exit(-1);
    }

    xuac_dbg(1, "[Device_SendCommand] result=%d (bytes sent)", res);

    return res;
}

/*! \brief Wait for an answer from USB device (internal function).
    This function can be used after a bulk command was sent to
    the USB device and an answer (consisting of status and data value)
    is expected.

 \param hDev
    A XUAC_HANDLE containing the handle of the USB device.

 \param data
    Pointer to a variable that receives the data value sent along with
    status from USB device. NULL if no data requested.

 \return
    >=0 : Status value.
    -1  : Fatal error. Check console output and debug messages.
*/
static int
Device_ReceiveAnswer(usb_dev_handle *hDev, unsigned __int32 *data)
{
    int nBytes, deviceBusy, ret;
    unsigned char answerBuf[XUAC_ANSWER_LEN];

    xuac_dbg(2, "[Device_ReceiveAnswer] waiting for answer");
    deviceBusy = 1;
    while (deviceBusy) {
        nBytes = usb.bulk_read(hDev,
            XUAC_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
            (char *)answerBuf, sizeof(answerBuf), LIBUSB_NO_TIMEOUT);
        if (nBytes == XUAC_ANSWER_LEN) {
            switch (XUAC_GET_STATUS(answerBuf)) {
            case XUAC_IO_BUSY:
                xuac_dbg(2, "device busy, waiting");
                break;
            case XUAC_IO_ERROR:
                fprintf(stderr, "device reports error\n");
                /* FALLTHROUGH */
            case XUAC_IO_READY:
                deviceBusy = 0;
                break;
            default:
                fprintf(stderr, "unknown status value: %d\n",
                    XUAC_GET_STATUS(answerBuf));
                exit(-1);
            }
        } else {
            fprintf(stderr, "USB error in Device_ReceiveAnswer: %s\n",
                usb.strerror());
            exit(-1);
        }
    }

    // Once we have a valid response (done ok), get extended status
    if (XUAC_GET_STATUS(answerBuf) == XUAC_IO_READY)
    {
        ret = (int)*((__int16 *)(answerBuf+1));
        if (data != NULL) {
            *data = *((unsigned __int32 *)(answerBuf+3));
        }
    }
    else
        ret = -1;

    if (data != NULL) {
        xuac_dbg(2, "[Device_ReceiveAnswer] answer=%d:%u", ret, *data);
    } else {
        xuac_dbg(2, "[Device_ReceiveAnswer] answer=%d:-", ret);
    }

    return ret;
}

/*! \brief Send bulk command+subcommand+data to USB device,
    optionally send bulk data, optionally receive bulk data,
    optionally receive status+data.

 \param hDev
    A XUAC_HANDLE containing the handle of the USB device.

 \param cmd
    USB function request ID.
    A service request (command) for the USB device.

 \param subcmd
    USB function subrequest ID.
    A service request (command) may optionally have a subcommand.

 \param data
    Data sent along with above command.
    A command may come with additional 4 bytes of custom data.

 \param SendBuffer
    Pointer to a buffer containing the data to be written to the USB device.
    If the pointer is NULL no data will be written.

 \param SendLength
    The number of bytes to be written to the USB device.
    Not referenced if the SendBuffer pointer is NULL.

 \param BytesWritten
    Pointer to a variable that receives the number of bytes written.
    If the pointer is NULL no data will be written.

 \param RecvBuffer
    Pointer to a buffer for the data to be received from the USB device.
    If the pointer is NULL no data will be read.

 \param RecvLength
    The number of bytes to be received from the USB device.
    Not referenced if the RecvBuffer pointer is NULL.

 \param BytesRead
    Pointer to a variable that receives the number of bytes read.
    If the pointer is NULL no data will be read.

 \param Status
    Pointer to a variable that receives the device request status.
    If the pointer is NULL no answer (consisting of status and optional
    data value) will be read.

 \param data
    Pointer to a variable that receives the data value sent along with
    status from USB device. NULL if no data requested.

 \return
     1 : Finished successfully.
    <0 : Fatal error. Check console output and debug messages.
*/
int
Device_comm_bulk(
    usb_dev_handle *hDev,
    unsigned char cmd, unsigned char subcmd, unsigned __int32 cmd_data,
    const unsigned char *SendBuffer, size_t SendLength, int *BytesWritten,
          unsigned char *RecvBuffer, size_t RecvLength, int *BytesRead,
    int *Status, unsigned __int32 *answer_data)
{
    int res;
    BOOL isTapeCmd = ((XUAC_TAP_MOTOR_ON <= cmd) && (cmd <= XUAC_TAP_MOTOR_OFF));

    xuac_dbg(1, "[Device_comm_bulk] cmd=%u:%u:%u", cmd, subcmd, cmd_data);

    // Send command to device.
    res = Device_SendCommand(hDev, cmd, subcmd, cmd_data);

    if (res < 0) {
        // Error. Stop transfer here.
        xuac_dbg(1, "[Device_comm_bulk] error %d", res);
        return res;
    }

    if ((SendBuffer != NULL) && (BytesWritten != NULL)) {
    	  // Bulk data transfer: SendBuffer -> device
        *BytesWritten = Device_write_buffer(hDev, isTapeCmd, SendBuffer, SendLength);

        xuac_dbg(1, "[Device_comm_bulk] BytesWritten = %d", *BytesWritten);

        if (*BytesWritten < 0) {
            return *BytesWritten; // Error. Stop transfer here.
        }
    }

    if ((RecvBuffer != NULL) && (BytesRead != NULL)) {
        // Bulk data transfer: device -> RecvBuffer
        *BytesRead = Device_read_buffer(hDev, isTapeCmd, RecvBuffer, RecvLength);

        xuac_dbg(1, "[Device_comm_bulk] BytesRead = %d", *BytesRead);

        if (*BytesRead < 0) {
            return *BytesRead; // Error. Stop transfer here.
        }
    }

    if (Status != NULL) {
        *Status = Device_ReceiveAnswer(hDev, answer_data);
        if (answer_data != NULL) {
            xuac_dbg(1, "[Device_comm_bulk] answer=%d:%u", *Status, *answer_data);
        } else {
            xuac_dbg(1, "[Device_comm_bulk] answer=%d:-", *Status);
        }
    }

    return 1;
}
