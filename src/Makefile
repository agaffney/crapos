include common.mk

OUTPUT=kernel
DEPS=core #video keyboard

.PHONY: all

all: $(OUTPUT)

build-deps: $(addprefix build-dep-, $(DEPS))

build-dep-%:
	-make -C $(subst build-dep-,,$@)

$(foreach I,$(DEPS),$I/$I.o):
	-make -C $(dir $@)

$(OUTPUT): $(foreach I,$(DEPS),$I/$I.o)
	$(info $^)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $^

clean: $(addprefix clean-dep-, $(DEPS))
	-rm $(OUTPUT)

clean-dep-%:
	-make -C $(subst clean-dep-,,$@) clean
