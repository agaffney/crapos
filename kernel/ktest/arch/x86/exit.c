#include <arch/x86/io.h>
#include <core/kprint.h>
#include <ktest/ktest.h>

/*
 * QEMU's isa-debug-exit device (see QEMU_TEST_OPTS in kernel/Makefile) exits
 * QEMU when a value is written to its port. The exit status is (value << 1) | 1,
 * so these give 33 for a pass and 35 for a failure. QEMU itself never exits
 * with an even status here, so a crash or timeout can't look like a pass.
 */
#define KTEST_EXIT_PORT 0xf4
#define KTEST_EXIT_PASS 0x10
#define KTEST_EXIT_FAIL 0x11

void ktest_exit(int passed) {
	outb(KTEST_EXIT_PORT, passed ? KTEST_EXIT_PASS : KTEST_EXIT_FAIL);

	// QEMU handles the exit request asynchronously, so the CPU keeps running for
	// a moment after the write. Wait it out, so that the message below only
	// appears when the device really is missing.
	for (volatile unsigned int i = 0; i < 100000000; i++) {
	}

	kprint("ktest: isa-debug-exit device not present, halting\n");
	for (;;) {
		asm volatile ("cli; hlt");
	}
}
