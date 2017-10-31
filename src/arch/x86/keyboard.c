#include <arch/x86/common.h>
#include <arch/x86/keyboard.h>
#include <core/keyboard.h>

extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(PIC1_DATA_ADDR, 0xFD);
}

void keyboard_handler_main(void) {
	unsigned char status;
	unsigned char keycode;
	unsigned int key;
	int released = 0;
	int modifier = 0;

	/* write EOI */
	write_port(PIC1_COMMAND_ADDR, PIC_EOI_ACK);

	while(1) {
		status = read_port(KEYBOARD_STATUS_PORT);
		/* Lowest bit of status will be set if buffer is not empty */
		if (!(status & 0x01)) {
			break;
		}
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode == KEYBOARD_SET1_EXTENDED_PREFIX) {
			modifier = KEYBOARD_SET1_EXTENDED_PREFIX;
			continue;
		}			
		if(keycode & KEYBOARD_KEY_RELEASED_FLAG) {
			// Strip off "released" flag to get real value
			keycode = keycode & ~KEYBOARD_KEY_RELEASED_FLAG;
			released = 1;
		}
		key = keyboard_map[keycode];
		if(modifier == KEYBOARD_SET1_EXTENDED_PREFIX) {
			key = keyboard_map_e0[keycode];
			modifier = 0;
		}
		keyboard_event_handler(key, released ? KEYBOARD_KEY_STATE_UP : KEYBOARD_KEY_STATE_DOWN);
	}
}
