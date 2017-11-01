#include <core/keyboard.h>
#include <core/video.h>

keyboard_event_handler_func keyboard_event_handlers[128];
int keyboard_event_handler_count = 0;

void register_keyboard_event_handler(keyboard_event_handler_func keh) {
	keyboard_event_handlers[keyboard_event_handler_count] = keh;
	keyboard_event_handler_count++;
}

void keyboard_event_handler(int key, int state) {
	int i;

	for(i = 0; i < keyboard_event_handler_count; i++) {
		keyboard_event_handlers[i](key, state);
	}
}

