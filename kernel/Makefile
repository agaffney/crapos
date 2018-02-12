include ../common.mk

OUTPUT=kernel
SUBDIRS=core fs $(KERNEL_ARCHDIR)

# Default target
.PHONY: all
all: $(OUTPUT)

KERNEL_OBJS = $(call generate_obj_list,$(SUBDIRS))
LIBK_OBJ = $(BASEDIR)/libc/libk.a

# Serial console only
QEMU_OUTPUT_OPTS = -nographic
# VGA/keyboard and serial console
#QEMU_OUTPUT_OPTS = -serial stdio -display sdl
QEMU_COMMON_OPTS = -kernel $(OUTPUT) -append "foo=bar baz=123 test" -no-reboot $(QEMU_OUTPUT_OPTS) #-d int
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

