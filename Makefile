.PHONY: all

all: kernel libc

# Top-level kernel targets
.PHONY: kernel kernel-%

kernel:
	$(MAKE) -C kernel

kernel-%:
	$(MAKE) -C kernel $(subst kernel-,,$@)

# Top-level libc targets
.PHONY: libc libc-%

libc:
	$(MAKE) -C libc

libc-%:
	$(MAKE) -C libc $(subst libc-,,$@)

# Top-level 'clean' target
.PHONY: clean

clean: kernel-clean libc-clean
