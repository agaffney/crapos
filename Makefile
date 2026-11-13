.PHONY: all

all: kernel libc

# The kernel build already builds libc's libk.a, which the 'libc' target builds
# too. Run them one after the other, or a parallel make runs two builds of the
# same files at once.
.NOTPARALLEL: all

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
