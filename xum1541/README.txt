xum1541 firmware
================

The xum1541 is firmware for USB device(s) that connect a 15x1 drive to
your PC. It is based on the Atmel AT90USB family of microcontrollers
and is provided under the GPL license.

For more information, see the xum1541 web page at:

    http://www.root.org/~nate/xum1541/


Installation
============
To install or upgrade the firmware on a device, you just need to flash it
with the .hex file from this distribution. The debug files log additional
information over the AVR's UART and may run slower, so they should only
be used if you're trying to fix a problem.

All at90usb CPUs come with a USB bootloader builtin. This means that you
don't need any special cables to flash the device. Simply plug it into
your USB port and program it. I recommend the free Atmel Flip programming
software.

With the AT90USBKEY board, the steps to program it with Flip are as follows.

1. Start up Atmel Flip
2. Plug in the device via USB
3. Press and hold both the RST and HWB buttons at the same time. Now,
release the RST button and then the HWB button. This puts the device
in upgrade mode. There's no need to hurry, just make sure both buttons
are held down at the same time and then released in the right order.
4. Click on the USB cable icon and select "USB" then "open". If all goes
well, the lower right corner will say "USB ON". If it can't connect to
the device, you'll see "Could not open USB device". In this case, repeat
the steps above in #3. Be sure you're releasing the buttons in the right
order.
5. File->Load Hex File and choose the firmware .hex file named
xum1541-AT90USBKEY-(version).hex. It should say "HEX file parsed" in the
lower left of the window status area.
6. Device->Select and be sure the AT90USB1287 cpu is selected
7. Be sure the "Operations Flow" checkboxes are set to Erase, Program, and
Verify.
8. Click the Run button
9. If all goes well, the lower left status area will say "Verify PASS".

You're done! Now unplug and replug in the xum1541 and verify it is present
by running "cbmctrl detect". You should see something like:
"8: 1540 or 1541 (XP1541)".


Compiling
=========
Whenever new releases come out, the .hex files are updated as well.
Thus most users should never need to compile the firmware. If you're a
developer, here's how to do it.

Get avr-gcc, avr-binutils, avr-libc, and a Unix shell environment (make).
On Windows, I use WinAVR to get all of the AVR utils in one place and
Cygwin for the shell environment. Try just typing "make" and it should
build the firmware. To enable the debug build, uncomment the appropriate
line in the Makefile. If the build fails, check your path to be sure
the AVR bin directory is present.


Developer notes
===============
The xum1541 is very different from the xu1541 (e.g., the USB IO model).
However, the IEC routines are based on code from the xu1541 and I got a
lot of ideas from Till Harbaum's design. The LUFA USB library by Dean
Camera was also invaluable.

The xum1541 has 3 USB endpoints: control, bulk in, and bulk out.
The control endpoint is used for initializing the device and reseting it
and the CBM drive if an error occurs. It is run from an interrupt handler
so that the command can override any pending transfer.

The bulk endpoints are used for the command, data, and status phases of
various transactions. The ordinary procedure for a command that transfers
data from the drive is:

1. Write 4-byte command descriptor to bulk out pipe
2. Wait indefinitely on bulk in pipe for data to be transferred.
   Once it is ready, keep reading until all has been retrieved.
3. Write 4-byte command descriptor for GET_RESULT to bulk out pipe
4. Wait indefinitely on bulk in pipe for 2-byte status to be transferred.

The procedure for a command that transfers data to the drive is:
1. Write 4-byte command descriptor to bulk out pipe
2. Write data to the bulk out pipe until all has been transferred.
3. Write 4-byte command descriptor for GET_RESULT to bulk out pipe
4. Wait indefinitely on bulk in pipe for 2-byte status to be transferred.

The xu1541 uses only control transfers, and thus has to implement IO in
two stages. First it transfers data to the microcontroller in a 128-byte
buffer, then it transfers it to the PC or drive. We still use this model
for the CBM protocol since it also gives an easy way to decouple the PC
from the IEC bus. However, for data transfers for s1, s2, pp, p2, etc.,
it can stream the data all at once as we have some guarantee that the
drive is waiting for us and we won't be holding the bus for too long.
This increases performance.

The firmware is organized in a logical way to support multiple device
models being based off the same firmware. The first model is the AT90USBKEY,
which is based on the Atmel development board of the same name. The CPU
files (e.g., cpu-usbkey.h) define routines that are unique to the CPU, in
this case the at90usb1287. This includes timers and initialization. The
board files (e.g., board-usbkey.[ch]) define the methods for setting up
and interacting with the IO pins on the board. Each device is composed of
a combination of board/cpu in the Makefile.

This approach allows reuse. Adding a new design is simply a matter of
making a copy of the board and cpu files and adding your own interface
routines. However, you should avoid doing this whenever possible.
For example, if you changed to an at90usb16 cpu from the at90usb1287, there
is not any need yet to change CPU files as both use the same IO ports and
same basic timer routines.


AT90USBKEY model
================
This is the first generation board and is based on the Atmel AT90USBKEY
developer's kit. The ports are allocated as follows (see board-usbkey.c):

  A: CBM IEC lines, in and out
  C: CBM parallel lines, bidirectional
  D: UART for debug output (optional, under #ifdef DEBUG)

We use these lines to interface with an existing XAP1541 adapter via the
standard DB25 printer port. Thus, the prototype requires you to own an
XA/XAP1541, which is connected to the USBKEY via a female DB25 port that
is directly wired to the board. Alternatively, the XAP1541 IEC and CBM
parallel port, transistors, etc. can be built into a daugterboard that is
plugged into the USBKEY. The xum1541 firmware cannot tell the difference
between these two designs as they behave identically.

Pinouts:
The config is wired for an XAP1541 adapter, meaning logic lines are
inverted for driving the IEC bus (drive 1 out the IO port to pull
the corresponding IEC line low). Inputs are read normally. Port A is next
to the Atmel logo, port C is the next one to the right.

Port A (IEC):
1  nc
2  GND
3  RST in
4  RST out
5  DATA in
6  DATA out
7  CLK in
8  CLK out
9  ATN in
10 ATN out

Port C (parallel):
1  nc
2  GND
3  data7
4  data6
5  data5
6  data4
7  data3
8  data2
9  data1
10 data0


Other models
============
I expect others will offer custom or prepackaged boards based on this
firmware.


Tasks
=====
Bugs:
- "cbmctrl change 8" times out instead of waiting forever
- Drive does not answer again after aborting "cbmread 8 file" with ^C
- Driver error for command: cbmforng -e 41 -v -c -s 9 disk,id
  Does not occur for normal formatting.
- cbmctrl status after ^C aborting a cbmrpm41 returns bad byte for index 0

Improvements:
- LEDs don't look cool enough  :-)
- remove GET_RESULT command
- add handling of USB stalls in error case
- Readability: xu1541_ioctl needs to be split up in plugin for 3 types of IO
- enable WDT in order to avoid need for manual reset of AVR in cases where
  it is hung up

General opencbm issues not in xum1541 code:
- cbmcopy only does byte at a time, does not use read_n() API from plugin
