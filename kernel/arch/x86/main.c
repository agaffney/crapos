#include <arch/x86/io.h>
#include <arch/x86/video.h>
#include <arch/x86/keyboard.h>
#include <arch/x86/serial.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/idt.h>
#include <arch/x86/vmm.h>
#include <core/kprint.h>

void arch_init(void) {
	x86_video_init();
	init_serial();
	vmm_init();
	idt_init();
	kb_init();
	char buf[20];
	cpuid_vendor_string(buf);
	kprint("CPU vendor: %s\n", buf);
}

