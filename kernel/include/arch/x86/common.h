#ifndef _X86_COMMON_H
#define _X86_COMMON_H

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

#define IDT_SIZE 256
#define IDT_TYPE_INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

// Defined via ASM
extern void load_idt(unsigned long *);
extern void keyboard_handler(void);
extern char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

void idt_init(void);

#endif
