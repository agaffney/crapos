#include <arch/x86/io.h>
#include <arch/x86/idt.h>
#include <arch/x86/int.h>
#include <arch/x86/syscall.h>
#include <arch/x86/timer.h>
#include <core/kprint.h>
#include <libk/string.h>

struct idtdesc kidt[IDT_SIZE];
struct idtr kidtr;

void idt_init_desc(uint8_t idx, void * handler_func, uint16_t selector, uint8_t type, uint8_t attr) {
	kdebug("idx=%d, func=%#x, selector=%d, type=%d, attr=%d\n", idx, handler_func, selector, type, attr);
	kidt[idx].offset0_15 = (uint32_t)handler_func & 0xffff;
	kidt[idx].selector = selector * sizeof(struct gdtdesc);
	kidt[idx].type_attr = type | attr;
	kidt[idx].offset16_31 = ((uint32_t)handler_func & 0xffff0000) >> 16;
}

void idt_init(void) {
	// CPU interrupts
	idt_init_desc(0, _asm_int_0, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(1, _asm_int_1, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(2, _asm_int_2, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(3, _asm_int_3, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(4, _asm_int_4, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

	// IRQ 0-7
	idt_init_desc(PIC1_OFFSET_ADDR, _asm_int_pit, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 1, keyboard_handler, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 2, _asm_int_34, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 3, _asm_int_35, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 4, _asm_int_36, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 5, _asm_int_37, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 6, _asm_int_38, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 7, _asm_int_39, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

	// Syscall
	idt_init_desc(INT_SYSCALL, _asm_int_syscall, GDT_INDEX_KCODE, IDT_TYPE_TRAP_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

	/* ICW1 - begin initialization */
	outb(PIC1_COMMAND_ADDR, 0x11);
	outb(PIC2_COMMAND_ADDR, 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	outb(PIC1_DATA_ADDR, PIC1_OFFSET_ADDR);
	outb(PIC2_DATA_ADDR, PIC2_OFFSET_ADDR);

	/* ICW3 - setup cascading */
	outb(PIC1_DATA_ADDR, 0x00);
	outb(PIC2_DATA_ADDR, 0x00);

	/* ICW4 - environment info */
	outb(PIC1_DATA_ADDR, 0x01);
	outb(PIC2_DATA_ADDR, 0x01);
	/* Initialization finished */

	/* mask interrupts */
	outb(PIC1_DATA_ADDR, 0xff);
	outb(PIC2_DATA_ADDR, 0xff);

	kidtr.base = IDT_BASE;
	kidtr.limit = (sizeof(struct idtdesc) * IDT_SIZE) - 1;

	// copy the gdtr to its memory area
	memcpy((char *) kidtr.base, (char *) kidt, kidtr.limit + 1);

	// Load IDT
	asm volatile ("lidt (kidtr)");

	// Enable interrupts
	asm volatile ("sti");

	// Unmask interrupts
	uint8_t foo;
	foo = inb(PIC1_DATA_ADDR);
	kdebug("current interrupt mask = %08b\n", foo);
	outb(PIC1_DATA_ADDR, PIC_IRQ1 & PIC_IRQ2 & PIC_IRQ3 & PIC_IRQ4 & PIC_IRQ7);
	foo = inb(PIC1_DATA_ADDR);
	kdebug("new interrupt mask = %08b\n", foo);
}
