#include <arch/x86/common.h>
#include <arch/x86/keyboard.h>

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

extern unsigned int current_loc;
extern char *vidptr;

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(PIC1_DATA_ADDR, 0xFD);
}

void keyboard_handler_main(void) {
	unsigned char status;
	unsigned char keycode;
	char addr[20];
	int released = 0;

	/* write EOI */
	write_port(PIC1_COMMAND_ADDR, PIC_EOI_ACK);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
//		if(keycode < 0)
//			return;
		if(keycode & KEYBOARD_KEY_RELEASED_FLAG && keycode != KEYBOARD_SET1_EXTENDED_PREFIX) {
			// Strip off "released" flag to get real value
			keycode = keycode & ~KEYBOARD_KEY_RELEASED_FLAG;
			released = 1;
		}
		if(keyboard_map[keycode] == 'c') {
			clear_screen();
			return;
		}
		itoa((int)keycode, addr, 16);
		kprint(addr);
		kprint("  ");
		vidptr[current_loc++] = keyboard_map[keycode];
		vidptr[current_loc++] = 0x07;
		if(released) {
			kprint(" (released)");
			released = 0;
		}
		kprint_newline();
	}
}
