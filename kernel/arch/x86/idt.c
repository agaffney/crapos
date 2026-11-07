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
	idt_init_desc(5, _asm_int_5, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(6, _asm_int_6, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(7, _asm_int_7, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(8, _asm_int_8, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(9, _asm_int_9, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(10, _asm_int_10, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(11, _asm_int_11, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(12, _asm_int_12, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(13, _asm_int_13, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(14, _asm_int_14, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(15, _asm_int_15, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(16, _asm_int_16, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(17, _asm_int_17, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(18, _asm_int_18, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(19, _asm_int_19, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(20, _asm_int_20, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(21, _asm_int_21, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(22, _asm_int_22, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(23, _asm_int_23, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(24, _asm_int_24, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(25, _asm_int_25, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(26, _asm_int_26, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(27, _asm_int_27, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(28, _asm_int_28, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(29, _asm_int_29, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(30, _asm_int_30, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(31, _asm_int_31, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

	// IRQ 0-7
	idt_init_desc(PIC1_OFFSET_ADDR, _asm_int_pit, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 1, keyboard_handler, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 2, _asm_int_34, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 3, _asm_int_35, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 4, _asm_int_36, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 5, _asm_int_37, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 6, _asm_int_38, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC1_OFFSET_ADDR + 7, _asm_int_39, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

	// IRQ 8-15
	idt_init_desc(PIC2_OFFSET_ADDR, _asm_int_40, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 1, _asm_int_41, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 2, _asm_int_42, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 3, _asm_int_43, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 4, _asm_int_44, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 5, _asm_int_45, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 6, _asm_int_46, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);
	idt_init_desc(PIC2_OFFSET_ADDR + 7, _asm_int_47, GDT_INDEX_KCODE, IDT_TYPE_INT_GATE, IDT_ATTR_PRESENT | IDT_ATTR_PRIV_0);

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
	outb(PIC1_DATA_ADDR, PIC_SLAVE_IRQ_MASK);
	outb(PIC2_DATA_ADDR, PIC_SLAVE_IRQ_IDENTITY);

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
	outb(PIC1_DATA_ADDR, 0x0);
	outb(PIC2_DATA_ADDR, 0x0);
	foo = inb(PIC1_DATA_ADDR);
	kdebug("new interrupt mask = %08b\n", foo);
}
