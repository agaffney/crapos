OUTPUT=kernel
SUBDIRS=arch core
LD_EXTRA_FLAGS = -T link.ld

include common.mk

$(OUTPUT): $(SUBDIRS)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(foreach I,$(SUBDIRS),$I/$I.o)
	$(OBJCOPY) --only-keep-debug $(OUTPUT) $(OUTPUT).sym
	$(STRIP) $(OUTPUT)

.PHONY: run run-debug

run: $(OUTPUT)
	qemu-system-i386 -kernel $(OUTPUT)

run-debug: $(OUTPUT)
	qemu-system-i386 -monitor stdio -s -S -kernel $(OUTPUT)

