include ../bootloader/Makefile.common
LDFLAGS += -Wl,$(COMMON_SECTIONS)

DEFINES += -DF_CPU=12000000
COMPILE = avr-gcc -Wall -O2 -Iusbdrv -I. -mmcu=atmega8 $(DEFINES)

OBJECTS = flasher.o

TARGET=flash-firmware

all:	$(TARGET).hex

.INTERMEDIATE: $(OBJECTS) flash-firmware.bin

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

$(TARGET).bin:	$(OBJECTS)
	$(COMPILE) -o $(TARGET).bin $(OBJECTS) ${LDFLAGS}

$(TARGET).hex:	$(TARGET).bin
	rm -f $(TARGET).hex $(TARGET).eep.hex
	avr-objcopy -j .text -j .data -O ihex $(TARGET).bin $(TARGET).hex
	./checksize $(TARGET).bin 8192 960

disasm:	$(TARGET).bin
	avr-objdump -d $(TARGET).bin

cpp:
	$(COMPILE) -E flasher.c

program: $(TARGET).hex
	../update_tool/xu1541_update $(TARGET).hex

terminal:
	avrdude -c stk200 -p atmega8 -t
