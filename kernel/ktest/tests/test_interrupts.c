#include <arch/x86/gdt.h>
#include <arch/x86/int.h>
#include <arch/x86/syscall.h>
#include <ktest/ktest.h>
#include <libk/string.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Interrupts are turned off around the tests that load unusual segment
 * selectors, so a real IRQ can't arrive in the middle. IRQ_GUARD(name) turns
 * them off and, through the cleanup attribute, puts the interrupt flag back to
 * what it was when the variable goes out of scope, on every way out of the
 * test: normal end, or an assertion returning early.
 */
static inline uint32_t irq_save_disable(void)
{
	uint32_t flags;
	asm volatile ("pushf\n\tpopl %0\n\tcli" : "=r" (flags) : : "memory");
	return flags;
}

static inline void irq_restore(uint32_t * flags)
{
	if (*flags & (1 << 9)) {
		asm volatile ("sti" : : : "memory");
	}
}

#define IRQ_GUARD(name) \
	uint32_t name __attribute__((cleanup(irq_restore))) = irq_save_disable()

/* A breakpoint exception (vector 3) is handled and execution continues after it. */
KTEST(int3_returns_to_caller)
{
	int reached = 0;
	asm volatile ("int $3" ::: "memory");
	reached = 1;
	KTEST_ASSERT_TRUE(reached);
}

/* The syscall gate (int 0x80) is handled and returns to the caller. */
KTEST(syscall_returns_to_caller)
{
	int reached = 0;
	asm volatile ("mov $3, %%eax\n\tint $0x80" ::: "eax", "memory");
	reached = 1;
	KTEST_ASSERT_TRUE(reached);
}

/*
 * The IRQ stubs for vectors 34-47 (INTERRUPT_PIC1 and INTERRUPT_PIC2) are
 * entered here with a software "int", which skips the PIC. That is enough to
 * check that each IDT entry exists, that the stub returns to the caller, and
 * that it loads and restores the registers it should. It does not check the
 * PIC itself (the cascade setup, or EOI delivery for a real IRQ). Vector 32 is
 * the timer stub and 33 is the keyboard wrapper, which have their own handlers.
 *
 * The general registers are set to known values. The stubs overwrite al for
 * the EOI, so eax checks that RESTORE_REGS really restores it. ebp is left out
 * because the compiler needs it as the frame pointer.
 *
 * The data segment registers always hold the kernel data selector when a stub
 * is entered from ring 0, and the stubs load that same selector, so on their
 * own neither a missing load nor a missing restore would show. So the test
 * first loads a different, equally flat selector (the user data descriptor,
 * which ring 0 may use) into ds, es, fs and gs. Then:
 *  - ktest_int_hook() below records the segment registers from inside the
 *    handler, which must be the kernel selector (the stub loaded them), and
 *  - the registers are read again after the int and must hold the test
 *    selector (the stub restored them).
 * The kernel selector is put back before leaving the asm block.
 *
 * The hook only records state and must stay that way: it runs for every
 * interrupt. It is called from arch_x86_int_handler_default() and
 * arch_x86_int_handler_syscall(), but not from the timer and keyboard stubs,
 * which have their own handlers. So those two are not covered by the segment
 * register checks, and nor is the syscall stub, because the checks below only
 * run for vectors 34-47.
 */
#define TEST_DATA_SEL ((int)(GDT_INDEX_UDATA * sizeof(struct gdtdesc)))
#define KERNEL_DATA_SEL ((int)(GDT_INDEX_KDATA * sizeof(struct gdtdesc)))

/* SAVE_REGS (kernel/include/arch/x86/regs.h) hardcodes this selector */
_Static_assert(KERNEL_DATA_SEL == 0x10, "SAVE_REGS loads a different selector than the GDT's kernel data one");

/* Segment registers as seen from inside the interrupt handler */
static volatile uint16_t hook_ds, hook_es, hook_fs, hook_gs;
static volatile uint32_t hook_vector;
/* The frame the handler was given */
static arch_x86_int_frame hook_frame;

void ktest_int_hook(uint32_t vector, const arch_x86_int_frame * frame)
{
	uint16_t ds, es, fs, gs;
	asm volatile ("movw %%ds, %0" : "=r" (ds));
	asm volatile ("movw %%es, %0" : "=r" (es));
	asm volatile ("movw %%fs, %0" : "=r" (fs));
	asm volatile ("movw %%gs, %0" : "=r" (gs));
	hook_ds = ds;
	hook_es = es;
	hook_fs = fs;
	hook_gs = gs;
	hook_vector = vector;
	hook_frame = *frame;
}

#define IRQ_STUB_RETURNS_TEST(vec) \
	KTEST(int_##vec##_returns_with_registers_preserved) \
	{ \
		int eax = 0x66666666, ebx = 0x11111111, ecx = 0x22222222; \
		int edx = 0x33333333, esi = 0x44444444, edi = 0x55555555; \
		uint16_t test_sel = TEST_DATA_SEL, kernel_sel = KERNEL_DATA_SEL; \
		uint16_t ds, es, fs, gs; \
		/* Sentinels, so a hook that never ran can't leave stale values behind */ \
		hook_vector = 0; \
		hook_ds = hook_es = hook_fs = hook_gs = 0xffff; \
		/* Keep a real IRQ from arriving while the selectors are unusual */ \
		IRQ_GUARD(irq_flags); \
		asm volatile ( \
			"movw %[tsel], %%ds\n\t" \
			"movw %[tsel], %%es\n\t" \
			"movw %[tsel], %%fs\n\t" \
			"movw %[tsel], %%gs\n\t" \
			"int $" #vec "\n\t" \
			"movw %%ds, %[ds]\n\t" \
			"movw %%es, %[es]\n\t" \
			"movw %%fs, %[fs]\n\t" \
			"movw %%gs, %[gs]\n\t" \
			"movw %[ksel], %%ds\n\t" \
			"movw %[ksel], %%es\n\t" \
			"movw %[ksel], %%fs\n\t" \
			"movw %[ksel], %%gs" \
			: "+a" (eax), "+b" (ebx), "+c" (ecx), "+d" (edx), "+S" (esi), "+D" (edi), \
			  [ds] "=m" (ds), [es] "=m" (es), [fs] "=m" (fs), [gs] "=m" (gs) \
			: [tsel] "m" (test_sel), [ksel] "m" (kernel_sel) \
			: "memory"); \
		/* Keep this first: it says "the hook never ran", which would otherwise \
		   show up as confusing segment register mismatches below */ \
		KTEST_ASSERT_EQ_INT(hook_vector, vec); \
		KTEST_ASSERT_EQ_INT(hook_ds, KERNEL_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(hook_es, KERNEL_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(hook_fs, KERNEL_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(hook_gs, KERNEL_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(eax, 0x66666666); \
		KTEST_ASSERT_EQ_INT(ebx, 0x11111111); \
		KTEST_ASSERT_EQ_INT(ecx, 0x22222222); \
		KTEST_ASSERT_EQ_INT(edx, 0x33333333); \
		KTEST_ASSERT_EQ_INT(esi, 0x44444444); \
		KTEST_ASSERT_EQ_INT(edi, 0x55555555); \
		KTEST_ASSERT_EQ_INT(ds, TEST_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(es, TEST_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(fs, TEST_DATA_SEL); \
		KTEST_ASSERT_EQ_INT(gs, TEST_DATA_SEL); \
	}

IRQ_STUB_RETURNS_TEST(34)
IRQ_STUB_RETURNS_TEST(35)
IRQ_STUB_RETURNS_TEST(36)
IRQ_STUB_RETURNS_TEST(37)
IRQ_STUB_RETURNS_TEST(38)
IRQ_STUB_RETURNS_TEST(39)
IRQ_STUB_RETURNS_TEST(40)
IRQ_STUB_RETURNS_TEST(41)
IRQ_STUB_RETURNS_TEST(42)
IRQ_STUB_RETURNS_TEST(43)
IRQ_STUB_RETURNS_TEST(44)
IRQ_STUB_RETURNS_TEST(45)
IRQ_STUB_RETURNS_TEST(46)
IRQ_STUB_RETURNS_TEST(47)

/*
 * arch_x86_int_frame has to match what the stubs push, and syscall.S writes
 * the syscall result to a hardcoded offset in it. Fields are 4 bytes each:
 * 4 segment registers, 8 from pushal, the vector, the error code, and the 3
 * the CPU pushes.
 */
_Static_assert(offsetof(arch_x86_int_frame, gs) == 0, "frame layout");
_Static_assert(offsetof(arch_x86_int_frame, edi) == 16, "frame layout");
_Static_assert(offsetof(arch_x86_int_frame, eax) == 44, "syscall.S saves the result at 44(%esp)");
_Static_assert(offsetof(arch_x86_int_frame, vector) == 48, "frame layout");
_Static_assert(offsetof(arch_x86_int_frame, error_code) == 52, "frame layout");
_Static_assert(offsetof(arch_x86_int_frame, eip) == 56, "frame layout");
_Static_assert(offsetof(arch_x86_int_frame, eflags) == 64, "frame layout");
_Static_assert(sizeof(arch_x86_int_frame) == 68, "frame layout");

/*
 * What the exception stubs hand to the C handler. Each test sets the general
 * registers and the four data segment registers to distinct known values,
 * triggers a vector, and checks that the frame the handler received holds them,
 * plus the vector, the error code, the return address and the code segment. It
 * also checks that the stack pointer is the same afterwards, which catches a
 * stub that drops the wrong number of slots. ebp is left out as before.
 *
 * Two ways to enter a stub:
 *  - a software "int N", for vectors where the CPU does not push an error code.
 *    A dummy error code of 0 is expected in the frame.
 *  - for the vectors where the CPU does push one (8, 10-14, 17, 21, 29, 30), a
 *    software "int N" would not, so the test builds the frame the CPU would
 *    have (eflags, cs, return address, error code) and jumps to the stub. The
 *    stub's iret then returns to the label after the jump, as it would from a
 *    real fault.
 * The data segment registers get four different values so a stub that stores
 * them in the wrong order shows up.
 *
 * This checks the stubs' stack layout, and that is all. A software "int 2" is
 * not a real NMI and the jump for vector 8 is not a double fault, so delivery
 * of real faults (the CPU choosing the vector, pushing the frame, switching
 * stacks) is not tested here. That needs a handler that can recover from a
 * real fault.
 */
#define XSTR_(x) #x
#define XSTR(x) XSTR_(x)

#define TEST_ERROR_CODE 0x1234
#define KERNEL_CODE_SEL ((int)(GDT_INDEX_KCODE * sizeof(struct gdtdesc)))
/*
 * Four different values, all usable from ring 0. ds must stay flat, as the
 * test uses it for its own memory operands. fs is the user data descriptor
 * again but with a requested privilege level of 3, which is a different value
 * that still loads and is flat. gs is the null selector.
 */
#define FRAME_DS_SEL TEST_DATA_SEL
#define FRAME_ES_SEL KERNEL_DATA_SEL
#define FRAME_FS_SEL (TEST_DATA_SEL | 3)
#define FRAME_GS_SEL 0

/*
 * Fill the stack below the caller with 1 bits, so the slots the stubs push into
 * hold stale non-zero data. A segment register push that leaves the upper half
 * of its slot alone (some CPUs do) would then show up in the frame.
 *
 * This only works because the asm that triggers the interrupt runs in the same
 * function that calls this one, so the stub's pushes land in the region that was
 * just dirtied (the 1KB here is more than the 68 byte frame plus the C handler's
 * own stack use). If that asm ever moves into a deeper helper, its pushes would
 * land lower down, in untouched stack, and the check would quietly stop meaning
 * anything: call dirty_stack() from that same depth.
 */
static void __attribute__((noinline)) dirty_stack(void)
{
	volatile uint32_t scratch[256];
	int i;
	for (i = 0; i < 256; i++) {
		scratch[i] = 0xffffffff;
	}
}

#define FRAME_TEST(test_name, trigger, expected_error, expected_vector) \
	KTEST(test_name) \
	{ \
		int eax = 0x66666666, ebx = 0x11111111, ecx = 0x22222222; \
		int edx = 0x33333333, esi = 0x44444444, edi = 0x55555555; \
		uint16_t dsel = FRAME_DS_SEL, esel = FRAME_ES_SEL; \
		uint16_t fsel = FRAME_FS_SEL, gsel = FRAME_GS_SEL; \
		uint16_t kernel_sel = KERNEL_DATA_SEL; \
		uint16_t ds, es, fs, gs; \
		uint32_t after, esp_before, esp_after; \
		memset(&hook_frame, 0xff, sizeof(hook_frame)); \
		IRQ_GUARD(irq_flags); \
		dirty_stack(); \
		asm volatile ( \
			"movw %[dsel], %%ds\n\t" \
			"movw %[esel], %%es\n\t" \
			"movw %[fsel], %%fs\n\t" \
			"movw %[gsel], %%gs\n\t" \
			"movl $1f, %[after]\n\t" \
			"movl %%esp, %[esp_before]\n\t" \
			trigger "\n\t" \
			"1:\n\t" \
			"movl %%esp, %[esp_after]\n\t" \
			"movw %%ds, %[ds]\n\t" \
			"movw %%es, %[es]\n\t" \
			"movw %%fs, %[fs]\n\t" \
			"movw %%gs, %[gs]\n\t" \
			"movw %[ksel], %%ds\n\t" \
			"movw %[ksel], %%es\n\t" \
			"movw %[ksel], %%fs\n\t" \
			"movw %[ksel], %%gs" \
			: "+a" (eax), "+b" (ebx), "+c" (ecx), "+d" (edx), "+S" (esi), "+D" (edi), \
			  [after] "=m" (after), [esp_before] "=m" (esp_before), [esp_after] "=m" (esp_after), \
			  [ds] "=m" (ds), [es] "=m" (es), [fs] "=m" (fs), [gs] "=m" (gs) \
			: [dsel] "m" (dsel), [esel] "m" (esel), [fsel] "m" (fsel), [gsel] "m" (gsel), \
			  [ksel] "m" (kernel_sel) \
			: "memory"); \
		/* First, so that "the stub never ran" is not reported as a bad field */ \
		KTEST_ASSERT_EQ_INT(hook_frame.vector, expected_vector); \
		KTEST_ASSERT_EQ_INT(hook_frame.error_code, expected_error); \
		KTEST_ASSERT_EQ_INT(hook_frame.eip, after); \
		KTEST_ASSERT_EQ_INT(hook_frame.cs, KERNEL_CODE_SEL); \
		KTEST_ASSERT_EQ_INT(hook_frame.eax, 0x66666666); \
		KTEST_ASSERT_EQ_INT(hook_frame.ebx, 0x11111111); \
		KTEST_ASSERT_EQ_INT(hook_frame.ecx, 0x22222222); \
		KTEST_ASSERT_EQ_INT(hook_frame.edx, 0x33333333); \
		KTEST_ASSERT_EQ_INT(hook_frame.esi, 0x44444444); \
		KTEST_ASSERT_EQ_INT(hook_frame.edi, 0x55555555); \
		KTEST_ASSERT_EQ_INT(hook_frame.ds, FRAME_DS_SEL); \
		KTEST_ASSERT_EQ_INT(hook_frame.es, FRAME_ES_SEL); \
		KTEST_ASSERT_EQ_INT(hook_frame.fs, FRAME_FS_SEL); \
		KTEST_ASSERT_EQ_INT(hook_frame.gs, FRAME_GS_SEL); \
		KTEST_ASSERT_EQ_INT(esp_after, esp_before); \
		KTEST_ASSERT_EQ_INT(eax, 0x66666666); \
		KTEST_ASSERT_EQ_INT(ebx, 0x11111111); \
		KTEST_ASSERT_EQ_INT(ecx, 0x22222222); \
		KTEST_ASSERT_EQ_INT(edx, 0x33333333); \
		KTEST_ASSERT_EQ_INT(esi, 0x44444444); \
		KTEST_ASSERT_EQ_INT(edi, 0x55555555); \
		KTEST_ASSERT_EQ_INT(ds, FRAME_DS_SEL); \
		KTEST_ASSERT_EQ_INT(es, FRAME_ES_SEL); \
		KTEST_ASSERT_EQ_INT(fs, FRAME_FS_SEL); \
		KTEST_ASSERT_EQ_INT(gs, FRAME_GS_SEL); \
	}

#define NOERR_VECTOR_FRAME_TEST(vec) \
	FRAME_TEST(vector_##vec##_frame_has_cpu_state_and_dummy_error_code, "int $" #vec, 0, vec)

#define ERR_VECTOR_FRAME_TEST(vec) \
	FRAME_TEST(vector_##vec##_frame_has_cpu_error_code, \
		"pushfl\n\tpushl %%cs\n\tpushl $1f\n\tpushl $" XSTR(TEST_ERROR_CODE) "\n\tjmp _asm_int_" #vec, \
		TEST_ERROR_CODE, vec)

/* Exceptions where the CPU does not push an error code */
NOERR_VECTOR_FRAME_TEST(0)
NOERR_VECTOR_FRAME_TEST(1)
NOERR_VECTOR_FRAME_TEST(2)
NOERR_VECTOR_FRAME_TEST(3)
NOERR_VECTOR_FRAME_TEST(4)
NOERR_VECTOR_FRAME_TEST(5)
NOERR_VECTOR_FRAME_TEST(6)
NOERR_VECTOR_FRAME_TEST(7)
NOERR_VECTOR_FRAME_TEST(9)
NOERR_VECTOR_FRAME_TEST(15)
NOERR_VECTOR_FRAME_TEST(16)
NOERR_VECTOR_FRAME_TEST(18)
NOERR_VECTOR_FRAME_TEST(19)
NOERR_VECTOR_FRAME_TEST(20)
NOERR_VECTOR_FRAME_TEST(22)
NOERR_VECTOR_FRAME_TEST(23)
NOERR_VECTOR_FRAME_TEST(24)
NOERR_VECTOR_FRAME_TEST(25)
NOERR_VECTOR_FRAME_TEST(26)
NOERR_VECTOR_FRAME_TEST(27)
NOERR_VECTOR_FRAME_TEST(28)
NOERR_VECTOR_FRAME_TEST(31)

/* Exceptions where the CPU pushes an error code */
ERR_VECTOR_FRAME_TEST(8)
ERR_VECTOR_FRAME_TEST(10)
ERR_VECTOR_FRAME_TEST(11)
ERR_VECTOR_FRAME_TEST(12)
ERR_VECTOR_FRAME_TEST(13)
ERR_VECTOR_FRAME_TEST(14)
ERR_VECTOR_FRAME_TEST(17)
ERR_VECTOR_FRAME_TEST(21)
ERR_VECTOR_FRAME_TEST(29)
ERR_VECTOR_FRAME_TEST(30)

/* The value the syscall handler is about to return, from its result hook */
static volatile int hook_syscall_result;

void ktest_syscall_result_hook(int result)
{
	hook_syscall_result = result;
}

/*
 * The syscall gate: the syscall number (eax) shows up as the frame's vector,
 * the handler's return value replaces eax, and the other registers survive.
 *
 * The result is read from the handler's result hook, so the syscall runs only
 * once. The check is that whatever the handler returned is what the caller sees
 * in eax. Whether the result is the right one for the syscall is up to tests of
 * the handler itself.
 *
 * The check needs the result to differ from the syscall number that was in eax
 * going in, because otherwise "eax was overwritten with the result" and "eax
 * was left alone" look the same. A handler that returns its argument (an echo)
 * makes this test unable to tell, and it says so rather than passing.
 */
#define SYSCALL_TEST_NUMBER 7
/* Marks "the result hook never ran", which is not any value a handler returns */
#define SYSCALL_RESULT_UNSET ((int)0xdeadbeef)

KTEST(syscall_frame_has_number_and_result_replaces_eax)
{
	int eax = SYSCALL_TEST_NUMBER, ebx = 0x11111111, ecx = 0x22222222;
	int edx = 0x33333333, esi = 0x44444444, edi = 0x55555555;
	hook_vector = 0;
	hook_syscall_result = SYSCALL_RESULT_UNSET;
	memset(&hook_frame, 0xff, sizeof(hook_frame));
	asm volatile ("int $0x80"
		: "+a" (eax), "+b" (ebx), "+c" (ecx), "+d" (edx), "+S" (esi), "+D" (edi)
		: : "memory");
	KTEST_ASSERT_EQ_INT(hook_vector, 0x80);
	KTEST_ASSERT_EQ_INT(hook_frame.vector, SYSCALL_TEST_NUMBER);
	KTEST_ASSERT_EQ_INT(hook_frame.error_code, 0);
	KTEST_ASSERT_EQ_INT(hook_frame.eax, SYSCALL_TEST_NUMBER);
	KTEST_ASSERT_EQ_INT(hook_frame.ebx, 0x11111111);
	KTEST_ASSERT_EQ_INT(hook_frame.edi, 0x55555555);
	KTEST_ASSERT_EQ_INT(ebx, 0x11111111);
	KTEST_ASSERT_EQ_INT(ecx, 0x22222222);
	KTEST_ASSERT_EQ_INT(edx, 0x33333333);
	KTEST_ASSERT_EQ_INT(esi, 0x44444444);
	KTEST_ASSERT_EQ_INT(edi, 0x55555555);

	/* Before the comparison below, so a hook that never ran is not misreported */
	if (hook_syscall_result == SYSCALL_RESULT_UNSET) {
		KTEST_FAIL("the syscall handler never called ktest_syscall_result_hook\n");
		return;
	}
	if (hook_syscall_result == SYSCALL_TEST_NUMBER) {
		KTEST_FAIL("the handler returned the syscall number, so this test cannot tell the result "
			"from an untouched eax: change SYSCALL_TEST_NUMBER (a test limitation, not a handler bug)\n");
		return;
	}
	KTEST_ASSERT_EQ_INT(eax, hook_syscall_result);
}
