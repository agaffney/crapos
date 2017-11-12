#include <core/keyboard_keys.h>

/*
   Set 1 codes
   http://www.quadibloc.com/comp/scan.htm
*/

/* The following array is taken from 
    http://www.osdever.net/bkerndev/Docs/keyboard.htm
   All credits where due
*/
unsigned int keyboard_map[128] =
{
    0,  KEYBOARD_KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    KEYBOARD_KEY_LCTRL,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   KEYBOARD_KEY_LSHIFT,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   KEYBOARD_KEY_RSHIFT,
  '*',
    KEYBOARD_KEY_LALT,
  ' ',	/* Space bar */
    KEYBOARD_KEY_CAPS_LOCK,
    KEYBOARD_KEY_F1,
    KEYBOARD_KEY_F2,
    KEYBOARD_KEY_F3,
    KEYBOARD_KEY_F4,
    KEYBOARD_KEY_F5,
    KEYBOARD_KEY_F6,
    KEYBOARD_KEY_F7,
    KEYBOARD_KEY_F8,
    KEYBOARD_KEY_F9,
    KEYBOARD_KEY_F10,
    KEYBOARD_KEY_NUM_LOCK,
    KEYBOARD_KEY_SCROLL_LOCK,
    KEYBOARD_KEY_HOME,
    KEYBOARD_KEY_UP_ARROW,
    KEYBOARD_KEY_PGUP,
  '-',
    KEYBOARD_KEY_LEFT_ARROW,
    0,
    KEYBOARD_KEY_RIGHT_ARROW,
  '+',
    KEYBOARD_KEY_END,
    KEYBOARD_KEY_DOWN_ARROW,
    KEYBOARD_KEY_PGDOWN,
    KEYBOARD_KEY_INSERT,
    KEYBOARD_KEY_DELETE,
    KEYBOARD_KEY_SYSRQ,   0,   0,
    KEYBOARD_KEY_F11,
    KEYBOARD_KEY_F12,
    0,	/* All other keys are undefined */
};

unsigned int keyboard_map_e0[128] =
{
    // 0-27 are unused with the E0 modifier
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEYBOARD_KEY_ENTER_KP,
    KEYBOARD_KEY_RCTRL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEYBOARD_KEY_RALT,
};
