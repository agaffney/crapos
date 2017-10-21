include common.mk

OUTPUT=kernel
DEPS=core keyboard #video

all: $(OUTPUT)

$(foreach I,$(DEPS),$I/$I.o):
	make -C $(dir $@)

$(OUTPUT): $(foreach I,$(DEPS),$I/$I.o)
	$(LD) $(LDFLAGS) -T $(BASE_DIR)link.ld -o $(OUTPUT) $^

clean: $(addprefix clean-dep-, $(DEPS))
	rm -f $(OUTPUT)

clean-dep-%:
	make -C $(subst clean-dep-,,$@) clean-local
