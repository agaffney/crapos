include ../common.mk

BINARIES=libk.a

LIBC_OBJS = $(call generate_obj_list)
LIBK_OBJS = $(subst .o,.libk.o,$(LIBC_OBJS))
# The flags for each variant of the library, recorded separately so changing
# the kernel's flags doesn't rebuild the host library, and the other way round
LIBC_STAMP_CONTENT = $(CC) ($(CC_VERSION)) $(CFLAGS) -D__is_libc $(DEPFLAGS)
LIBK_STAMP_CONTENT = $(CC) ($(CC_VERSION)) $(KERNEL_CFLAGS) -I $(LIBC_INCLUDE_DIR) -D__is_libk $(DEPFLAGS)
$(eval $(call flags_stamp,LIBC_STAMP,build.libc,LIBC_STAMP_CONTENT))
$(eval $(call flags_stamp,LIBK_STAMP,build.libk,LIBK_STAMP_CONTENT))

all: $(BINARIES)

clean:
	find . \( -name '*.o' -o -name '*.[cSs].d' -o -name '*.libk.d' \) -delete
	rm -f $(BINARIES) build.libc.flags-* build.libk.flags-*

libc.a: $(LIBC_OBJS)
	$(AR) rcs $@ $(LIBC_OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.c.o: %.c $(LIBC_STAMP)
	$(CC) -c -o $@ $< $(CFLAGS) -D__is_libc $(DEPFLAGS)

%.c.libk.o: %.c $(LIBK_STAMP)
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS) -I $(LIBC_INCLUDE_DIR) -D__is_libk $(DEPFLAGS)

-include $(LIBC_OBJS:.o=.d) $(LIBK_OBJS:.o=.d)
