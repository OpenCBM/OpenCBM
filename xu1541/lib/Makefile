CFLAGS+=-g -Wall -pedantic -I../include/

LIB = libxu1541.a

SRCS = \
	close.c \
	display_device_info.c \
	find.c \
	get_device_info.c \
	get_pagesize.c \
	is_in_bootloader_mode.c \
	set_to_boot_mode.c \
	wait.c \

OBJS = $(SRCS:.c=.o)

.INTERMEDIATE: $(OBJS)

all: $(LIB)

clean:
	rm -f *.o *~

$(LIB): $(OBJS)
	$(AR) -r $@ $(OBJS)
