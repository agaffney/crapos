#include <arch/x86/gdt.h>
#include <libk/string.h>
#include <core/kprint.h>

struct gdtdesc kgdt[GDT_SIZE];
struct tss default_tss;
struct gdtr kgdtr;

void gdt_init_desc(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags, struct gdtdesc *desc) {
	desc->limit0_15 = (limit & 0xffff);
	desc->base0_15 = (base & 0xffff);
	desc->base16_23 = (base & 0xff0000) >> 16;
	desc->access = access;
	desc->limit16_19 = (limit & 0xf0000) >> 16;
	desc->flags = (flags & 0xf);
	desc->base24_31 = (base & 0xff000000) >> 24;
}

void gdt_init() {
	default_tss.debug_flag = 0x00;
	default_tss.io_map = 0x00;
	default_tss.esp0 = 0x1FFF0;
	default_tss.ss0 = 0x18;

	// null descriptor
	gdt_init_desc(0x0, 0x0, 0x0, 0x0, &kgdt[0]);

	// kernel code
	gdt_init_desc(0x0, 0xFFFFF,
		GDT_ACCESS_KERN_BASE | GDT_ACCESS_EXEC,
		GDT_FLAGS_DEFAULT, &kgdt[1]);
	// kernel data
	gdt_init_desc(0x0, 0xFFFFF,
		GDT_ACCESS_KERN_BASE,
		GDT_FLAGS_DEFAULT, &kgdt[2]);
	// kernel stack
	gdt_init_desc(0x0, 0x0,
		GDT_ACCESS_KERN_BASE | GDT_ACCESS_DC,
		GDT_FLAGS_DEFAULT, &kgdt[3]);

	// userspace code
	gdt_init_desc(0x0, 0xFFFFF,
		GDT_ACCESS_USER_BASE | GDT_ACCESS_DC | GDT_ACCESS_EXEC,
		GDT_FLAGS_DEFAULT, &kgdt[4]);
	// userspace data
	gdt_init_desc(0x0, 0xFFFFF,
		GDT_ACCESS_USER_BASE,
		GDT_FLAGS_DEFAULT, &kgdt[5]);
	// userspace stack
	gdt_init_desc(0x0, 0x0,
		GDT_ACCESS_USER_BASE | GDT_ACCESS_DC,
		GDT_FLAGS_DEFAULT, &kgdt[6]);

	// TSS
	gdt_init_desc((uint32_t) & default_tss, sizeof(default_tss) - 1,
		GDT_ACCESS_PRESENT | GDT_ACCESS_EXEC | GDT_ACCESS_PRIV_3,
		0x00, &kgdt[7]);

	// initialize the gdtr structure
	kgdtr.limit = GDT_SIZE * sizeof(struct gdtdesc) - 1;
	kgdtr.base = GDT_BASE;

	// copy the gdtr to its memory area
	memcpy((char *) kgdtr.base, (char *) kgdt, kgdtr.limit);

	// load the gdtr registry
	asm("lgdtl (kgdtr)");

	/*
	 * Stick index of kernel data segment selector (0x10) in DS/ES/FS/GS, and
	 * then long jump using kernel code segment selector index (0x08)
	 */
	asm("   movw $0x10, %ax	\n \
            movw %ax, %ds	\n \
            movw %ax, %es	\n \
            movw %ax, %fs	\n \
            movw %ax, %gs	\n \
            ljmp $0x08, $next	\n \
            next:		\n");

}
