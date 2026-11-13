#pragma once

#include <stdint.h>

/*
 * Helpers for tests that care whether interrupts are on.
 *
 * ktest_irq_save_disable() turns interrupts off and ktest_irq_save_enable()
 * turns them on. Both return the flags from before, and ktest_irq_restore()
 * puts the interrupt flag back to exactly that: on if it was on, off if it was
 * off.
 *
 * IRQ_GUARD(name) is for tests that must not be interrupted by a real IRQ, for
 * example because they load unusual segment selectors or compare PIC logs. It
 * turns interrupts off and, through the cleanup attribute, restores them when
 * the variable goes out of scope, on every way out of the test: the normal end,
 * or an assertion returning early.
 */
static inline uint32_t ktest_irq_save_disable(void)
{
	uint32_t flags;
	asm volatile ("pushf\n\tpopl %0\n\tcli" : "=r" (flags) : : "memory");
	return flags;
}

static inline uint32_t ktest_irq_save_enable(void)
{
	uint32_t flags;
	asm volatile ("pushf\n\tpopl %0\n\tsti" : "=r" (flags) : : "memory");
	return flags;
}

static inline void ktest_irq_restore(uint32_t * flags)
{
	if (*flags & (1 << 9)) {
		asm volatile ("sti" : : : "memory");
	} else {
		asm volatile ("cli" : : : "memory");
	}
}

#define IRQ_GUARD(name) \
	uint32_t name __attribute__((cleanup(ktest_irq_restore))) = ktest_irq_save_disable()
