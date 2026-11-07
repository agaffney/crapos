#include <core/kprint.h>
#include <ktest/ktest.h>
#include <stdint.h>

/*
 * These fail on purpose, to prove that a failing assertion is reported with
 * its location, marks the test FAIL and makes the run exit with the failure
 * status. They only run when selected by name; "make kernel-test-qemu-selftest" runs
 * them and checks that they do fail.
 *
 * The selftest_halt_vector_<N>_* tests cause a fatal CPU exception that no test
 * is expecting, so the kernel prints its report and halts. That run never exits
 * on its own: the Makefile expects it to time out, with the report for vector N
 * in the output. The vector is read from the test name.
 */

KTEST(selftest_failed_assertion)
{
	KTEST_ASSERT_TRUE(1 == 2);
}

KTEST(selftest_failed_eq_assertion)
{
	KTEST_ASSERT_EQ_INT(2 + 2, 5);
}

/*
 * What the report for a halt has to contain is printed by the test itself,
 * before it faults, as "ktest-expect: <text>" lines. The Makefile checks that
 * each of those texts appears in the rest of the output. Keeping the expected
 * values here, next to the instruction that causes them, means the Makefile
 * does not need to know them.
 */

/* A label on the faulting instruction, so the test can say where eip must point */
extern const char ktest_selftest_ud2_insn[];

KTEST(selftest_halt_vector_6_invalid_opcode)
{
	kprint("ktest-expect: *** fatal CPU exception 6 (invalid opcode) ***\n");
	kprint("ktest-expect: error code=0x0 eip=0x%x cs=0x8 eflags=\n", (uint32_t)ktest_selftest_ud2_insn);
	kprint("ktest-expect: eax=0x1234 ebx=\n");
	kprint("ktest-expect: System halted\n");
	asm volatile (
		"mov $0x1234, %%eax\n\t"
		".globl ktest_selftest_ud2_insn\n"
		"ktest_selftest_ud2_insn:\n\t"
		"ud2"
		: : : "eax");
}

extern const char ktest_selftest_pf_insn[];

/* The address is not mapped, so the write faults: error code 2 is a supervisor write to a missing page */
KTEST(selftest_halt_vector_14_page_fault)
{
	kprint("ktest-expect: *** fatal CPU exception 14 (page fault) ***\n");
	kprint("ktest-expect: error code=0x2 eip=0x%x cs=0x8 eflags=\n", (uint32_t)ktest_selftest_pf_insn);
	kprint("ktest-expect: faulting address (cr2)=0x50000000\n");
	kprint("ktest-expect: System halted\n");
	asm volatile (
		".globl ktest_selftest_pf_insn\n"
		"ktest_selftest_pf_insn:\n\t"
		"movl $1, 0x50000000");
}
