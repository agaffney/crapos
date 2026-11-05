include ../common.mk

OUTPUT=kernel
SUBDIRS=core fs $(KERNEL_ARCHDIR)

# The in-kernel tests (kernel/ktest) are only compiled and linked with KTEST=1.
# That build is written to its own file, so a normal build never contains the
# tests and the two never get mistaken for each other.
ifeq ($(KTEST),1)
SUBDIRS += ktest/core ktest/arch/$(ARCH) ktest/tests
OUTPUT = kernel-ktest
endif

# Default target
.PHONY: all
all: $(OUTPUT)

KERNEL_OBJS = $(call generate_obj_list,$(SUBDIRS))
LIBK_OBJ = $(BASEDIR)/libc/libk.a

# Serial console only
QEMU_OUTPUT_OPTS = -nographic
# VGA/keyboard and serial console
#QEMU_OUTPUT_OPTS = -serial stdio -display sdl
QEMU_COMMON_OPTS = -kernel $(OUTPUT) -append "foo=bar baz=123 test" -no-reboot $(QEMU_OUTPUT_OPTS) #-d int
QEMU_DEBUG_OPTS = -s -S
# Debug device that allows kernel to trigger a qemu exit
QEMU_TEST_OPTS = -device isa-debug-exit,iobase=0xf4,iosize=0x04

$(OUTPUT): $(KERNEL_OBJS) $(LIBK_OBJ)
	$(LD) $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIBK_OBJ)
	$(OBJCOPY) --only-keep-debug $@ $(@).sym
	# Disable stripping for easier debugging later
	#$(STRIP) $(OUTPUT)

clean:
	find . -name '*.o' -delete
	rm -f kernel kernel.sym kernel-ktest kernel-ktest.sym

$(LIBK_OBJ):
	$(MAKE) -C $(BASEDIR)/libc libk.a

%.c.o: %.c
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS)

%.s.o: %.s
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS)

# Empty rule needed to override implicit "%: %.o" rule
%.asm:
%.asm.o: %.asm
	$(NASM) $(NASM_FLAGS) $(KERNEL_ARCH_NASM_FLAGS) $< -o $@

.PHONY: run run-debug

run: $(OUTPUT)
	# Ctrl-a x to exit, Ctrl-a h for help
	$(QEMU_SYSTEM) $(QEMU_COMMON_OPTS)

run-debug: $(OUTPUT)
	# Ctrl-a x to exit, Ctrl-a h for help
	$(QEMU_SYSTEM) $(QEMU_COMMON_OPTS) $(QEMU_DEBUG_OPTS)

# Build the kernel with the in-kernel tests and boot it headless under QEMU.
# From the repo root:
#   make kernel-test-qemu                      run every test
#   make kernel-test-qemu KTEST_FILTER=<name>  run one test
# (or "make test-qemu ..." from inside kernel/)
# The kernel reports through QEMU's exit status (see kernel/ktest/arch/x86/exit.c):
# 33 means everything passed, 35 means a test failed, and anything else (a
# crash, or running past KTEST_TIMEOUT seconds) is also a failure.
KTEST_TIMEOUT ?= 60
KTEST_FILTER ?=
KTEST_QEMU_OPTS = -kernel $(OUTPUT) -no-reboot -nographic $(QEMU_TEST_OPTS)

# The filter reaches the recipe through the environment, so shell characters in
# it are never reinterpreted. (Kernel arguments are split on spaces, so a
# filter can't contain one.)
export KTEST_FILTER

# The selftest_* tests fail on purpose, to prove that a failing assertion is
# reported. They never run as part of "all", only when asked for by name.
# The list is read from the sources, so a new selftest is picked up automatically.
# The pattern accepts indentation and spaces inside the parentheses, but the
# KTEST(...) line must hold the whole test name. It is a relative path, like
# every other path in this Makefile, so make has to run in kernel/ (the root
# Makefile does that with -C).
KTEST_SELFTESTS = $(shell sed -n 's/^[[:space:]]*KTEST([[:space:]]*\(selftest_[A-Za-z0-9_]*\)[[:space:]]*).*/\1/p' ktest/tests/*.c)

.PHONY: test-qemu test-qemu-selftest ktest-run ktest-selftest

test-qemu:
	$(MAKE) KTEST=1 ktest-run

test-qemu-selftest:
	$(MAKE) KTEST=1 ktest-selftest

ktest-run: $(OUTPUT)
	@status=0; \
	timeout $(KTEST_TIMEOUT) $(QEMU_SYSTEM) $(KTEST_QEMU_OPTS) -append "ktest$${KTEST_FILTER:+=$$KTEST_FILTER}" </dev/null || status=$$?; \
	if [ $$status -eq 33 ]; then \
		echo "ktest: PASS"; \
	elif [ $$status -eq 124 ]; then \
		echo "ktest: FAIL (timed out after $(KTEST_TIMEOUT)s)"; exit 1; \
	else \
		echo "ktest: FAIL (qemu exit status $$status)"; exit 1; \
	fi

# Each self-test must make the kernel exit with the failure status (35) and
# print its FAIL line.
ktest-selftest: $(OUTPUT)
	@[ -n "$(KTEST_SELFTESTS)" ] || { echo "ktest: no selftests found"; exit 1; }; \
	for t in $(KTEST_SELFTESTS); do \
		status=0; \
		out=$$(timeout $(KTEST_TIMEOUT) $(QEMU_SYSTEM) $(KTEST_QEMU_OPTS) -append "ktest=$$t" </dev/null) || status=$$?; \
		echo "$$out" | grep -q "^FAIL  $$t" && [ $$status -eq 35 ] || { \
			echo "ktest: selftest $$t did not fail as expected (qemu exit status $$status)"; \
			echo "$(QEMU_SYSTEM) $(KTEST_QEMU_OPTS) -append \"ktest=$$t\""; \
			echo "$$out" | tr -d '\033'; exit 1; }; \
		echo "ok (failed as expected, qemu exit status $$status): $$t"; \
	done; \
	echo "ktest: selftests PASS"
