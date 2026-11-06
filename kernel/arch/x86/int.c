#include <arch/x86/idt.h>
#include <arch/x86/int.h>
#include <arch/x86/io.h>
#include <core/kprint.h>

// ktest_int_hook only exists in KTEST=1 builds; weak, so it is null otherwise
#pragma weak ktest_int_hook

void _int_handler_default(uint32_t code) {
	if (ktest_int_hook) {
		ktest_int_hook(code);
	}
	kdebug("received interrupt: %d\n", code);
	if(code >= PIC2_OFFSET_ADDR) {
		kdebug("IRQ %d\n", code - PIC2_OFFSET_ADDR + 8);
	} else if(code >= PIC1_OFFSET_ADDR) {
		kdebug("IRQ %d\n", code - PIC1_OFFSET_ADDR);
	}
}
