/*
 * Main loop for at90usb-based devices
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <string.h>

#include "xum1541.h"

// Flag indicating we should abort any in-progress data transfers
volatile bool doDeviceReset;

static uint8_t AbortOnReset(void);
static bool USB_BulkWorker(void);

static volatile bool device_running;

int
main(void)
{

    doDeviceReset = false;
    device_running = false;

    // Setup the CPU and board-specific configuration, USB ports
    cpu_init();
    board_init();

    // Indicate device not ready and try to reset the drive
    board_set_status(STATUS_INIT);
    cbm_init();

    /*
     * Process bulk transactions as they appear. Control requests are
     * handled separately via IRQs.
     */
    USB_Init();
    for (;;) {
        while (device_running) {
            // If we have a command to run, call the worker function
            if (USB_BulkWorker()) {
                DEBUGF("runcmd\n");
                xu1541_handle();
            }

            // Toggle LEDs each command
            board_update_display();
        }

        // TODO: save power here when device is not running
    }
}

void
EVENT_USB_Connect(void)
{
    DEBUGF("usbcon\n");
    board_set_status(STATUS_CONNECTING);
    doDeviceReset = false;
}

void
EVENT_USB_Disconnect(void)
{
    DEBUGF("usbdiscon\n");

    // Halt the main() command loop and indicate we are not configured
    device_running = false;
    board_set_status(STATUS_INIT);
}

void
EVENT_USB_ConfigurationChanged(void)
{
    DEBUGF("usbconfchg\n");

    // Clear out any old configuration before allocating
    USB_ResetConfig();

    /*
     * Setup and enable the two bulk endpoints. This must be done in
     * increasing order of endpoints (3, 4) to avoid fragmentation of
     * the USB RAM.
     */
    Endpoint_ConfigureEndpoint(XUM_BULK_IN_ENDPOINT, EP_TYPE_BULK,
        ENDPOINT_DIR_IN, XUM_ENDPOINT_BULK_SIZE, ENDPOINT_BANK_DOUBLE);
    Endpoint_ConfigureEndpoint(XUM_BULK_OUT_ENDPOINT, EP_TYPE_BULK,
        ENDPOINT_DIR_OUT, XUM_ENDPOINT_BULK_SIZE, ENDPOINT_BANK_DOUBLE);

    // Indicate USB connected and ready to start event loop in main()
    board_set_status(STATUS_READY);
    device_running = true;
}

void
EVENT_USB_UnhandledControlPacket(void)
{
    uint8_t replyBuf[4];
    int8_t len;

    /*
     * Ignore non-class requests. We also only handle commands
     * that don't transfer any data or just transfer it into the host.
     */
    if ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE) !=
        REQTYPE_CLASS) {
        DEBUGF("bad ctrl req %x\n", USB_ControlRequest.bmRequestType);
        return;
    }

    // Process the command and get any returned data
    len = usbHandleControl(USB_ControlRequest.bRequest, replyBuf);
    if (len == -1) {
        DEBUGF("ctrl req err\n");
        board_set_status(STATUS_ERROR);
        return;
    }

    // Control request was handled so ack it to allow another
    Endpoint_ClearSETUP();

    // Send data back to host and finalize the status phase
    if ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION) ==
        REQDIR_DEVICETOHOST) {
        Endpoint_Write_Control_Stream_LE(replyBuf, len);
        Endpoint_ClearOUT();
    } else {
        while (!Endpoint_IsINReady());
        Endpoint_ClearIN();
    }
}

static bool
USB_BulkWorker()
{
    uint8_t cmdBuf[XUM_CMDBUF_SIZE];
    int8_t status;

    /*
     * If we are not connected to the host or a command has not yet
     * been sent, no more processing is required.
     */
    if (!USB_IsConnected)
        return false;
    Endpoint_SelectEndpoint(XUM_BULK_OUT_ENDPOINT);
    if (!Endpoint_IsReadWriteAllowed())
        return false;

#ifdef DEBUG
    // Dump the status of both endpoints before getting the command
    Endpoint_SelectEndpoint(XUM_BULK_IN_ENDPOINT);
    DEBUGF("bsti %x %x %x %x %x %x %x %x %x\n", Endpoint_GetCurrentEndpoint(),
        Endpoint_BytesInEndpoint(), Endpoint_IsEnabled(),
        Endpoint_IsReadWriteAllowed(), Endpoint_IsConfigured(),
        Endpoint_IsINReady(), Endpoint_IsOUTReceived(), Endpoint_IsStalled());
    Endpoint_SelectEndpoint(XUM_BULK_OUT_ENDPOINT);
    DEBUGF("bsto %x %x %x %x %x %x %x %x %x\n", Endpoint_GetCurrentEndpoint(),
        Endpoint_BytesInEndpoint(), Endpoint_IsEnabled(),
        Endpoint_IsReadWriteAllowed(), Endpoint_IsConfigured(),
        Endpoint_IsINReady(), Endpoint_IsOUTReceived(), Endpoint_IsStalled());
#endif

    // Read in the command from the host now that one is ready.
    if (!USB_ReadBlock(cmdBuf, sizeof(cmdBuf))) {
        board_set_status(STATUS_ERROR);
        return false;
    }

    /*
     * Decode and process the command. For long running commands, this
     * may only queue it for xu1541_handle() since we have to get off
     * the USB bus within 1 second. For shorter commands, this processes
     * it immediately and we return the response inline.
     *
     * We use the input buffer to store the output as well. So no direct
     * command response can be >4 bytes.
     */
    status = usbHandleBulk(cmdBuf, cmdBuf);
    if (status > 0) {
        USB_WriteBlock(cmdBuf, status);
    } else if (status == -1) {
        DEBUGF("usbblk err\n");
        board_set_status(STATUS_ERROR);
        Endpoint_StallTransaction();
        return false;
    }

    // TODO: stall pipes in the error case here

    return true;
}

/*
 * The Linux and OSX call the configuration changed entry each time
 * a transaction is started (e.g., multiple runs of cbmctrl status).
 * We need to reset the endpoints before reconfiguring them, otherwise
 * we get a hang the second time through.
 *
 * We keep the original endpoint selected after returning.
 */
void
USB_ResetConfig()
{
    uint8_t lastEndpoint = Endpoint_GetCurrentEndpoint();

    Endpoint_SelectEndpoint(XUM_BULK_IN_ENDPOINT);
    Endpoint_ResetFIFO(XUM_BULK_IN_ENDPOINT);
    Endpoint_ResetDataToggle();
    Endpoint_SelectEndpoint(XUM_BULK_OUT_ENDPOINT);
    Endpoint_ResetFIFO(XUM_BULK_OUT_ENDPOINT);
    Endpoint_ResetDataToggle();

    Endpoint_SelectEndpoint(lastEndpoint);
}

bool
USB_ReadBlock(uint8_t *buf, uint8_t len)
{
    // Get the requested data from the host
    Endpoint_SelectEndpoint(XUM_BULK_OUT_ENDPOINT);
    Endpoint_Read_Stream_LE(buf, len, AbortOnReset);

    // Check if the current command is being aborted by the host
    if (doDeviceReset)
        return false;

    Endpoint_ClearOUT();
    return true;
}

bool
USB_WriteBlock(uint8_t *buf, uint8_t len)
{
    // Get the requested data from the host
    Endpoint_SelectEndpoint(XUM_BULK_IN_ENDPOINT);
    Endpoint_Write_Stream_LE(buf, len, AbortOnReset);

    // Check if the current command is being aborted by the host
    if (doDeviceReset)
        return false;

    Endpoint_ClearIN();
    return true;
}

/*
 * Callback for the Endpoint_Read/Write_Stream functions. We abort the
 * current stream transfer if the user sent a reset message to the
 * control endpoint.
 */
static uint8_t
AbortOnReset()
{
    return doDeviceReset ? STREAMCALLBACK_Abort : STREAMCALLBACK_Continue;
}
