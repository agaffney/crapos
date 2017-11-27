include ../common.mk

OUTPUT=kernel
SUBDIRS=core $(KERNEL_ARCHDIR)

# Default target
.PHONY: all
all: $(OUTPUT)

KERNEL_OBJS = $(call generate_obj_list,$(SUBDIRS))
LIBK_OBJ = $(BASEDIR)/libc/libk.a

QEMU_COMMON_OPTS = -nographic -kernel $(OUTPUT)
QEMU_DEBUG_OPTS = -s -S

$(OUTPUT): $(KERNEL_OBJS) $(LIBK_OBJ)
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

%.s.o: %.s
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS)

# Empty rule needed to override implicit "%: %.o" rule
%.asm:
%.asm.o: %.asm
	$(NASM) $(NASM_FLAGS) $(KERNEL_ARCH_NASM_FLAGS) $< -o $@

.PHONY: run run-debug

run: $(OUTPUT)
	# Ctrl-a x to exit, Ctrl-a h for help
	$(QEMU_SYSTEM) $(QEMU_COMMON_OPTS)

run-debug: $(OUTPUT)
	# Ctrl-a x to exit, Ctrl-a h for help
	$(QEMU_SYSTEM) $(QEMU_COMMON_OPTS) $(QEMU_DEBUG_OPTS)

