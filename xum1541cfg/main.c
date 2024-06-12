/*
 * xum1541cfg -- configuration and firmware updater for xum1541 devices
 *
 * Copyright 2011 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <getopt.h>
#define sleep(x)    Sleep((x) * 1000)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "usbcommon.h"

#include "util.h"

// dfu-programmer includes
#include "commands.h"
#include "dfu-device.h"
#include "dfu.h"
#include "arguments.h"
#include "intel_hex.h"

// Mapping of common name to Atmel CPU names
struct XumDevice {
    char *commonName;
    char *atmelName;
};

static struct XumDevice validDeviceTypes[] = {
    /* the order follows the order in xum1541.h */

    { "USBKEY",        "at90usb1287" },
    { "BUMBLEB",       "at90usb162" },
    { "ZOOMFLOPPY",    "atmega32u2" },
    { "OLIMEX",        "at90usb162" },
    { "TEENSY2",       "atmega32u4" },
    { "PROMICRO",      "atmega32u4" },
    { "PROMICRO_7406", "atmega32u4" },
    { NULL,            NULL }
};
#define NO_OF_DEVICE_TYPES (sizeof(validDeviceTypes) / sizeof(struct XumDevice) - 1)

// The index of the default CPU we'll use (above)
#define DEFAULT_TYPE_IDX    2

// Maximum size the firmware could be
#define AVR_MAX_FW_SIZE     262144

static int StartBootloader(char *deviceType);
static int RunUpdate(dfu_device_t *devHandle, char *firmwareFile,
     char *deviceType, int forceFlag);
static int PrintFirmwareInfo(char *firmwareFile);

static int ParseFirmwareFile(char *firmwareFile, int *fileModel,
    int *fileVersion);
static int LoadIhex(char *firmwareFile, int16_t **buf, int *bufSize);
static int GetFileVersion(int16_t *buf, int bufSize, int *modelNum,
    int *versionNum);
#if HAVE_LIBUSB0
static int GetXumDeviceAndVariant(char *deviceType, usb_dev_handle **usbHandle, char * devNameBuf, size_t devNameBufLen);
static int GetXumDevice(char *deviceType, usb_dev_handle **usbHandle);
static int SetDFUMode(usb_dev_handle *usbHandle);
#elif HAVE_LIBUSB1
static int GetXumDeviceAndVariant(char *deviceType, libusb_device_handle **usbHandle, libusb_device **usbDevice, char * devNameBuf, size_t devNameBufLen);
static int GetXumDevice(char *deviceType, libusb_device_handle **usbHandle, libusb_device **usbDevice);
static int SetDFUMode(libusb_device_handle *usbHandle);
#endif
static int GetDFUDevice(char *deviceType, dfu_device_t *devHandle,
    struct programmer_arguments *args);
#if HAVE_LIBUSB0
static int CheckFirmwareVersion(usb_dev_handle *usbHandle, int fileModel,
    int fileVersion);
#elif HAVE_LIBUSB1
static int CheckFirmwareVersion(libusb_device_handle *usbHandle, libusb_device *usbDevice,
    int fileModel, int fileVersion);
#endif
static int EraseFlash(dfu_device_t *dev, struct programmer_arguments *args);
static int UpdateFlash(dfu_device_t *dev, struct programmer_arguments *args,
    char *firmwareFile);
static int StartDevice(dfu_device_t *dev, struct programmer_arguments *args);
static struct XumDevice *FindDeviceByName(const char *commonName);
static int16_t *ihex_search(int16_t *buf, int bufSize,
    uint8_t *pattern, uint8_t patternSize);

int verbose;
int debug;  // Used by dfu-programmer

#ifdef HAVE_LIBUSB_1_0
libusb_context *usbcontext;
#endif


// put the given device into bootloader mode, so it can be programmed
// (for example, with avrdude)
static int
StartBootloader(char *deviceType)
{
    int ret;
#if HAVE_LIBUSB0
    usb_dev_handle *usbHandle;
#elif HAVE_LIBUSB1
    libusb_device_handle *usbHandle;
    libusb_device *usbDevice;
#endif

    // Find an xum1541 device
    fprintf(stderr, "finding and preparing device for update...\n");
#if HAVE_LIBUSB0
    ret = GetXumDevice(deviceType, &usbHandle);
#elif HAVE_LIBUSB1
    ret = GetXumDevice(deviceType, &usbHandle, &usbDevice);
#endif
    if (ret == 0) {
        ret = SetDFUMode(usbHandle);
        if (ret != 0)
            return ret;
    } else {
        fprintf(stderr, "warning: no xum1541 found\n");
    }

    return 0;
}

// Update a device with new firmware from a file
static int
RunUpdate(dfu_device_t *devHandle, char *firmwareFile, char *deviceType,
    int forceFlag)
{
    int ret, fileModel, fileVersion;
#if HAVE_LIBUSB0
    usb_dev_handle *usbHandle;
#elif HAVE_LIBUSB1
    libusb_device_handle *usbHandle;
    libusb_device *usbDevice;
#endif
    struct programmer_arguments args;

    // phase 1: prepare

    // Validate the input firmware file and extract its version info
    ret = ParseFirmwareFile(firmwareFile, &fileModel, &fileVersion);
    if (ret != 0)
        return ret;

    // phase 2: find device

    // Find an xum1541 device and verify the firmware matches
    fprintf(stderr, "finding and preparing device for update...\n");
#if HAVE_LIBUSB0
    ret = GetXumDevice(deviceType, &usbHandle);
#elif HAVE_LIBUSB1
    ret = GetXumDevice(deviceType, &usbHandle, &usbDevice);
#endif
    if (ret == 0) {
        // Verify firmware version is older and put it in DFU mode
#if HAVE_LIBUSB0
        ret = CheckFirmwareVersion(usbHandle, fileModel, fileVersion);
#elif HAVE_LIBUSB1
        ret = CheckFirmwareVersion(usbHandle, usbDevice, fileModel, fileVersion);
#endif
        if (ret != 0) {
            // Fatal error or version mismatch but user specified override.
            if (ret < 0)
                return ret;
            if (!forceFlag) {
                fprintf(stderr,
            "add the -f flag if you really have the right firmware.\n");
                return -1;
            }
            fprintf(stderr,
                "warning: version mismatch but proceeding to update anyway\n");
        }
        ret = SetDFUMode(usbHandle);
        if (ret != 0)
            return ret;
    } else {
        fprintf(stderr,
"warning: no xum1541 found, continuing to look for devices in DFU mode\n");
    }

    // Get a handle to the device in DFU mode. If not found, abort.
    ret = GetDFUDevice(deviceType, devHandle, &args);
    if (ret != 0) {
        fprintf(stderr, "error: no devices found to update\n");
        return ret;
    }

    // phase 3: do update

    // Perform the update and restart the device
    fprintf(stderr, "updating firmware...\n");
    ret = EraseFlash(devHandle, &args);
    if (ret != 0) {
        fprintf(stderr, "error: flash erase failed\n");
        return ret;
    }
    ret = UpdateFlash(devHandle, &args, firmwareFile);
    if (ret != 0) {
        fprintf(stderr, "error: flash update failed\n");
        return ret;
    }

    // phase 4: cleanup
    ret = StartDevice(devHandle, &args);
    if (ret != 0) {
        fprintf(stderr,
    "warning: can't restart xum1541 but update was ok. Try re-plugging it.\n");
        return ret;
    }

    fprintf(stderr, "update completed ok!\n");
    return 0;
}

static int
PrintFirmwareInfo(char *firmwareFile)
{
    int fileModel, fileVersion;

    if (ParseFirmwareFile(firmwareFile, &fileModel, &fileVersion) != 0)
        return -1;

    // XXX improve this to print model string from int
    printf("xum1541 firmware, model %d version %d\n", fileModel, fileVersion);
    return 0;
}

static int
PrintDeviceInfo(char *deviceType)
{
    char devNameBuf[128];

#if HAVE_LIBUSB0
    usb_dev_handle *usbHandle;
#elif HAVE_LIBUSB1
    libusb_device_handle *usbHandle;
    libusb_device *usbDevice;
#endif
    int devModel, devVersion, ret;

    // Find an xum1541 device and verify the firmware matches
    fprintf(stderr, "finding device...\n");
#if HAVE_LIBUSB0
    ret = GetXumDeviceAndVariant(deviceType, &usbHandle, devNameBuf, sizeof(devNameBuf));
#elif HAVE_LIBUSB1
    ret = GetXumDeviceAndVariant(deviceType, &usbHandle, &usbDevice, devNameBuf, sizeof(devNameBuf));
#endif
    if (ret == 0)
    {
        char * deviceName = NULL;

        // Download the current version info
#if HAVE_LIBUSB0
        ret = xum1541_get_model_version(usbHandle, &devModel, &devVersion);
#elif HAVE_LIBUSB1
        ret = xum1541_get_model_version(usbHandle, usbDevice, &devModel, &devVersion);
#endif
        if (ret != 0)
        {
            fprintf(stderr, "failed to retrieve device version\n");
            return ret;
        }
        if (devModel < NO_OF_DEVICE_TYPES) {
            deviceName = validDeviceTypes[devModel].commonName;
        }
        else {
            deviceName = "UNKNOWN";
        }
        printf("xum1541 device, model %d (%s), firmware version %d\n", devModel, deviceName, devVersion);

        if (devModel >= NO_OF_DEVICE_TYPES) {
            printf("  device reports itself as: '%s'\n", devNameBuf);
        }
        return 0;
    }
    else
    {
        fprintf(stderr, "error: no xum1541 found\n");
        return ret;
    }

}

// Set the serial so that multiple xum1541 devices can be addressed.
static int
SetSerial(int newSerial)
{
    // TODO
    // Find device and put in DFU mode
    // Read the EEPROM
    // Set a new serial number
    // Overwrite the EEPROM with the new data
    // Restart the device
    return 0;
}

static int
ParseFirmwareFile(char *firmwareFile, int *fileModel, int *fileVersion)
{
    int16_t *buf;
    int ret, bufSize;

    ret = LoadIhex(firmwareFile, &buf, &bufSize);
    if (ret != 0) {
        fprintf(stderr, "error: not a valid firmware file\n");
        return ret;
    }
    ret = GetFileVersion(buf, bufSize, fileModel, fileVersion);
    if (ret != 0) {
        fprintf(stderr,
            "error: valid firmware but can't find xum1541 version in it\n");
        return ret;
    }
    free(buf);
    return 0;
}

// Load and verify ihex file, including checksums
static int
LoadIhex(char *firmwareFile, int16_t **buf, int *bufSize)
{
    int16_t *hex_data;
    int memUsed;

    hex_data = intel_hex_to_buffer(firmwareFile, AVR_MAX_FW_SIZE, &memUsed);
    if (NULL == hex_data) {
        fprintf(stderr, "\"%s\" is not a valid Intel hex firmware file\n",
            firmwareFile);
        return -1;
    }

    *buf = hex_data;
    *bufSize = memUsed;
    return 0;
}

// Get the version from the firmware file
static int
GetFileVersion(int16_t *buf, int bufSize, int *modelNum, int *versionNum)
{
    int16_t *ptr;
    static uint8_t descriptor[] = "\xff\x00\x00\x08\xd0\x16\x04\x05";
    static uint8_t idString[] = "x\0u\0m\0\x31\0\x35\0\x34\0\x31\0";

    *versionNum = -1;

    // Find an identifier string to be sure it's an xum1541 firmware
    ptr = ihex_search(buf, bufSize, idString, sizeof(idString) - 1);
    if (ptr == NULL)
        return -1;

    // Find the USB descriptor, using our hard-coded VID/PID above.
    ptr = ihex_search(buf, bufSize, descriptor, sizeof(descriptor) - 1);
    if (ptr == NULL)
        return -1;

    *modelNum = ptr[1];
    *versionNum = ptr[0];
    return 0;
}

// Find xum1541 device or Atmel DFU device, and do NOT check that we have the "right" variant
// Instead, return the name reported by the device
static int
#if HAVE_LIBUSB0
GetXumDeviceAndVariant(char *deviceType, usb_dev_handle **usbHandle, char * devNameBuf, size_t devNameBufLen)
#elif HAVE_LIBUSB1
GetXumDeviceAndVariant(char *deviceType, libusb_device_handle **usbHandle, libusb_device **usbDevice, char * devNameBuf, size_t devNameBufLen)
#endif
{
    devNameBuf[0] = 0;
    if (devNameBufLen > 0)
        devNameBuf[devNameBufLen - 1] = 0;

    // Find xum1541 device or Atmel DFU device.
#if HAVE_LIBUSB0
    *usbHandle = xum1541_find_device(0/*serial*/, devNameBuf, devNameBufLen);
#elif HAVE_LIBUSB1
    *usbHandle = xum1541_find_device(usbDevice, 0/*serial*/, devNameBuf, devNameBufLen);
#endif
    if (*usbHandle == NULL)
        return -1;

    return 0;
}

// Find xum1541 device or Atmel DFU device
static int
#if HAVE_LIBUSB0
GetXumDevice(char *deviceType, usb_dev_handle **usbHandle)
#elif HAVE_LIBUSB1
GetXumDevice(char *deviceType, libusb_device_handle **usbHandle, libusb_device **usbDevice)
#endif
{
    char devNameBuf[128];

#if HAVE_LIBUSB0
    int ret = GetXumDeviceAndVariant(deviceType, usbHandle, devNameBuf, sizeof(devNameBuf));
#elif HAVE_LIBUSB1
    int ret = GetXumDeviceAndVariant(deviceType, usbHandle, usbDevice, devNameBuf, sizeof(devNameBuf));
#endif
    if (ret < 0)
        return ret;

    // If valid but mismatches desired xum1541 hardware type, abort
    if (strstr(devNameBuf, deviceType) == NULL) {
        fprintf(stderr, "\"%s\" found, but not specified type %s\n",
            devNameBuf, deviceType);
        return -1;
    }

    return 0;
}

// Put xum1541 in DFU mode with a private control msg
static int
#if HAVE_LIBUSB0
SetDFUMode(usb_dev_handle *usbHandle)
#elif HAVE_LIBUSB1
SetDFUMode(libusb_device_handle *usbHandle)
#endif
{
    int nBytes;

#if HAVE_LIBUSB0
    nBytes = usb_control_msg(usbHandle, USB_TYPE_CLASS | USB_ENDPOINT_OUT,
        XUM1541_ENTER_BOOTLOADER, 0, 0, NULL, 0, 1000);
#elif HAVE_LIBUSB1
    nBytes = libusb_control_transfer(usbHandle, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
        XUM1541_ENTER_BOOTLOADER, 0, 0, NULL, 0, 1000);
#endif
    if (nBytes < 0) {
        fprintf(stderr, "could not put device into DFU mode: %s\n",
#if HAVE_LIBUSB0
            usb_strerror());
#elif HAVE_LIBUSB1
            libusb_error_name(nBytes));
#endif
        return -1;
    }
#if HAVE_LIBUSB0
    if (usb_close(usbHandle) != 0)
        fprintf(stderr, "USB close error: %s\n", usb_strerror());

    /*
     * Rescan immediately. This seems to help libusb find the DFU
     * device on my Mac. Other hosts may/may not need this hack.
     */
    usb_find_busses();
    usb_find_devices();
#elif HAVE_LIBUSB1
    libusb_close(usbHandle);
#endif

    return 0;
}

static int
GetDFUDevice(char *deviceType, dfu_device_t *devHandle,
    struct programmer_arguments *args)
{
#define DFU_NUM_ARGV    3
    struct XumDevice *xumDev;
    void *retPtr;
    char argvData[DFU_NUM_ARGV][32], *argv[DFU_NUM_ARGV];
    int retries;

    memset(devHandle, 0, sizeof(*devHandle));
    memset(args, 0, sizeof(*args));

    // Lookup the Atmel name from the device name to pass to dfu-programmer
    xumDev = FindDeviceByName(deviceType);
    if (!xumDev)
        return -1;
    strcpy(argvData[1], xumDev->atmelName);

    // Ugh, we have to make this dynamic mem because dfu-programmer modifies
    // the argv data as it parses it.
    strcpy(argvData[0], "dfu-programmer");
    strcpy(argvData[2], "version");
    argv[0] = argvData[0];
    argv[1] = argvData[1];
    argv[2] = argvData[2];

    // Now get the VID/PID for this Atmel DFU device
    if (parse_arguments(args, DFU_NUM_ARGV, argv) != 0)
        return -1;

    /*
     * Connect to the device and make sure it's working
     * On success, the contents of *devHandle and *args are initialized.
     *
     * We retry several times since the bus may take a while to settle
     * after being put in DFU mode by SetDFUMode().
     */
    retPtr = NULL;
    for (retries = 30; retries != 0; retries--) {
        retPtr = dfu_device_init(args->vendor_id, args->chip_id, devHandle,
            args->initial_abort, args->honor_interfaceclass);
        if (retPtr != NULL)
            break;
        else
            sleep(1);
    }
    if (retPtr == NULL)
        return -1;

    // Set some more global defaults
    args->quiet = 1;
    args->suppressbootloader = 1;
    return 0;
}

static int
#if HAVE_LIBUSB0
CheckFirmwareVersion(usb_dev_handle *usbHandle, int fileModel, int fileVersion)
#elif HAVE_LIBUSB1
CheckFirmwareVersion(libusb_device_handle *usbHandle, libusb_device *usbDevice, int fileModel, int fileVersion)
#endif
{
    int devModel, devVersion, ret;

    // Download the current version info
#if HAVE_LIBUSB0
    ret = xum1541_get_model_version(usbHandle, &devModel, &devVersion);
#elif HAVE_LIBUSB1
    ret = xum1541_get_model_version(usbHandle, usbDevice, &devModel, &devVersion);
#endif
    if (ret != 0) {
        fprintf(stderr, "failed to retrieve device version\n");
        return -1;
    }

    // Device type must match, no matter what.
    if (devModel != fileModel) {
        fprintf(stderr,
            "error: device type %d does not match firmware type %d\n",
            devModel, fileModel);
        return -1;
    }

    // Check for version being the same or newer than the update
    if (devVersion >= fileVersion) {
        fprintf(stderr,
    "note: device has version %d but firmware is not newer (version %d)\n",
            devVersion, fileVersion);
        return 1;
    }

    return 0;
}

// Erase the current firmware and verify it succeeded (is blank).
static int
EraseFlash(dfu_device_t *dev, struct programmer_arguments *args)
{
    args->command = com_erase;
    args->com_erase_data.suppress_validation = 0;
    return execute_command(dev, args);
}

// Download new firmware to the device
static int
UpdateFlash(dfu_device_t *dev, struct programmer_arguments *args,
    char *firmwareFile)
{
    args->command = com_flash;
    args->com_flash_data.suppress_validation = 0;
    args->com_flash_data.file = firmwareFile;
    args->com_flash_data.original_first_char = *firmwareFile;

    // XXX If failed, retry 3 times. Verify download succeeded
    // If failed, try download again.
    return execute_command(dev, args);
}

// Reset device so it will go back to normal execution
static int
StartDevice(dfu_device_t *dev, struct programmer_arguments *args)
{
    args->command = com_start_app;
    return execute_command(dev, args);
}

// Lookup a pointer to a device description, given the name (e.g. ZOOMFLOPPY)
static struct XumDevice *
FindDeviceByName(const char *commonName)
{
    struct XumDevice *devPtr;

    for (devPtr = validDeviceTypes; devPtr->commonName; devPtr++) {
        if (!strcmp(commonName, devPtr->commonName))
            break;
    }
    if (devPtr->commonName == NULL)
        devPtr = NULL;
    return devPtr;
}

// Search for a byte string in a firmware buffer
static int16_t *
ihex_search(int16_t *buf, int bufSize, uint8_t *pattern, uint8_t patternSize)
{
    int i, j;

    j = 0;
    for (i = 0; i < bufSize; i++) {
        while (buf[i] == pattern[j] && j < patternSize) {
            i++;
            j++;
        }
        if (j == patternSize)
            break;
        else
            j = 0;
    }
    if (i >= bufSize)
        return NULL;

    return &buf[i];
}

static void
usage(void)
{
    struct XumDevice *devPtr;
    int deviceNo = 0;

    fprintf(stderr, "usage: xum1541cfg [flags] command [args]\n\n"
"Flags:\n"
"  -v: enable verbose status messages\n\n"
"Commands:\n"
"* update xum1541-firmware.hex\n"
"    Updates the firmware of an xum1541 device. Optional flags:\n"
"      -t type: specify the device type (defaults to \"ZOOMFLOPPY\")\n"
"      -f: force update to given firmware. USE WITH CAUTION!\n"
"* bootloader\n"
"    Put the xum1541 device into bootloader mode. Optional flags:\n"
"      -t type: specify the device type (defaults to \"ZOOMFLOPPY\")\n"
"* info xum1541-firmware.hex\n"
"    Prints info extracted from the firmware file argument.\n"
"* devinfo\n"
"    Prints info extracted from the device\n"
"* list (NOT YET IMPLEMENTED)\n"
"    Prints info about all attached xum1541 devices.\n"
"* set-serial 0-255 (NOT YET IMPLEMENTED)\n"
"    Sets the firmware serial number so multiple devices can be used.\n"
"    Changes the first device found so only one xum1541 device should\n"
"    be plugged in when running this command.\n\n"
        );
    fprintf(stderr, "Supported device types (case-insensitive):\n");
    fprintf(stderr, "  %s (default)",
        validDeviceTypes[DEFAULT_TYPE_IDX].commonName);
    devPtr = &validDeviceTypes[0];
    for (deviceNo = 0; devPtr->commonName; devPtr++, deviceNo++) {
        if (deviceNo == DEFAULT_TYPE_IDX)
            continue;
        fprintf(stderr, ", %s", devPtr->commonName);
    }
    fprintf(stderr, "\n");
    exit(1);
}


int
#ifdef WIN32
__cdecl
#endif
main(int argc, char *argv[])
{
    int ch, force, retval, rv;
    char *deviceType;
    struct XumDevice *devPtr;
    dfu_device_t dfuDevice;
#ifdef HAVE_LIBUSB_1_0
    libusb_context *usbContext;
#endif

    // Parse command line flags
    force = 0;
    deviceType = validDeviceTypes[DEFAULT_TYPE_IDX].commonName;
    while ((ch = getopt(argc, argv, "fvt:")) != -1) {
        switch (ch) {
        case 'f':
            force = 1;
            break;
        case 't':
            deviceType = optarg;
            devPtr = FindDeviceByName(deviceType);
            if (devPtr == NULL) {
                printf("error: \"%s\" is not a valid xum1541 device name\n",
                    deviceType);
                usage();
            }
            break;
        case 'v':
            verbose = 1;
            break;
        case '?':
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;
    if (argc < 1)
        usage();

#ifdef HAVE_LIBUSB_1_0
    if (libusb_init(&usbContext)) {
        fprintf(stderr, "error, can't init libusb\n");
        exit(1);
    }
#else
    usb_init();
#endif
    memset(&dfuDevice, 0, sizeof(dfuDevice));

    // Now run the given command
    retval = 1;
    if (!strcmp(*argv, "update")) {
        if (argc != 2) {
            fprintf(stderr, "\"update\" needs a single filename arg\n");
            goto error;
        }
        if (RunUpdate(&dfuDevice, argv[1], deviceType, force) != 0)
            goto error;
    } else if (!strcmp(*argv, "bootloader")) {
        if (StartBootloader(deviceType) != 0)
            goto error;
    } else if (!strcmp(*argv, "info")) {
        if (PrintFirmwareInfo(argv[1]) != 0)
            goto error;
    } else if (!strcmp(*argv, "devinfo")) {
        if (PrintDeviceInfo(deviceType) !=0) {
            goto error;
        }
    } else if (!strcmp(*argv, "set-serial")) {
        fprintf(stderr, "command not yet supported, sorry\n");
        if (argc != 2)
            usage();
        if (SetSerial(atoi(argv[1])) != 0)
            goto error;
    } else {
        fprintf(stderr, "unknown command: %s\n", *argv);
    }

error:
    // Shutdown libusb and device cleanly
    if (dfuDevice.handle != NULL) {
#ifdef HAVE_LIBUSB_1_0
        rv = libusb_release_interface(dfuDevice.handle, dfuDevice.interface);
#else
        rv = usb_release_interface(dfuDevice.handle, dfuDevice.interface);
#endif
        if (rv != 0) {
            fprintf(stderr, "error, failed to release interface %d\n",
                dfuDevice.interface);
            retval = 1;
        }
    }
    if (dfuDevice.handle != NULL) {
#ifdef HAVE_LIBUSB_1_0
        libusb_close(dfuDevice.handle);
#else
        if (usb_close(dfuDevice.handle) != 0) {
            fprintf(stderr, "error, failed to close the handle\n");
            retval = 1;
        }
#endif
    }

#ifdef HAVE_LIBUSB_1_0
    libusb_exit(usbContext);
#endif
    return retval;
}
