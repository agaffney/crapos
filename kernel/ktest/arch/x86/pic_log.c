#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <ktest/irq_guard.h>
#include <ktest/pic_log.h>

static struct pic_write pic_writes[PIC_LOG_SIZE];
static volatile uint32_t log_len;

/* See pic_log_probe_isr_at_eoi() */
static volatile int probe_armed, probe_running, probe_finished;
static volatile uint16_t probe_port;
static volatile uint8_t probe_isr[2];

/*
 * Called by pic.c before every PIC command write. Interrupts are turned off
 * while the log is updated, because a test may call the PIC code with them on,
 * and an interrupt in the middle would otherwise log into the same slot.
 */
void ktest_pic_command_hook(uint16_t port, uint8_t value)
{
	IRQ_GUARD(irq_flags);
	uint32_t n;

	/* The probe's own reads come through here too, and are not logged */
	if (probe_running) {
		return;
	}
	n = log_len;
	if (n < PIC_LOG_SIZE) {
		pic_writes[n].port = port;
		pic_writes[n].value = value;
	}
	log_len = n + 1;

	if (probe_armed && port == probe_port && value == PIC_EOI_ACK) {
		probe_armed = 0;
		probe_running = 1;
		probe_isr[PIC_MASTER] = arch_x86_pic_read_isr(PIC_MASTER);
		probe_isr[PIC_SLAVE] = arch_x86_pic_read_isr(PIC_SLAVE);
		probe_running = 0;
		probe_finished = 1;
	}
}

void pic_log_probe_isr_at_eoi(uint16_t port)
{
	probe_port = port;
	probe_finished = 0;
	probe_armed = 1;
}

int pic_log_probe_done(void)
{
	return probe_finished;
}

uint8_t pic_log_probe_isr(int pic)
{
	return probe_isr[pic];
}

void pic_log_reset(void)
{
	log_len = 0;
	/* A probe armed by an earlier test, that never fired, must not fire now */
	probe_armed = 0;
	probe_finished = 0;
}

uint32_t pic_log_len(void)
{
	return log_len;
}

struct pic_write pic_log_get(uint32_t index)
{
	struct pic_write none = { 0, 0 };
	return index < PIC_LOG_SIZE ? pic_writes[index] : none;
}

uint32_t pic_log_count(uint16_t port, uint8_t value)
{
	uint32_t i, count = 0, n = log_len;
	if (n > PIC_LOG_SIZE) {
		n = PIC_LOG_SIZE;
	}
	for (i = 0; i < n; i++) {
		if (pic_writes[i].port == port && pic_writes[i].value == value) {
			count++;
		}
	}
	return count;
}
