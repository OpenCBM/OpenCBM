xu1541 update tool
------------------

This is 99.9% based on the avr usb bootloader by Thomas Fischl.
See www.fischl.de for details.

This version has been adopted to the xu1541 cable. See
http://www.harbaum.org/till/xu1541 for details.


Quickstart:

1. Install jumper switch between pins 9 and 10 of SV2. This
   is the pin pair of the 10 pin header facing the pcb corner.

2. Plug device into usb port. The yellow LED must light up
   and stay on.

3. Use update tool with latest firmware. E.g.:
   ./xu1541_update main.hex

4. Unplug device from USB

5. Remove jumper switch

6. Plug device into USB port. The yellow LED must light up
   for a fraction of a second and then stay off.

7. You are done ...
