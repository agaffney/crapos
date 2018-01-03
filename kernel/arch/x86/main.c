#include <arch/x86/io.h>
#include <arch/x86/video.h>
#include <arch/x86/keyboard.h>
#include <arch/x86/serial.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/idt.h>
#include <arch/x86/vmm.h>
#include <arch/x86/multiboot.h>
#include <core/kprint.h>
#include <core/cmdline.h>
#include <libk/string.h>

extern multiboot_info_t* _multiboot_info;
extern unsigned int _multiboot_magic_value;

void process_multiboot();

void arch_init(void) {
	x86_video_init();
	init_serial();
	process_multiboot();
	vmm_init();
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
	kdebug("_multiboot_info = %#x\n", _multiboot_info);
	kdebug("flags = %016b\n", _multiboot_info->flags);
	if (_multiboot_info->flags & MULTIBOOT_INFO_MEMORY) {
		kdebug("flags (memory) = %016b\n", (_multiboot_info->flags & MULTIBOOT_INFO_MEMORY));
		kdebug("mem_lower = %uKB, mem_upper = %uKB\n", _multiboot_info->mem_lower, _multiboot_info->mem_upper);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_BOOTDEV) {
		kdebug("flags (bootdev) = %016b\n", (_multiboot_info->flags & MULTIBOOT_INFO_BOOTDEV));
		kdebug("boot_device = %#x\n", _multiboot_info->boot_device);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_CMDLINE) {
		// Copy kernel commandline into global buffer
		strncpy(cmdline, (char *)_multiboot_info->cmdline, KERNEL_CMDLINE_MAX_LEN);
		kprint("Kernel commandline (saved): %s\n", cmdline);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_ELF_SHDR) {
		multiboot_elf_section_header_table_t *multiboot_elf_sec = &(_multiboot_info->u.elf_sec);

		kprint("multiboot_elf_sec: num = %u, size = 0x%x,"
			" addr = 0x%x, shndx = 0x%x\n",
			(unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
			(unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
	}
	if (_multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP) {
		multiboot_memory_map_t *mmap;

		kprint("mmap_addr = 0x%x, mmap_length = 0x%x\n", (unsigned) _multiboot_info->mmap_addr, (unsigned) _multiboot_info->mmap_length);
		for (mmap = (multiboot_memory_map_t *) _multiboot_info->mmap_addr;
		     (unsigned long) mmap < _multiboot_info->mmap_addr + _multiboot_info->mmap_length;
		     mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)))
		kprint(" size = %#x, base_addr = 0x%x-%08x,"
			" length = 0x%x-%08x (%dKB), type = %#x\n",
			(unsigned) mmap->size,
			(unsigned) (mmap->addr >> 32),
			(unsigned) (mmap->addr & 0xffffffff),
			(unsigned) (mmap->len >> 32),
			(unsigned) (mmap->len & 0xffffffff),
			(unsigned) ((mmap->len & 0xffffffff) / 1024),
			(unsigned) mmap->type);
	}
}
