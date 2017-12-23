include ../common.mk

BINARIES=libk.a

LIBC_OBJS = $(call generate_obj_list)
LIBK_OBJS = $(subst .o,.libk.o,$(LIBC_OBJS))

all: $(BINARIES)

clean:
	find . -name '*.o' -delete
	rm -f $(BINARIES)

libc.a: $(LIBC_OBJS)
	$(AR) rcs $@ $(LIBC_OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.c.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) -D__is_libc

%.c.libk.o: %.c
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS) -I $(LIBC_INCLUDE_DIR) -D__is_libk
