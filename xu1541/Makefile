.PHONY:	cleanup misc update_tool bootloader bootloader-avrusb bootloader-usbtiny update-bootloader firmware firmware-avrusb firmware-usbtiny

all:	all-linux

mrproper: clean
	rm -f firmware/*.hex bootloader/*.hex misc/read_event_log.exe misc/usb_echo_test.exe update_tool/xu1541_update.exe

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
