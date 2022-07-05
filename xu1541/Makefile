.PHONY: all mrproper distclean clean all-linux firmware firmware-avrusb firmware-usbtiny bootloader bootloader-avrusb bootloader-usbtiny update-bootloader misc update_tool program-avrusb program-usbtiny update-avrusb update-usbtiny update-bios-avrusb update-bios-usbtiny update-all-avrusb update-all-usbtiny diff terminal version xu1541lib xmingw exe cygwin

all:	all-linux

mrproper: clean
	rm -f -- firmware/*.hex bootloader/*.hex update-bootloader/*.hex misc/read_event_log.exe misc/usb_echo_test.exe update_tool/xu1541_update.exe

distclean: mrproper

clean:
	rm -f -- firmware/*.bin bootloader/*.bin update-bootloader/*.bin misc/read_event_log misc/usb_echo_test update_tool/xu1541_update lib/libxu1541.a


all-linux: xu1541lib misc update_tool firmware bootloader update-bootloader

firmware firmware/firmware.hex:
	$(MAKE) -C firmware

firmware-avrusb: firmware/firmware.hex bootloader/bootloader-avrusb.hex
	(head -n -1 firmware/firmware.hex; cat bootloader/bootldr-avrusb.hex) > firmware+bootloader-avrusb.hex

firmware-usbtiny: firmware/firmware.hex bootloader/bootloader-usbtiny.hex
	(head -n -1 firmware/firmware.hex; cat bootloader/bootldr-usbtiny.hex) > firmware+bootloader-usbtiny.hex

bootloader: bootloader-avrusb bootloader-usbtiny

bootloader-avrusb bootloader/bootloader-avrusb.hex:
	$(MAKE) -C bootloader -f Makefile-avrusb

bootloader-usbtiny bootloader/bootloader-usbtiny.hex:
	$(MAKE) -C bootloader -f Makefile-usbtiny

update-bootloader:
	$(MAKE) -C update-bootloader

misc:
	$(MAKE) -C misc/

update_tool:
	$(MAKE) -C update_tool/src/

program-avrusb: bootloader-avrusb
	$(MAKE) -C bootloader -f Makefile-avrusb program

program-usbtiny: bootloader-usbtiny
	$(MAKE) -C bootloader -f Makefile-usbtiny program

update-firmware: firmware update_tool
	./update_tool/xu1541_update ./firmware/firmware.hex

update-bios-avrusb: update-avrusb
update-avrusb: bootloader-avrusb update_tool update-bootloader
	$(MAKE) -C update-bootloader program-avrusb

update-bios-usbtiny: update-usbtiny
update-usbtiny: bootloader-usbtiny update_tool update-bootloader
	$(MAKE) -C update-bootloader program-usbtiny

update-all-avrusb: bootloader-avrusb update_tool update-bootloader firmware
	./update_tool/xu1541_update ./update-bootloader/flash-firmware.hex -o=0x1000 ./bootloader/bootldr-avrusb.hex -R ./firmware/firmware.hex

update-all-usbtiny: bootloader-usbtiny update_tool update-bootloader firmware
	./update_tool/xu1541_update ./update-bootloader/flash-firmware.hex -o=0x1000 ./bootloader/bootldr-usbtiny.hex -R ./firmware/firmware.hex

diff:
	$(MAKE) distclean
	cvs diff|view -

terminal:
	$(MAKE) -C update-bootloader terminal

version: misc
	./misc/usb_echo_test

xu1541lib:
	$(MAKE) -C lib

xmingw:
	(export MINGW=1; $(MAKE) -C lib/)
	(export MINGW=1; $(MAKE) -C misc/)
	(export MINGW=1; $(MAKE) -C update_tool/src/)

exe:	misc update_tool

cygwin:	exe
