OUTPUT=kernel

include common.mk

SUBDIRS=core keyboard #video

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(OUTPUT): $(SUBDIRS)
	$(LD) $(LDFLAGS) -T $(BASE_DIR)link.ld -o $(OUTPUT) $(foreach I,$(SUBDIRS),$I/$I.o)

.PHONY: clean clean-dep-%

clean: $(addprefix clean-dep-, $(SUBDIRS))
	rm -f $(OUTPUT)

clean-dep-%:
	$(MAKE) -C $(subst clean-dep-,,$@) clean-local
