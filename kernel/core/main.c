#include <kernel.h>
#include <core/keyboard.h>
#include <core/kprint.h>
#include <core/video.h>
#include <core/vfs.h>
#include <core/cmdline.h>
#include <core/process.h>
#include <libk/stdlib.h>
#include <libk/stdio.h>

extern void arch_early_init(void);
extern void arch_init(void);

void test_keyboard_event_handler(int, int);

void kmain(void)
{
	arch_early_init();
	kprint(KERNEL_NAME " kernel version " KERNEL_VERSION "\n");
	cmdline_parse();
	arch_init();
	process_init();
	vfs_init();
	video_init();

	video_clear();
	kprint("\n\nWelcome to CrapOS!\n\n\n");

	register_keyboard_event_handler(test_keyboard_event_handler);

/*
	// Test interrupt handler with divide by zero
	int foo = 5 / 0;
	kdebug("%d\n", foo);
*/

/*
	// Test syscall handler
	asm("mov $3, %eax\n\t"
	    "int $0x80");

	kdebug("called syscall 3\n");
*/

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
/*
	if(key == 'c') {
		video_clear();
	}
*/
}
