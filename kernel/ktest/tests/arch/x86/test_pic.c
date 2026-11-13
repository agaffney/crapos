#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/pic.h>
#include <ktest/irq_guard.h>
#include <ktest/ktest.h>
#include <ktest/pic_log.h>
#include <stdint.h>

/*
 * Tests for arch_x86_pic_send_eoi() and the EOIs the IRQ stubs send.
 *
 * Every command written to a PIC is logged by the test hook (see pic_log.h),
 * so these tests check exactly which writes went to which port, in which order.
 * That is what the PIC sees, so a double EOI, an EOI to the wrong controller or
 * a wrong command byte all show up.
 *
 * A software "int N" runs the stub for vector N without the PIC being
 * involved, so the IRQ's bit in the in-service register is never set. For IRQ
 * 7 and 15 that looks exactly like a spurious interrupt, which is how those
 * paths are tested here. Interrupts are turned off around each one, so that a
 * real timer tick can't add to the log in the middle.
 */

#define MASTER PIC1_COMMAND_ADDR
#define SLAVE PIC2_COMMAND_ADDR

/*
 * Check the whole log: the writes since pic_log_reset(), in order. The values
 * to give are { port, value } pairs.
 */
#define ASSERT_WRITES(...) do { \
	static const struct pic_write want[] = { __VA_ARGS__ }; \
	uint32_t i, n = sizeof(want) / sizeof(want[0]); \
	KTEST_ASSERT_EQ_INT(pic_log_len(), n); \
	for (i = 0; i < n; i++) { \
		KTEST_ASSERT_EQ_INT(pic_log_get(i).port, want[i].port); \
		KTEST_ASSERT_EQ_INT(pic_log_get(i).value, want[i].value); \
	} \
} while (0)

/*
 * Check only the EOIs in the log, in order, by the port each went to. The
 * in-service register reads that IRQ 7 and 15 cause are not looked at.
 */
#define ASSERT_EOIS(...) do { \
	static const uint16_t want[] = { __VA_ARGS__ }; \
	uint32_t i, got = 0, n = sizeof(want) / sizeof(want[0]); \
	for (i = 0; i < pic_log_len() && i < PIC_LOG_SIZE; i++) { \
		if (pic_log_get(i).value == PIC_EOI_ACK) { \
			KTEST_ASSERT_TRUE(got < n); \
			KTEST_ASSERT_EQ_INT(pic_log_get(i).port, want[got]); \
			got++; \
		} \
	} \
	KTEST_ASSERT_EQ_INT(got, n); \
} while (0)

#define ASSERT_NO_EOIS() \
	KTEST_ASSERT_EQ_INT(pic_log_count(MASTER, PIC_EOI_ACK) + pic_log_count(SLAVE, PIC_EOI_ACK), 0)

/* A software interrupt for IRQ 7 is spurious: no EOI to either PIC. */
KTEST(spurious_irq7_sends_no_eoi)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	asm volatile ("int $39" : : : "memory");
	ASSERT_NO_EOIS();
}

/*
 * A spurious IRQ 15 gets an EOI to the master only: the master saw IRQ 2
 * for real, but the slave has nothing in service.
 */
KTEST(spurious_irq15_acknowledges_master_only)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	asm volatile ("int $47" : : : "memory");
	ASSERT_EOIS(MASTER);
}

/*
 * IRQ 7 and 15 ask their own controller which IRQs are in service, with the
 * read-ISR command (0x0b) written to that controller's command port. The
 * handler checks for a spurious IRQ and then arch_x86_pic_send_eoi() does, so there are
 * two reads.
 */
KTEST(spurious_check_reads_isr_from_the_irqs_own_controller)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	asm volatile ("int $39" : : : "memory");
	ASSERT_WRITES({ MASTER, PIC_OCW3_READ_ISR }, { MASTER, PIC_OCW3_READ_ISR });

	pic_log_reset();
	asm volatile ("int $47" : : : "memory");
	ASSERT_WRITES({ SLAVE, PIC_OCW3_READ_ISR }, { SLAVE, PIC_OCW3_READ_ISR },
		{ MASTER, PIC_EOI_ACK });
}

/*
 * A probe that was armed and never fired is disarmed by the next reset, so it
 * can't leak into a later test and fire on that test's EOI.
 */
KTEST(armed_isr_probe_is_disarmed_by_a_reset)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	pic_log_probe_isr_at_eoi(MASTER);
	pic_log_reset();
	asm volatile ("int $34" : : : "memory");
	KTEST_ASSERT_TRUE(!pic_log_probe_done());
	/* Armed and not reset, the same interrupt does fire it */
	pic_log_reset();
	pic_log_probe_isr_at_eoi(MASTER);
	asm volatile ("int $34" : : : "memory");
	KTEST_ASSERT_TRUE(pic_log_probe_done());
}

/* An IRQ on the master (other than 7) gets exactly one EOI, to the master. */
KTEST(master_irq_sends_one_eoi_to_master)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	asm volatile ("int $34" : : : "memory");
	ASSERT_WRITES({ MASTER, PIC_EOI_ACK });
}

/* An IRQ on the slave (other than 15) gets one EOI to each PIC, slave first. */
KTEST(slave_irq_sends_one_eoi_to_each_pic)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	asm volatile ("int $41" : : : "memory");
	ASSERT_WRITES({ SLAVE, PIC_EOI_ACK }, { MASTER, PIC_EOI_ACK });
}

/*
 * A genuine IRQ 7 or 15 has its bit set in the in-service register, and must
 * be acknowledged like any other IRQ. No device here can raise one, so these
 * pass the register value in directly. A real EOI goes out with nothing in
 * service, which the PIC ignores.
 */
KTEST(genuine_irq7_is_acknowledged)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	arch_x86_pic_send_eoi_with_isr(7, 0x80);
	ASSERT_WRITES({ MASTER, PIC_EOI_ACK });
}

KTEST(genuine_irq15_is_acknowledged_on_both_pics)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	arch_x86_pic_send_eoi_with_isr(15, 0x80);
	ASSERT_WRITES({ SLAVE, PIC_EOI_ACK }, { MASTER, PIC_EOI_ACK });
}

/* Other bits set in the register don't make IRQ 7 or 15 genuine; only bit 7 does. */
KTEST(irq7_with_only_other_bits_in_service_is_spurious)
{
	IRQ_GUARD(irq_flags);
	pic_log_reset();
	arch_x86_pic_send_eoi_with_isr(7, 0x7f);
	ASSERT_NO_EOIS();
}

/* The handler's spurious check agrees: nothing is in service, so IRQ 7 and 15 are spurious. */
KTEST(irq_is_spurious_only_for_idle_irq7_and_irq15)
{
	IRQ_GUARD(irq_flags);
	KTEST_ASSERT_TRUE(arch_x86_pic_irq_is_spurious(7));
	KTEST_ASSERT_TRUE(arch_x86_pic_irq_is_spurious(15));
	KTEST_ASSERT_TRUE(!arch_x86_pic_irq_is_spurious(0));
	KTEST_ASSERT_TRUE(!arch_x86_pic_irq_is_spurious(6));
	KTEST_ASSERT_TRUE(!arch_x86_pic_irq_is_spurious(8));
}

/*
 * The real RTC periodic interrupt is IRQ 8, the first IRQ on the slave. It only
 * reaches the CPU if the cascade through master IRQ 2 works, and it is only
 * acknowledged properly if both PICs get an EOI, the slave's first.
 *
 * The RTC raises its line again only after register C has been read, and the
 * default IRQ handler doesn't read it. So exactly one interrupt arrives while
 * the periodic interrupt is on, which is what the test counts on. Register B is
 * put back to exactly what it was.
 *
 * Real timer ticks keep arriving too, and put master EOIs in the log, so the
 * test looks for the slave EOI and the master EOI right after it.
 */
#define CMOS_INDEX 0x70
#define CMOS_DATA 0x71
#define CMOS_REG_B 0x0b
#define CMOS_REG_C 0x0c
#define CMOS_REG_B_PERIODIC_IRQ (1 << 6)
/* The RTC ticks at 1024Hz by default, so this is far longer than one period */
#define RTC_WAIT_LIMIT 100000000u

static uint8_t cmos_read(uint8_t reg)
{
	outb(CMOS_INDEX, reg);
	return (uint8_t)inb(CMOS_DATA);
}

static void cmos_write(uint8_t reg, uint8_t value)
{
	outb(CMOS_INDEX, reg);
	outb(CMOS_DATA, value);
}

KTEST(rtc_interrupt_arrives_through_the_cascade_and_is_acknowledged)
{
	uint32_t remaining = RTC_WAIT_LIMIT, i, slave_at = 0;
	uint8_t reg_b = cmos_read(CMOS_REG_B);
	uint32_t saved_flags;

	/* Reading register C clears any interrupt flag left pending */
	(void)cmos_read(CMOS_REG_C);

	saved_flags = ktest_irq_save_disable();
	pic_log_reset();
	pic_log_probe_isr_at_eoi(SLAVE);
	asm volatile ("sti" : : : "memory");
	cmos_write(CMOS_REG_B, reg_b | CMOS_REG_B_PERIODIC_IRQ);
	while (pic_log_count(SLAVE, PIC_EOI_ACK) == 0 && --remaining) {
		asm volatile ("" : : : "memory");
	}
	asm volatile ("cli" : : : "memory");

	/* Put the RTC back as it was and clear its flag, so it can't fire again */
	cmos_write(CMOS_REG_B, reg_b);
	(void)cmos_read(CMOS_REG_C);
	ktest_irq_restore(&saved_flags);

	KTEST_ASSERT_TRUE(remaining != 0);
	KTEST_ASSERT_EQ_INT(pic_log_count(SLAVE, PIC_EOI_ACK), 1);
	for (i = 0; i + 1 < pic_log_len() && i + 1 < PIC_LOG_SIZE; i++) {
		if (pic_log_get(i).port == SLAVE && pic_log_get(i).value == PIC_EOI_ACK) {
			slave_at = i + 1;
			KTEST_ASSERT_EQ_INT(pic_log_get(i + 1).port, MASTER);
			KTEST_ASSERT_EQ_INT(pic_log_get(i + 1).value, PIC_EOI_ACK);
		}
	}
	KTEST_ASSERT_TRUE(slave_at != 0);
	/*
	 * While the handler ran, IRQ 8 was in service on the slave (its bit 0) and
	 * so was IRQ 2, the cascade, on the master (bit 2). The probe read both
	 * registers just before the slave's EOI.
	 */
	KTEST_ASSERT_TRUE(pic_log_probe_done());
	KTEST_ASSERT_EQ_INT(pic_log_probe_isr(PIC_SLAVE), 0x01);
	KTEST_ASSERT_EQ_INT(pic_log_probe_isr(PIC_MASTER), 0x04);
	/* Nothing is left in service on either PIC */
	KTEST_ASSERT_EQ_INT(arch_x86_pic_read_isr(PIC_SLAVE), 0);
	KTEST_ASSERT_EQ_INT(arch_x86_pic_read_isr(PIC_MASTER), 0);
}
