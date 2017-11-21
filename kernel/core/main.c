#include <core/keyboard.h>
#include <core/video.h>
#include <stdlib.h>
#include <stdio.h>

extern void arch_init(void);

void test_keyboard_event_handler(int, int);

void kmain(void)
{
	arch_init();
	video_init();

	const char *str = "my first kernel with keyboard support";
	video_clear();
	kprint(str);
	kprint_newline();
	kprint_newline();

	register_keyboard_event_handler(test_keyboard_event_handler);

	while(1);
}

void test_keyboard_event_handler(int key, int state) {
	char buf[20];

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
	sprintf(buf, "0x%x %c (0x%x)", key, (char)key, key);
	kprint(buf);
	if(state == KEYBOARD_KEY_STATE_UP) {
		kprint(" (released)");
	}
	kprint_newline();
	if(key == 'c') {
		video_clear();
	}
}
