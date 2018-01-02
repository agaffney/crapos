#include <arch/x86/io.h>
#include <arch/x86/video.h>
#include <arch/x86/keyboard.h>
#include <arch/x86/serial.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/idt.h>
#include <arch/x86/vmm.h>
#include <arch/x86/multiboot.h>
#include <core/kprint.h>

extern multiboot_info_t* _multiboot_info;
extern unsigned int _multiboot_magic_value;

void process_multiboot();

void arch_init(void) {
	vmm_init();
	x86_video_init();
	init_serial();
	process_multiboot();
	idt_init();
	kb_init();
	char buf[20];
	cpuid_vendor_string(buf);
	kprint("CPU vendor: %s\n", buf);
}

void process_multiboot() {
	if (_multiboot_magic_value != MULTIBOOT_BOOTLOADER_MAGIC) {
		kdebug("the multiboot structure does not appear to be valid\n");
		return;
	}
	kdebug("flags = %b\n", _multiboot_info->flags);
	if (_multiboot_info->flags & MULTIBOOT_INFO_MEMORY) {
		kdebug("flags (memory) = %b\n", (_multiboot_info->flags & MULTIBOOT_INFO_MEMORY));
		kdebug("mem_lower = %uKB, mem_upper = %uKB\n", _multiboot_info->mem_lower, _multiboot_info->mem_upper);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_BOOTDEV) {
		kdebug("flags (bootdev) = %b\n", (_multiboot_info->flags & MULTIBOOT_INFO_BOOTDEV));
		kdebug("boot_device = 0x%x\n", _multiboot_info->boot_device);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_CMDLINE) {
		kdebug("flags (cmdline) = %b\n", (_multiboot_info->flags & MULTIBOOT_INFO_CMDLINE));
		kprint("Kernel commandline: %s\n", _multiboot_info->cmdline);
	}
}
