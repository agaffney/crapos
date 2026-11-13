#pragma once

#include <stdint.h>

/*
 * 8259 PIC support shared by every IRQ handler.
 *
 * Each IRQ handler finishes by calling arch_x86_pic_send_eoi() with its IRQ number
 * (0-15). It knows which controllers to tell, in which order, and when an
 * IRQ 7 or 15 is spurious and must not be acknowledged.
 */

/* Which controller: 'pic' arguments take one of these */
#define PIC_MASTER 0
#define PIC_SLAVE  1

/* OCW3 command that makes the next read of the command port return the ISR */
#define PIC_OCW3_READ_ISR 0x0b

/*
 * Test hook, defined only by the in-kernel test build (ktest). It is called
 * with the port and value of every command written to a PIC (EOIs and the
 * in-service register read), just before the write. The tests use it to check
 * exactly what was sent where. The normal kernel doesn't define it, and the
 * call is skipped.
 */
void ktest_pic_command_hook(uint16_t port, uint8_t value);

/*
 * Read a controller's in-service register (bit n set while its IRQ n is being
 * handled and has not been acknowledged yet). 'pic' is PIC_MASTER or PIC_SLAVE.
 *
 * This leaves the controller in "read ISR" mode, so a later read of its command
 * port returns the ISR, not the interrupt request register (IRR), until an
 * OCW3 command selects the IRR (0x0a). Nothing here reads the IRR.
 */
uint8_t arch_x86_pic_read_isr(int pic);

/*
 * Acknowledge IRQ 'irq' (0-15). An IRQ on the slave (8-15) needs an EOI to the
 * slave and then to the master, because the slave is wired to master IRQ 2.
 *
 * IRQ 7 and 15 can be spurious: the PIC raises them when a request goes away
 * before the CPU acknowledges it. A spurious IRQ never sets its ISR bit, and
 * must not get an EOI, or the EOI would end a different IRQ that is really in
 * service. A spurious IRQ 15 is the exception: the master did see IRQ 2, so it
 * (and only it) still needs one.
 */
void arch_x86_pic_send_eoi(uint8_t irq);

/*
 * True if IRQ 'irq' is a spurious IRQ 7 or 15 right now: the IRQ is the last
 * on its controller and that controller has nothing in service for it. Reads
 * the in-service register, so call it before the EOI.
 *
 * Call it with interrupts off, as an IRQ handler is. arch_x86_pic_send_eoi() reads the
 * register again, and the two only agree if nothing can change it in between.
 */
int arch_x86_pic_irq_is_spurious(uint8_t irq);

/*
 * The part of arch_x86_pic_send_eoi() that decides what to send, given the in-service
 * register of the IRQ's controller ('isr') instead of reading it. Only IRQ 7
 * and 15 look at it. This exists so that the case of a genuine IRQ 7 or 15,
 * which no device here can raise, can be tested.
 */
void arch_x86_pic_send_eoi_with_isr(uint8_t irq, uint8_t isr);
