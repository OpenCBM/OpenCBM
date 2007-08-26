.PHONY: all mrproper distclean clean all-linux firmware firmware-avrusb firmware-usbtiny bootloader bootloader-avrusb bootloader-usbtiny update-bootloader misc update_tool program-bios-avrusb program-bios-usbtiny update-avrusb update-usbtiny update-bios-avrusb update-bios-usbtiny update-all-avrusb update-all-usbtiny diff terminal version

all:	all-linux

mrproper: clean
	rm -f firmware/*.hex bootloader/*.hex update-bootloader/*.hex misc/read_event_log.exe misc/usb_echo_test.exe update_tool/xu1541_update.exe

distclean: mrproper

clean:
	cvspurge

all-linux: misc update_tool firmware bootloader update-bootloader

firmware: firmware-avrusb firmware-usbtiny

firmware-avrusb:
	make -C firmware -f Makefile-avrusb

firmware-usbtiny:
	make -C firmware -f Makefile-usbtiny

bootloader: bootloader-avrusb bootloader-usbtiny

bootloader-avrusb:
	make -C bootloader -f Makefile-avrusb

bootloader-usbtiny:
	make -C bootloader -f Makefile-usbtiny

update-bootloader:
	make -C update-bootloader

misc:
	make -C misc/

update_tool:
	make -C update_tool/src/

program-bios-avrusb: bootloader-avrusb
	make -C bootloader -f Makefile-avrusb program

program-bios-usbtiny: bootloader-usbtiny
	make -C bootloader -f Makefile-usbtiny program

update-avrusb: firmware-avrusb update_tool
	./update_tool/xu1541_update ./firmware/firmware-avrusb.hex

update-usbtiny: firmware-usbtiny update_tool
	./update_tool/xu1541_update ./firmware/firmware-usbtiny.hex

update-bios-avrusb: bootloader-avrusb update_tool update-bootloader
	make -C update-bootloader program-avrusb

update-bios-usbtiny: bootloader-usbtiny update_tool update-bootloader
	make -C update-bootloader program-usbtiny

update-all-avrusb: bootloader-avrusb update_tool update-bootloader firmware-avrusb
	./update_tool/xu1541_update ./update-bootloader/flash-firmware.hex -o=0x1000 ./bootloader/bootldr-avrusb.hex -R ./firmware/firmware-avrusb.hex

update-all-usbtiny: bootloader-usbtiny update_tool update-bootloader firmware-usbtiny
	./update_tool/xu1541_update ./update-bootloader/flash-firmware.hex -o=0x1000 ./bootloader/bootldr-usbtiny.hex -R ./firmware/firmware-usbtiny.hex

diff:
	make distclean
	cvs diff|view -

terminal:
	make -C update-bootloader terminal

version: misc
	./misc/usb_echo_test
