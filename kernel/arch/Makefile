SUBDIRS=x86

include ../common.mk

$(OUTPUT): $(SUBDIRS)
	$(LD) $(LDFLAGS) -r -o $(OUTPUT) $(foreach I,$(SUBDIRS),$I/$I.o)
