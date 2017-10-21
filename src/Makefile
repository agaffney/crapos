include common.mk

OUTPUT=kernel
DEPS=core #video keyboard

all: $(OUTPUT)

$(foreach I,$(DEPS),$I/$I.o):
	make -C $(dir $@)

$(OUTPUT): $(foreach I,$(DEPS),$I/$I.o)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $^

clean-all: $(addprefix clean-dep-, $(DEPS))
	-rm $(OUTPUT)

clean-dep-%:
	make -C $(subst clean-dep-,,$@) clean
