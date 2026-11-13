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
# The flags every object is compiled with (see flags_stamp in common.mk)
FLAGS_STAMP_CONTENT = $(CC) $(KERNEL_CFLAGS) $(DEPFLAGS)
$(eval $(call flags_stamp,FLAGS_STAMP,build,FLAGS_STAMP_CONTENT))
# The same for the link command, kept apart so that a linker-only change relinks
# the kernel without recompiling anything
LINK_STAMP_CONTENT = $(LD) $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
$(eval $(call flags_stamp,LINK_STAMP,build.link,LINK_STAMP_CONTENT))
LIBK_OBJ = $(BASEDIR)/libc/libk.a

# Serial console only
QEMU_OUTPUT_OPTS = -nographic
# VGA/keyboard and serial console
#QEMU_OUTPUT_OPTS = -serial stdio -display sdl
QEMU_COMMON_OPTS = -kernel $(OUTPUT) -append "foo=bar baz=123 test" -no-reboot $(QEMU_OUTPUT_OPTS) #-d int
QEMU_DEBUG_OPTS = -s -S
# Debug device that allows kernel to trigger a qemu exit
QEMU_TEST_OPTS = -device isa-debug-exit,iobase=0xf4,iosize=0x04

$(OUTPUT): $(KERNEL_OBJS) $(LIBK_OBJ) $(LINK_STAMP) $(KERNEL_ARCHDIR)/link.ld
	$(LD) $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIBK_OBJ)
	$(OBJCOPY) --only-keep-debug $@ $(@).sym
	# Disable stripping for easier debugging later
	#$(STRIP) $(OUTPUT)

# Delete exactly the files the build generates (objects and their .d files, see
# DEPFLAGS in common.mk), not everything with a .d extension
clean:
	find . \( -name '*.o' -o -name '*.[cSs].d' \) -delete
	rm -f kernel kernel.sym kernel-ktest kernel-ktest.sym build.flags-* build.link.flags-*

# libk.a is built by the libc Makefile, which knows its own sources, headers
# and flags. FORCE runs that sub-make on every build, where it does nothing if
# libk.a is up to date. The kernel is only relinked when the sub-make really
# changed libk.a, because make looks at the file's timestamp again afterwards.
# Because make can't know that without running the sub-make, make -n lists the
# kernel link as needed and make -q always says it is out of date; the object
# files are reported correctly.
.PHONY: FORCE
FORCE:

$(LIBK_OBJ): FORCE
	$(MAKE) -C $(BASEDIR)/libc libk.a

# Each object also depends on the recorded flags, so a flag change rebuilds it
%.c.o: %.c $(FLAGS_STAMP)
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS) $(DEPFLAGS)

# .s files are not run through the C preprocessor, so the compiler's -MMD has no
# header list to give. The assembler writes its own for .include files instead.
# That is a GNU as option, so another assembler would need something else here.
%.s.o: %.s $(FLAGS_STAMP)
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS) -Wa,-MD,$(@:.o=.d)

# .S files are run through the C preprocessor first, so they can #include headers
%.S.o: %.S $(FLAGS_STAMP)
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS) $(DEPFLAGS)

# Header dependencies written by the compiler (missing on the first build)
-include $(KERNEL_OBJS:.o=.d)

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
KTEST_ALL_SELFTESTS = $(shell sed -n 's/^[[:space:]]*KTEST([[:space:]]*\(selftest_[A-Za-z0-9_]*\)[[:space:]]*).*/\1/p' ktest/tests/*.c)
# selftest_halt_vector_<N>_* hang the kernel on purpose, the rest fail an assertion
KTEST_HALT_SELFTESTS = $(filter selftest_halt_vector_%,$(KTEST_ALL_SELFTESTS))
KTEST_SELFTESTS = $(filter-out selftest_halt_vector_%,$(KTEST_ALL_SELFTESTS))
# A halting kernel never exits on its own, so these runs are stopped as soon as
# "System halted" shows up. The timeout only matters when it never does, so it
# can be generous without slowing down a passing run.
KTEST_HALT_TIMEOUT ?= 30

.PHONY: test-qemu test-qemu-selftest ktest-run ktest-selftest

test-qemu:
	$(MAKE) KTEST=1 ktest-run

test-qemu-selftest:
	$(MAKE) KTEST=1 ktest-selftest

# The output is shown as it arrives and also kept, so that a timeout can say
# which test was running and show how the output ended.
ktest-run: $(OUTPUT)
	@log=$$(mktemp); stfile=$$(mktemp); \
	{ timeout $(KTEST_TIMEOUT) $(QEMU_SYSTEM) $(KTEST_QEMU_OPTS) -append "ktest$${KTEST_FILTER:+=$$KTEST_FILTER}" </dev/null; echo $$? >$$stfile; } | tee $$log; \
	status=$$(cat $$stfile); \
	if [ $$status -eq 33 ]; then \
		rm -f $$log $$stfile; echo "ktest: PASS"; \
	elif [ $$status -eq 124 ]; then \
		echo "ktest: FAIL (timed out after $(KTEST_TIMEOUT)s)"; \
		last=$$(tr -d '\033' <$$log | sed -n 's/^ktest: running test //p' | tail -n 1); \
		[ -z "$$last" ] || echo "ktest: the last test started was $$last"; \
		echo "ktest: the output ended with:"; tr -d '\033' <$$log | tail -n 8 | sed 's/^/  | /'; \
		rm -f $$log $$stfile; exit 1; \
	else \
		rm -f $$log $$stfile; echo "ktest: FAIL (qemu exit status $$status)"; exit 1; \
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
	\
	[ -n "$(KTEST_HALT_SELFTESTS)" ] || { echo "ktest: no halt selftests found"; exit 1; }; \
	for t in $(KTEST_HALT_SELFTESTS); do \
		log=$$(mktemp); \
		timeout $(KTEST_HALT_TIMEOUT) $(QEMU_SYSTEM) $(KTEST_QEMU_OPTS) -append "ktest=$$t" </dev/null >$$log 2>&1 & \
		pid=$$!; \
		while kill -0 $$pid 2>/dev/null && ! grep -q "^System halted" $$log; do sleep 0.2; done; \
		kill $$pid 2>/dev/null; wait $$pid 2>/dev/null; \
		out=$$(tr -d '\033' <$$log | grep -v "^qemu-system.*terminating on signal"); rm -f $$log; \
		expects=$$(echo "$$out" | sed -n 's/^ktest-expect: //p'); \
		body=$$(echo "$$out" | grep -v "^ktest-expect: "); \
		missing=$$(echo "$$expects" | while IFS= read -r want; do echo "$$body" | grep -qF -- "$$want" || echo "$$want"; done); \
		if [ -z "$$expects" ] || [ -n "$$missing" ] || ! echo "$$body" | grep -q "^System halted"; then \
			echo "ktest: halt selftest $$t did not halt as expected"; \
			[ -n "$$expects" ] || echo "the test printed no 'ktest-expect:' lines"; \
			[ -z "$$missing" ] || { echo "missing from the output:"; echo "$$missing" | sed 's/^/  | /'; }; \
			echo "$$out"; exit 1; \
		fi; \
		echo "ok (halted as expected): $$t"; \
	done; \
	echo "ktest: selftests PASS"
