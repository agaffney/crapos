#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/pic.h>

// ktest_pic_command_hook only exists in KTEST=1 builds; weak, so it is null otherwise
#pragma weak ktest_pic_command_hook

static uint16_t command_port(int pic) {
	return pic == PIC_SLAVE ? PIC2_COMMAND_ADDR : PIC1_COMMAND_ADDR;
}

/* Every command sent to a PIC goes through here, so the tests can see it */
static void pic_command(uint16_t port, uint8_t value) {
	if (ktest_pic_command_hook) {
		ktest_pic_command_hook(port, value);
	}
	outb(port, value);
}

uint8_t arch_x86_pic_read_isr(int pic) {
	pic_command(command_port(pic), PIC_OCW3_READ_ISR);
	return (uint8_t)inb(command_port(pic));
}

static void eoi(int pic) {
	pic_command(command_port(pic), PIC_EOI_ACK);
}

static int irq_pic(uint8_t irq) {
	return irq >= 8 ? PIC_SLAVE : PIC_MASTER;
}

/* Only the last IRQ of each controller can be spurious */
static int could_be_spurious(uint8_t irq) {
	return (irq & 7) == 7;
}

int arch_x86_pic_irq_is_spurious(uint8_t irq) {
	return could_be_spurious(irq) && !(arch_x86_pic_read_isr(irq_pic(irq)) & (1 << 7));
}

void arch_x86_pic_send_eoi_with_isr(uint8_t irq, uint8_t isr) {
	int pic = irq_pic(irq);

	if (could_be_spurious(irq) && !(isr & (1 << 7))) {
		if (pic == PIC_SLAVE) {
			eoi(PIC_MASTER);
		}
		return;
	}

	if (pic == PIC_SLAVE) {
		eoi(PIC_SLAVE);
	}
	eoi(PIC_MASTER);
}

void arch_x86_pic_send_eoi(uint8_t irq) {
	/* The register is only read when it can matter */
	uint8_t isr = could_be_spurious(irq) ? arch_x86_pic_read_isr(irq_pic(irq)) : 0;
	arch_x86_pic_send_eoi_with_isr(irq, isr);
}
