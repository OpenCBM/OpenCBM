xum1541 firmware
================

The xum1541 is firmware for USB device(s) that connect a 15x1 drive to
your PC. It is based on the Atmel AT90USB family of microcontrollers.
For more information, see the xum1541 web page at:

    http://www.root.org/~nate/xum1541/


Developer notes
===============
The xum1541 is very different from the xu1541 (e.g., the USB IO model).
However, the IEC routines are based on code from the xu1541.

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
