#include <core/keyboard_keys.h>

typedef void (*keyboard_event_handler_func)(int, int);

void register_keyboard_event_handler(keyboard_event_handler_func);

void keyboard_event_handler(int, int);
