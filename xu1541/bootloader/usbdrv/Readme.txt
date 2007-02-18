This is the Readme file to Objective Development's firmware-only USB driver
for Atmel AVR microcontrollers. For more information please visit
http://www.obdev.at/avrusb/

This directory contains the USB firmware only. Copy it as-is to your own
project and add your own version of "usbconfig.h".


TECHNICAL DOCUMENTATION
=======================
The technical documentation for the firmware driver is contained in the file
"usbdrv.h". Please read all of it carefully!


USB IDENTIFIERS
===============
Every USB device needs a vendor- and a product-identifier (VID and PID). VIDs
are obtained from usb.org for a price of 1,500 USD. Once you have a VID, you
can assign PIDs at will.

Since an entry level cost of 1,500 USD is too high for most small companies
and hobbyists, we provide a single VID/PID pair for free. If you want to use
your own VID and PID instead of our's, define the macros "USB_CFG_VENDOR_ID"
and "USB_CFG_DEVICE_ID" accordingly in "usbconfig.h".

To use our predefined VID/PID pair, you MUST conform to a couple of
requirements. See the file "USBID-License.txt" for details.


HOST DRIVER
===========
You have received this driver together with an example device implementation
and an example host driver. The host driver is based on libusb and compiles
on various Unix flavors (Linux, BSD, Mac OS X). It also compiles natively on
Windows using MinGW (see www.mingw.org) and libusb-win32 (see
libusb-win32.sourceforge.net). The "Automator" project contains a native
Windows host driver (not based on libusb) for Human Interface Devices.


DEVELOPMENT SYSTEM
==================
This driver has been developed and optimized for the GNU compiler version 3
(gcc 3). It does work well with gcc 4 and future versions will probably be
optimized for gcc 4. We recommend that you use the GNU compiler suite because
it is freely available. AVR-USB has also been ported to the IAR compiler and
assembler. It has been tested with IAR 4.10B/W32 and 4.12A/W32 on an ATmega8
with the "small" memory model. The "tiny" memory is not supported. Please
note that gcc is more efficient for usbdrv.c because this module has been
deliberately optimized for gcc.


USING AVR-USB FOR FREE
======================
The AVR firmware driver is published under an Open Source compliant license.
See the file "License.txt" for details. Since it is not obvious for many
people how this license applies to their own projects, here's a short guide:

(1) The USB driver and all your modifications to the driver itself are owned
by Objective Development. You must give us a worldwide, perpetual,
irrevocable royalty free license for your modifications.

(2) Since you own the code you have written (except where you modify our
driver), you can (at least in principle) determine the license for it freely.
However, to "pay" for the USB driver code you link against, we demand that
you choose an Open Source compliant license (compatible with our license) for
your source code and the hardware circuit diagrams. Simply attach your
license of choice to your parts of the project and leave our "License.txt" in
the "usbdrv" subdirectory.

(3) We also demand that you publish your work on the Internet and drop us a
note with the URL. The publication must meet certain formal criteria (files
distributed, file formats etc.). See the file "License.txt" for details.

Other than that, you are allowed to manufacture any number of units and sell
them for any price. If you like our driver, we also encourage you to make a
donation on our web site.


COMMERCIAL LICENSES FOR AVR-USB
===============================
If you don't want to publish your source code and the circuit diagrams under
an Open Source license, you can simply pay money for AVR-USB. As an
additional benefit you get USB PIDs for free, licensed exclusively to you.
See http://www.obdev.at/products/avrusb/license.html for details.



