#pragma once

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEYBOARD_KEY_RELEASED_FLAG 0x80
#define KEYBOARD_SET1_EXTENDED_PREFIX 0xE0

void x86_keyboard_handler(void);
void kb_init(void);
