#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/timer.h>
#include <ktest/ktest.h>
#include <stdint.h>

/*
 * Real timer interrupts: the PIT raises IRQ 0, which reaches _asm_int_pit
 * through the PIC. The timer tests turn IRQ 0 on themselves and put the PIC
 * mask back afterwards, so they check the timer even if the boot code's
 * unmasking changes. That unmasking has its own test at the end.
 *
 * The PIT is left running by the firmware at about 18.2 Hz, one tick every
 * 55ms. The waits below are bounded by an iteration count, not by the tick
 * counter that is being tested, and are far longer than a tick.
 */
#define TICK_WAIT_LIMIT 300000000u
/* Ticks to wait for. More than one proves the PIC was told the first was handled. */
#define TICKS_TO_WAIT 3

/* PIC1 in-service register, read with the OCW3 command 0x0b */
#define PIC_OCW3_READ_ISR 0x0b
#define PIC_IRQ0_BIT 0x01

/* Interrupts on. The previous state is put back by irq_state_restore(). */
static inline uint32_t irq_state_enable(void)
{
	uint32_t flags;
	asm volatile ("pushf\n\tpopl %0\n\tsti" : "=r" (flags) : : "memory");
	return flags;
}

/* Put the interrupt flag back to exactly what irq_state_enable() found. */
static inline void irq_state_restore(uint32_t * flags)
{
	if (*flags & (1 << 9)) {
		asm volatile ("sti" : : : "memory");
	} else {
		asm volatile ("cli" : : : "memory");
	}
}

/* Unmask IRQ 0 on the master PIC and return the mask from before. */
static inline uint8_t pic1_unmask_irq0(void)
{
	uint8_t old = (uint8_t)inb(PIC1_DATA_ADDR);
	outb(PIC1_DATA_ADDR, old & ~PIC_IRQ0_BIT);
	return old;
}

/* Put the master PIC mask back to what pic1_unmask_irq0() found. */
static inline void pic1_mask_restore(uint8_t * old)
{
	outb(PIC1_DATA_ADDR, *old);
}

/* Interrupts enabled and IRQ 0 unmasked for as long as the variables live */
#define TIMER_RUNNING() \
	uint32_t irq_flags __attribute__((cleanup(irq_state_restore))) = irq_state_enable(); \
	uint8_t pic_mask __attribute__((cleanup(pic1_mask_restore))) = pic1_unmask_irq0()

/*
 * Spin until _clock_ticks is no longer 'start', or the wait limit runs out.
 * Returns non-zero if the counter changed. The loop is in asm so that it
 * re-reads the counter from memory every time round. The kernel is linked at
 * fixed addresses, so the asm names the symbol directly, as timer.s does.
 */
static int wait_for_tick(uint32_t start)
{
	uint32_t remaining = TICK_WAIT_LIMIT;
	asm volatile (
		"1:\n\t"
		"cmpl %[start], _clock_ticks\n\t"
		"jne 2f\n\t"
		"decl %[remaining]\n\t"
		"jnz 1b\n\t"
		"2:"
		: [remaining] "+r" (remaining)
		: [start] "r" (start)
		: "cc", "memory");
	return remaining != 0;
}

/* The tick counter goes up on its own, several times in a row. */
KTEST(timer_ticks_advance)
{
	int i;
	TIMER_RUNNING();
	for (i = 0; i < TICKS_TO_WAIT; i++) {
		uint32_t before = _clock_ticks;
		KTEST_ASSERT_TRUE(wait_for_tick(before));
		KTEST_ASSERT_TRUE(_clock_ticks == before + 1);
	}
}

/*
 * After a tick has been handled, IRQ 0 is no longer in service on the PIC. It
 * stays set until the handler sends the EOI. Interrupts are turned off first so
 * that no new tick can be acknowledged between the wait and the read.
 */
KTEST(timer_tick_is_acknowledged_to_the_pic)
{
	uint8_t isr;
	TIMER_RUNNING();
	KTEST_ASSERT_TRUE(wait_for_tick(_clock_ticks));
	asm volatile ("cli" : : : "memory");
	outb(PIC1_COMMAND_ADDR, PIC_OCW3_READ_ISR);
	isr = (uint8_t)inb(PIC1_COMMAND_ADDR);
	KTEST_ASSERT_EQ_INT(isr & PIC_IRQ0_BIT, 0);
}

/*
 * A timer interrupt arrives in the middle of running code and must leave every
 * register as it found it. The stub uses al for the EOI, so eax is the one that
 * matters; edx (used by the port instructions), esi and edi are checked too.
 * ecx is the wait counter and ebx or whichever register holds the start value
 * is the compiler's, so those two are not.
 */
KTEST(timer_interrupt_preserves_registers)
{
	int eax = 0x66666666, edx = 0x33333333, esi = 0x44444444, edi = 0x55555555;
	uint32_t start, remaining = TICK_WAIT_LIMIT;
	TIMER_RUNNING();
	start = _clock_ticks;
	asm volatile (
		"1:\n\t"
		"cmpl %[start], _clock_ticks\n\t"
		"jne 2f\n\t"
		"decl %[remaining]\n\t"
		"jnz 1b\n\t"
		"2:"
		: "+a" (eax), "+d" (edx), "+S" (esi), "+D" (edi), [remaining] "+c" (remaining)
		: [start] "r" (start)
		: "cc", "memory");
	KTEST_ASSERT_TRUE(remaining != 0);
	KTEST_ASSERT_EQ_INT(eax, 0x66666666);
	KTEST_ASSERT_EQ_INT(edx, 0x33333333);
	KTEST_ASSERT_EQ_INT(esi, 0x44444444);
	KTEST_ASSERT_EQ_INT(edi, 0x55555555);
}

/*
 * The boot code unmasks every IRQ on both PICs (idt_init), so the timer runs
 * from the start and nothing is silently left off. Tests run in alphabetical
 * order and the ones above put the mask back, so this sees the boot state.
 */
KTEST(pics_have_all_irqs_unmasked_after_boot)
{
	KTEST_ASSERT_EQ_INT((uint8_t)inb(PIC1_DATA_ADDR), 0);
	KTEST_ASSERT_EQ_INT((uint8_t)inb(PIC2_DATA_ADDR), 0);
}
