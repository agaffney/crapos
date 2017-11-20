include ../common.mk

ARCHDIR = arch/$(ARCH)

include $(ARCHDIR)/config.mk

OUTPUT=kernel
SUBDIRS=core $(ARCHDIR)

CFLAGS=-ggdb -O0 -fno-stack-protector -nostdinc -Wall -fno-builtin -I ./include $(ARCH_CFLAGS)

# Default target
.PHONY: all
all: $(OUTPUT)

KERNEL_OBJS = $(call generate_obj_list,$(SUBDIRS))

$(OUTPUT): $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(ARCH_LDFLAGS) -o $@ $(KERNEL_OBJS)
	$(OBJCOPY) --only-keep-debug $@ $(@).sym
	# Disable stripping for easier debugging later
	#$(STRIP) $(OUTPUT)

clean:
	find . -name '*.o' -delete
	rm -f $(OUTPUT) $(OUTPUT).sym

%.c.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.asm:
%.asm.o: %.asm
	$(NASM) $(NASM_FLAGS) $(ARCH_NASM_FLAGS) $< -o $@

.PHONY: run run-debug

run: $(OUTPUT)
	qemu-system-i386 -kernel $(OUTPUT)

run-debug: $(OUTPUT)
	qemu-system-i386 -monitor stdio -s -S -kernel $(OUTPUT)

