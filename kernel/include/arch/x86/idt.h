#pragma once

#include <arch/x86/gdt.h>
#include <stdint.h>

/*     Ports
*	 PIC1	PIC2
*Command 0x20	0xA0
*Data	 0x21	0xA1
*/

#define PIC1_COMMAND_ADDR 0x20
#define PIC1_DATA_ADDR 0x21
#define PIC1_OFFSET_ADDR 0x20

#define PIC2_COMMAND_ADDR 0xA0
#define PIC2_DATA_ADDR 0xA1
#define PIC2_OFFSET_ADDR 0x28

// Used to signal to PIC that the interrupt is being handled
#define PIC_EOI_ACK 0x20

#define PIC_IRQ0  0xFE
#define PIC_IRQ1  0xFD
#define PIC_IRQ2  0xFB
#define PIC_IRQ3  0xF7
#define PIC_IRQ4  0xEF
#define PIC_IRQ5  0xDF
#define PIC_IRQ6  0xBF
#define PIC_IRQ7  0x7F

#define IDT_SIZE 256
#define IDT_BASE 0x00000000

#define IDT_TYPE_INT_GATE   0x0E
#define IDT_TYPE_TRAP_GATE  0x0F
#define IDT_TYPE_TASK_GATE  0x05

#define IDT_ATTR_PRESENT    (1 << 7)
#define IDT_ATTR_PRIV_0     (0 << 5)
#define IDT_ATTR_PRIV_3     (3 << 5)

struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

// IDT segment descriptor
struct idtdesc {
	uint16_t offset0_15;
	uint16_t selector;
	uint8_t  _unused;
	uint8_t  type_attr;
	uint16_t offset16_31;
} __attribute__ ((packed));

// IDT registry "pointer"
struct idtr {
	uint16_t limit;
	uint32_t base;
} __attribute__ ((packed));
void idt_init(void);

void idt_init_desc(uint8_t idx, void * handler_func, uint16_t selector, uint8_t type, uint8_t attr);
