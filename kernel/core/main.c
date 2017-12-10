#include <core/keyboard.h>
#include <core/kprint.h>
#include <core/video.h>
#include <stdlib.h>
#include <stdio.h>

extern void arch_init(void);

void test_keyboard_event_handler(int, int);

void kmain(void)
{
	arch_init();
	video_init();

//	const char *str = "my first kernel with keyboard support";
	video_clear();
	kprint("Welcome to CrapOS!\n\n\n");

//	register_keyboard_event_handler(test_keyboard_event_handler);

	while(1);
}

void test_keyboard_event_handler(int key, int state) {
/*
	itoa((int)key, buf, 16);
	kprint(buf);
	kprint("  ");
	video_putch(key);
	kprint(" (");
	itoa(key, buf, 16);
	kprint(buf);
	kprint(")");
*/
	kprint("0x%x %c (0x%x)", key, (char)key, key);
	if(state == KEYBOARD_KEY_STATE_UP) {
		kprint(" (released)");
	}
	kprint("\n");
	if(key == 'c') {
		video_clear();
	}
}
