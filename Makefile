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

# Host-side unit tests
.PHONY: test test-%

test:
	$(MAKE) -C tests

test-%:
	$(MAKE) -C tests $(subst test-,,$@)

# Top-level 'clean' target
.PHONY: clean

clean: kernel-clean libc-clean
