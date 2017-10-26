#include <keyboard_map.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

extern unsigned int current_loc;
extern char *vidptr;

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void keyboard_handler_main(void) {
	unsigned char status;
	unsigned char keycode;
	char addr[20];
	int released = 0;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
//		if(keycode < 0)
//			return;
		// 0xE0 is a scancode prefix in set 1/2
		if(keycode > 0x80 && keycode < 0xE0) {
			// Strip off "released" flag to get real value
			keycode -= 0x80;
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
