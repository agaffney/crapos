#include <arch/x86/idt.h>
#include <arch/x86/int.h>
#include <arch/x86/io.h>
#include <core/kprint.h>

void _int_handler_default(uint32_t code) {
	kdebug("received interrupt: %d\n", code);
	if(code >= PIC2_OFFSET_ADDR) {
		kdebug("IRQ %d\n", code - PIC2_OFFSET_ADDR + 8);
		/* write EOI */
		outb(PIC2_COMMAND_ADDR, PIC_EOI_ACK);
	} else if(code >= PIC1_OFFSET_ADDR) {
		kdebug("IRQ %d\n", code - PIC1_OFFSET_ADDR);
		outb(PIC1_COMMAND_ADDR, PIC_EOI_ACK);
	}
}
