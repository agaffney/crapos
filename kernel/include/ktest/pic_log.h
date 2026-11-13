#pragma once

#include <stdint.h>

/*
 * A log of every command written to a PIC, filled in by the test hook in
 * ktest/arch/x86/pic_log.c. A test resets it, provokes an interrupt, and then
 * checks exactly which writes went to which port, in which order.
 */
/* How many writes the log keeps. Later ones are counted but not stored. */
#define PIC_LOG_SIZE 64

struct pic_write {
	uint16_t port;
	uint8_t value;
	/* The direction flag as the hook saw it, 0 or 1. The C ABI needs it clear. */
	uint8_t df;
};

/*
 * Forget everything logged so far, and disarm the ISR probe below if it is
 * armed. Call it with interrupts off.
 */
void pic_log_reset(void);

/* How many writes were logged since the reset (the log keeps the first PIC_LOG_SIZE) */
uint32_t pic_log_len(void);

/* The i'th write since the reset, for i < pic_log_len(). Zero beyond PIC_LOG_SIZE. */
struct pic_write pic_log_get(uint32_t index);

/* How many logged writes match this port and value */
uint32_t pic_log_count(uint16_t port, uint8_t value);

/*
 * Read both in-service registers from inside the hook, at the moment the next
 * EOI to the controller with command port 'port' is about to be written. That
 * is while the IRQ being handled is genuinely in service, which is the only
 * time the registers are not zero. The probe has run when pic_log_probe_done()
 * is true; pic_log_probe_isr() then gives what each controller reported
 * (pic is PIC_MASTER or PIC_SLAVE). Arm it with interrupts off, after
 * pic_log_reset() (which disarms it).
 */
void pic_log_probe_isr_at_eoi(uint16_t port);
int pic_log_probe_done(void);
uint8_t pic_log_probe_isr(int pic);
