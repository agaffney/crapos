#include <arch/x86/gdt.h>
#include <arch/x86/int.h>
#include <ktest/ktest.h>
#include <stdint.h>

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
 * interrupt. It is also only called from _int_handler_default, so the syscall
 * stub (see syscall_returns_to_caller above) and the timer and keyboard stubs
 * are not covered by the segment register checks.
 */
#define TEST_DATA_SEL ((int)(GDT_INDEX_UDATA * sizeof(struct gdtdesc)))
#define KERNEL_DATA_SEL ((int)(GDT_INDEX_KDATA * sizeof(struct gdtdesc)))

/* SAVE_REGS (kernel/include/arch/x86/regs.h) hardcodes this selector */
_Static_assert(KERNEL_DATA_SEL == 0x10, "SAVE_REGS loads a different selector than the GDT's kernel data one");

/* Segment registers as seen from inside the interrupt handler */
static volatile uint16_t hook_ds, hook_es, hook_fs, hook_gs;
static volatile uint32_t hook_vector;

void ktest_int_hook(uint32_t vector)
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
}

#define IRQ_STUB_RETURNS_TEST(vec) \
	KTEST(int_##vec##_returns_with_registers_preserved) \
	{ \
		int eax = 0x66666666, ebx = 0x11111111, ecx = 0x22222222; \
		int edx = 0x33333333, esi = 0x44444444, edi = 0x55555555; \
		uint16_t test_sel = TEST_DATA_SEL, kernel_sel = KERNEL_DATA_SEL; \
		uint16_t ds, es, fs, gs; \
		uint32_t flags; \
		/* Sentinels, so a hook that never ran can't leave stale values behind */ \
		hook_vector = 0; \
		hook_ds = hook_es = hook_fs = hook_gs = 0xffff; \
		/* Keep a real IRQ from arriving while the selectors are unusual */ \
		asm volatile ("pushf\n\tpopl %0\n\tcli" : "=r" (flags) : : "memory"); \
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
		/* Back to whatever the interrupt flag was */ \
		if (flags & (1 << 9)) { \
			asm volatile ("sti" : : : "memory"); \
		} \
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
