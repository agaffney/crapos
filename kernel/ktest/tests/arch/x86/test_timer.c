#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/pic.h>
#include <arch/x86/timer.h>
#include <ktest/irq_guard.h>
#include <ktest/ktest.h>
#include <ktest/pic_log.h>
#include <stdint.h>

/*
 * Real timer interrupts: the PIT raises IRQ 0, which reaches _asm_int_pit
 * through the PIC. The timer tests unmask IRQ 0 themselves, and mask every
 * other IRQ for as long as they run, so that a key press or the RTC can't
 * arrive and change what they observe. They put both PIC masks back
 * afterwards, so they check the timer even if the boot code's unmasking
 * changes. That unmasking has its own test at the end.
 *
 * The PIT is left running by the firmware at about 18.2 Hz, one tick every
 * 55ms. The waits below are bounded by an iteration count, not by the tick
 * counter that is being tested, and are far longer than a tick.
 */
#define TICK_WAIT_LIMIT 300000000u
/* Ticks to wait for. More than one proves the PIC was told the first was handled. */
#define TICKS_TO_WAIT 3

#define PIC_IRQ0_BIT 0x01

/* The interrupt masks of both PICs */
struct pic_masks {
	uint8_t master, slave;
};

/* Unmask IRQ 0 and mask every other IRQ on both PICs. Returns the masks from before. */
static inline struct pic_masks pics_unmask_only_irq0(void)
{
	struct pic_masks old = {
		(uint8_t)inb(PIC1_DATA_ADDR), (uint8_t)inb(PIC2_DATA_ADDR)
	};
	outb(PIC1_DATA_ADDR, (uint8_t)~PIC_IRQ0_BIT);
	outb(PIC2_DATA_ADDR, 0xff);
	return old;
}

/* Put both PIC masks back to what pics_unmask_only_irq0() found. */
static inline void pics_mask_restore(struct pic_masks * old)
{
	outb(PIC1_DATA_ADDR, old->master);
	outb(PIC2_DATA_ADDR, old->slave);
}

/* Interrupts enabled and only IRQ 0 unmasked for as long as the variables live */
#define TIMER_RUNNING() \
	uint32_t irq_flags __attribute__((cleanup(ktest_irq_restore))) = ktest_irq_save_enable(); \
	struct pic_masks pic_masks __attribute__((cleanup(pics_mask_restore))) = pics_unmask_only_irq0()

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
	isr = arch_x86_pic_read_isr(PIC_MASTER);
	KTEST_ASSERT_EQ_INT(isr & PIC_IRQ0_BIT, 0);
}

/*
 * While a tick is being handled, IRQ 0 is in service on the master (bit 0) and
 * nothing is on the slave. The PIC command hook reads both registers just before
 * the EOI goes out, which is the only time they are not zero. This is what
 * shows that arch_x86_pic_read_isr() reads the right register of the right controller.
 */
KTEST(timer_irq_is_in_service_while_it_is_handled)
{
	uint32_t remaining = TICK_WAIT_LIMIT;
	TIMER_RUNNING();
	asm volatile ("cli" : : : "memory");
	pic_log_reset();
	pic_log_probe_isr_at_eoi(PIC1_COMMAND_ADDR);
	asm volatile ("sti" : : : "memory");
	while (!pic_log_probe_done() && --remaining) {
		asm volatile ("" : : : "memory");
	}
	asm volatile ("cli" : : : "memory");
	KTEST_ASSERT_TRUE(pic_log_probe_done());
	KTEST_ASSERT_EQ_INT(pic_log_probe_isr(PIC_MASTER), PIC_IRQ0_BIT);
	KTEST_ASSERT_EQ_INT(pic_log_probe_isr(PIC_SLAVE), 0);
}

/*
 * Each tick is acknowledged exactly once, and only on the master PIC. A second
 * EOI would end a different IRQ that is in service, and a missing one would
 * stop the timer after the first tick. So the PIC command log must hold exactly
 * one master EOI per tick, and nothing else.
 */
KTEST(timer_tick_sends_exactly_one_eoi)
{
	uint32_t ticks_before, i, n;
	TIMER_RUNNING();
	KTEST_ASSERT_TRUE(wait_for_tick(_clock_ticks));
	asm volatile ("cli" : : : "memory");
	ticks_before = _clock_ticks;
	pic_log_reset();
	asm volatile ("sti" : : : "memory");
	KTEST_ASSERT_TRUE(wait_for_tick(ticks_before));
	KTEST_ASSERT_TRUE(wait_for_tick(_clock_ticks));
	asm volatile ("cli" : : : "memory");
	n = _clock_ticks - ticks_before;
	KTEST_ASSERT_TRUE(n >= 2);
	KTEST_ASSERT_EQ_INT(pic_log_len(), n);
	for (i = 0; i < n; i++) {
		KTEST_ASSERT_EQ_INT(pic_log_get(i).port, PIC1_COMMAND_ADDR);
		KTEST_ASSERT_EQ_INT(pic_log_get(i).value, PIC_EOI_ACK);
	}
}

/*
 * A timer interrupt arrives in the middle of running code and must leave every
 * register as it found it. The stub calls C code to send the EOI, and that code
 * may change eax, ecx and edx, so the stub has to save and restore them. eax,
 * edx, esi and edi are checked. ecx is the wait counter and ebx or whichever
 * register holds the start value is the compiler's, so those two are not.
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
