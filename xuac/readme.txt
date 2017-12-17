ZoomTape readme

Contents:

(1) The ZoomTape platform
(2) Recommended MCU board
(3) ZoomTape board schematic
(4) Connecting host, MCU board, ZoomTape board.
(5) Flashing a firmware binary
(6) Software installation
(7) Opensource firmware projects
(8) Final notice

------------------------------------------------------------------------------

(1) The ZoomTape platform

The ZoomTape platform allows you to read and write tape images to and from
the original media.

The ZoomTape platform currently consists of two parts:

- a microcontroller (MCU) board running the ZoomTape opensource firmware,
- an adapter board where you connect your Commodore 1530/1531 tape drive.

The MCU board is based on "ARM Cortex-M" microcontrollers manufactured by
STMicroelectronics. It is connected on one side via USB to your host PC and
on the other side to the adapter board. OpenCBM has the software tools for
reading and writing tapes.

There are photos of a few board setups in the "xuac/Pictures/" folder.

------------------------------------------------------------------------------

(2) Recommended MCU board

A few boards are currently supported and tested. Choose the MCU board you
like, see e.g. "xuac/Pictures/" for examples. Other boards may be ok, but you
may have to define a new pinout (see (4)) and build the ZoomTape firmware
binary yourself (see (7)).

Notes:

- The STM32F4 boards are faster and have more memory than the STM32F103 ones.
- "Blue Pill" boards may have a wrong USB D+ resistor (probably marked "R10").
  It may be necessary to replace the resistor if your USB connection does not
  work. Or try another host PC if this happens. See e.g. here:
  http://wiki.stm32duino.com/index.php?title=Blue_Pill
- "STM32_F4VE_V2_0_1509" board may have wrong USB D+/D- resistors (probably
  marked "R18" and "R19"). It may be necessary to remove (bypass) them if the
  USB connection does not work (happened for me). See e.g. here:
  http://www.stm32duino.com/viewtopic.php?f=39&t=2484
- ZoomTape firmware binaries are licensed for use with STMicroelectronics
  microcontrollers, see license agreement in license.txt for details.
- Example search keywords: "STM32F103C8T6", "STM32F407VET6", "STM32F407VGT6".

------------------------------------------------------------------------------

(3) ZoomTape adapter board schematic

You need a ZoomTape adapter board for connecting your Commodore 1530/1531
tape drive to your ZoomTape MCU board. Building the adapter board usually
requires soldering. See "xuac/Schematics/" for a few schematics.

Note: The "Simple2" schematic is untested.

------------------------------------------------------------------------------

(4) Connecting host PC, MCU board, adapter board.

Take care of electrostatic discharge when touching your hardware.

Connect your host PC via USB 2.0 to your MCU board (USB3 untested).
Connect your MCU board via a few pins to the adapter board: These pins
are board specific, you will find the correct pins in one of the following
files that corresponds to your MCU board:

- xuac/Firmware projects/STM32F103/Inc/board_defs_BlackPill.h
- xuac/Firmware projects/STM32F103/Inc/board_defs_BluePill.h

- xuac/Firmware projects/STM32F407/Inc/board_defs_DIY_MORE_STM32_407.h
- xuac/Firmware projects/STM32F407/Inc/board_defs_STM32_F4VE_V2_0_1509.h

You need the following pins: READ, WRITE, SENSE, DISCONNECT. (And GND.)
The MOTOR pin is optional (depends on the adapter board you choose).

There are photos of a few board setups in the "xuac/Pictures/" folder:
See "Pro Adapter*.jpg" and "Simple1 Adapter*.jpg".

The "Simple" adapters do not have software MOTOR control, the motor is
always on.

The MCU board is powered via USB. The adapter board is powered via external
PSU, see schematics in (3).

------------------------------------------------------------------------------

(5) Flashing a firmware binary

Your MCU board needs the correct ZoomTape firmware binary. There are binaries
for a few MCU boards in the "xuac/Firmware binaries/" folder, e.g.:

- STM32F103C8T6_BlackPill.binary
- STM32F103C8T6_BluePill.binary
- STM32F407VET6_STM32_F4VE_V2_0_1509.binary
- STM32F407VGT6_DIY_MORE_STM32_407.binary

Select the correct binary for your MCU board.
The photos in "xuac/Pictures/" may help you to identify your MCU board.

You must accept the license agreement in license.txt before using these
firmware binaries.

There are different ways of flashing a firmware binary into the MCU,
here is one way.

- Take care of electrostatic discharge when touching your hardware.
- Disconnect USB and any other power source from your MCU board.
- Set BOOT0 to 1 on your MCU board for flash mode (usually a jumper).
- Connect a USB-to-Serial adaptor to the MCU board via serial (+power):
  usually TX-RX and RX-TX, see (4) for the MCU board specific pinout, and see
  "xuac/Pictures/Flash*.jpg" for photos.
- Run the opensource software "stm32flash v0.5" in a console window to flash
  the firmware binary. Example:

  stm32flash -b 115200 -m 8e1 -g 0x8000000 -v -w STM32F407.binary COM4

  You may need to change "COM4" to your personal port name and configure your
  port correctly.
- When finished flashing disconnect the USB-to-Serial adaptor, disconnect any
  power source from your MCU board, set BOOT0 back to 0 and your MCU board is
  now ready to use via USB.

Note: BOOT1 is always 0.

Link:
https://sourceforge.net/projects/stm32flash/

------------------------------------------------------------------------------

(6) Software installation

You need to install the Windows driver (libusb-win32) and the OpenCBM plugin.

The Windows driver is provided in the "xuac/WinDrv/" folder, but a digital
signature is missing. Hence I did the following:

After flashing the ZoomTape firmware binary into the MCU board and connecting
it via USB I have successfully installed the "libusb-win32 (v1.2.6.0)" driver
for it using "Zadig - The Automated Driver Installer" version 2.3.
It is your own decision if you want to do this. I entered
"XUAC USB tape adapter (ZoomTape)" as the device name.

For OpenCBM installation you will need to open a console window and enter

instcbm.exe -r
instcbm.exe xuac

to remove the current plugin and install the new ZoomTape XUAC plugin.

The "opencbm 0.4.99.99 Users Guide" has detailed information about the
installation, see chapter "3. Installation" here:
http://opencbm.trikaliotis.net

Please refer to the "ZoomTape-Manual-1.0_DRAFT.pdf" for using the tape tools
(tapread, tapwrite, tapcontrol, cap2tap, tap2cap, tapview).

Links:
https://sourceforge.net/projects/libusb-win32/
https://github.com/OpenCBM
http://opencbm.trikaliotis.net
http://zadig.akeo.ie

------------------------------------------------------------------------------

(7) Opensource firmware projects

There are software projects provided in the "xuac/Firmware projects/" folder
for building the ZoomTape firmware binaries on your own. The following MCU
cores are currently supported:

- STM32F103

  Tested with:
  MCU: STM32F103C8T6 (ARM Cortex-M3),
  Boards: "Blue Pill", "Black Pill".

- STM32F407

  Tested with:
  MCUs: STM32F407VET6, STM32F407VGT6 (ARM Cortex-M4),
  Boards: "STM32_F4VE_V2.0_1509", "DIY MORE STM32-407".

These software projects are for use with "Atollic TrueSTUDIO for ARM" IDE.
Link: https://atollic.com/

You need to set the target board in "board_defs.h".

------------------------------------------------------------------------------

(8) Final notice

All product names, trademarks, registered trademarks and brands are the
property of their respective owners. All company and product names used are
for identification purposes only.

You must accept the license agreement in license.txt before using the
ZoomTape firmware binaries.

This document is licensed under Creative Commons Attribution-ShareAlike 4.0
International Public License (CC BY-SA 4.0),
see https://creativecommons.org/licenses/by-sa/4.0/
Author: Arnd Menge.

Thanks to:

- Spiro Trikaliotis for maintaining OpenCBM.
- Nate Lawson for designing the ZoomFloppy.
- Markus Brenner for mtap/ptap under MS-DOS.
- Jim Brain at RETRO Innovations.

------------------------------------------------------------------------------
