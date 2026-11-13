.PHONY: all

all: kernel libc

# Top-level kernel targets
.PHONY: kernel kernel-%

# The kernel links libc's libk.a, so build libc first. That way 'kernel' and
# 'libc' don't both build libk.a at the same time under make -j. (The kernel-%
# targets and make -C kernel still build it themselves, as they always did, so
# naming one of those together with 'libc' under -j can still race.)
kernel: libc
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
