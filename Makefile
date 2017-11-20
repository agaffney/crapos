.PHONY: all

all: kernel

# Top-level kernel targets
.PHONY: kernel kernel-%

kernel:
	$(MAKE) -C kernel

kernel-%:
	$(MAKE) -C kernel $(subst kernel-,,$@)

# Top-level 'clean' target
.PHONY: clean

clean: kernel-clean
