#include <arch/x86/gdt.h>
#include <libk/string.h>
#include <core/kprint.h>

struct gdtdesc kgdt[GDT_SIZE];
struct tss default_tss;
struct gdtr kgdtr;

void gdt_init_desc(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	kgdt[idx].limit0_15 = (limit & 0xffff);
	kgdt[idx].base0_15 = (base & 0xffff);
	kgdt[idx].base16_23 = (base & 0xff0000) >> 16;
	kgdt[idx].access = access;
	kgdt[idx].limit16_19 = (limit & 0xf0000) >> 16;
	kgdt[idx].flags = (flags & 0xf);
	kgdt[idx].base24_31 = (base & 0xff000000) >> 24;
}

void gdt_init() {
	default_tss.debug_flag = 0x00;
	default_tss.io_map = 0x00;
	// TODO: replace with kernel stack pointer
	default_tss.esp0 = 0x1FFF0;
	default_tss.ss0 = sizeof(struct gdtdesc) * (GDT_INDEX_KDATA + 1);

	// null descriptor
	gdt_init_desc(GDT_INDEX_NULL, 0x0, 0x0, 0x0, 0x0);

	// kernel code
	gdt_init_desc(GDT_INDEX_KCODE,
		0x0, 0xFFFFF,
		GDT_ACCESS_KERN_BASE | GDT_ACCESS_EXEC,
		GDT_FLAGS_DEFAULT);
	// kernel data
	gdt_init_desc(GDT_INDEX_KDATA,
		0x0, 0xFFFFF,
		GDT_ACCESS_KERN_BASE,
		GDT_FLAGS_DEFAULT);
	// kernel stack
	gdt_init_desc(GDT_INDEX_KSTACK,
		0x0, 0x0,
		GDT_ACCESS_KERN_BASE | GDT_ACCESS_DC,
		GDT_FLAGS_DEFAULT);

	// userspace code
	gdt_init_desc(GDT_INDEX_UCODE,
		0x0, 0xFFFFF,
		GDT_ACCESS_USER_BASE | GDT_ACCESS_DC | GDT_ACCESS_EXEC,
		GDT_FLAGS_DEFAULT);
	// userspace data
	gdt_init_desc(GDT_INDEX_UDATA,
		0x0, 0xFFFFF,
		GDT_ACCESS_USER_BASE,
		GDT_FLAGS_DEFAULT);
	// userspace stack
	gdt_init_desc(GDT_INDEX_USTACK,
		0x0, 0x0,
		GDT_ACCESS_USER_BASE | GDT_ACCESS_DC,
		GDT_FLAGS_DEFAULT);

	// TSS
	gdt_init_desc(GDT_INDEX_DEF_TSS,
		(uint32_t) & default_tss, sizeof(default_tss) - 1,
		GDT_ACCESS_PRESENT | GDT_ACCESS_EXEC | GDT_ACCESS_PRIV_3,
		0x00);

	// initialize the gdtr structure
	kgdtr.limit = GDT_SIZE * sizeof(struct gdtdesc);
	kgdtr.base = GDT_BASE;

	// copy the gdtr to its memory area
	memcpy((char *) kgdtr.base, (char *) kgdt, kgdtr.limit);

	// load the gdtr registry
	asm("lgdtl (kgdtr)");

	/*
	 * Stick offset of KDATA segment selector (0x10) in DS/ES/FS/GS, and
	 * then long jump using KCODE segment selector offset (0x08)
	 */
	asm("   movw $0x10, %ax	\n \
            movw %ax, %ds	\n \
            movw %ax, %es	\n \
            movw %ax, %fs	\n \
            movw %ax, %gs	\n \
            ljmp $0x08, $next	\n \
            next:		\n");

}
