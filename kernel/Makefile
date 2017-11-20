include ../common.mk

OUTPUT=kernel
SUBDIRS=core $(KERNEL_ARCHDIR)

# Default target
.PHONY: all
all: $(OUTPUT)

KERNEL_OBJS = $(call generate_obj_list,$(SUBDIRS))
LIBK_OBJ = $(BASEDIR)/libc/libk.a

$(OUTPUT): $(KERNEL_OBJS) $(LIBK_OBJ)
	echo $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIBK_OBJ)
	$(OBJCOPY) --only-keep-debug $@ $(@).sym
	# Disable stripping for easier debugging later
	#$(STRIP) $(OUTPUT)

clean:
	find . -name '*.o' -delete
	rm -f $(OUTPUT) $(OUTPUT).sym

$(LIBK_OBJ):
	$(MAKE) -C $(BASEDIR)/libc libk.a

%.c.o: %.c
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS)

# Empty rule needed to override implicit "%: %.o" rule
%.asm:
%.asm.o: %.asm
	$(NASM) $(NASM_FLAGS) $(KERNEL_ARCH_NASM_FLAGS) $< -o $@

.PHONY: run run-debug

run: $(OUTPUT)
	qemu-system-i386 -kernel $(OUTPUT)

run-debug: $(OUTPUT)
	qemu-system-i386 -monitor stdio -s -S -kernel $(OUTPUT)

