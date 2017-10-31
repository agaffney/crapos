#include <core/keyboard.h>
#include <core/video.h>

void keyboard_event_handler(int key, int state) {
	char buf[20];

	itoa((int)key, buf, 16);
	kprint(buf);
	kprint("  ");
	video_putch(key);
	kprint(" (");
	itoa(key, buf, 16);
	kprint(buf);
	kprint(")");
	if(state == KEYBOARD_KEY_STATE_UP) {
		kprint(" (released)");
	}
	kprint_newline();
	if(key == 'c') {
		video_clear();
	}
}
