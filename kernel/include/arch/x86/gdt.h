#pragma once

#include <stdint.h>

#define GDT_SIZE 32
#define GDT_BASE 0x00000800

enum gdt_indexes {
	GDT_INDEX_NULL = 0,
	GDT_INDEX_KCODE,
	GDT_INDEX_KDATA,
	GDT_INDEX_KSTACK,
	GDT_INDEX_UCODE,
	GDT_INDEX_UDATA,
	GDT_INDEX_USTACK,
	GDT_INDEX_DEF_TSS,
};

#define GDT_ACCESS_ACCESSED   (1 << 0)
#define GDT_ACCESS_RW         (1 << 1)
// Segment grows down (stack), or "conforming" for code privilege level
#define GDT_ACCESS_DC         (1 << 2)
#define GDT_ACCESS_EXEC       (1 << 3)
#define GDT_ACCESS_TYPE_SEG   (1 << 4)
#define GDT_ACCESS_PRIV_0     (0 << 5)
#define GDT_ACCESS_PRIV_3     (3 << 5)
#define GDT_ACCESS_PRESENT    (1 << 7)

#define GDT_ACCESS_KERN_BASE  GDT_ACCESS_TYPE_SEG | GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_PRIV_0
#define GDT_ACCESS_USER_BASE  GDT_ACCESS_TYPE_SEG | GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_PRIV_3

#define GDT_FLAG_SIZE_16      (0 << 2)
#define GDT_FLAG_SIZE_32      (1 << 2)
#define GDT_FLAG_GRAN_BYTE    (0 << 3)
#define GDT_FLAG_GRAN_PAGE    (1 << 3)

#define GDT_FLAGS_DEFAULT     GDT_FLAG_SIZE_32 | GDT_FLAG_GRAN_PAGE

// GDT segment descriptor
struct gdtdesc {
	uint16_t limit0_15;
	uint16_t base0_15;
	uint8_t base16_23;
	uint8_t access;
	uint8_t limit16_19:4;
	uint8_t flags:4;
	uint8_t base24_31;
} __attribute__ ((packed));

// GDT registry "pointer"
struct gdtr {
	uint16_t limit;
	uint32_t base;
} __attribute__ ((packed));

struct tss {
	uint16_t previous_task, __previous_task_unused;
	uint32_t esp0;
	uint16_t ss0, __ss0_unused;
	uint32_t esp1;
	uint16_t ss1, __ss1_unused;
	uint32_t esp2;
	uint16_t ss2, __ss2_unused;
	uint32_t cr3;
	uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint16_t es, __es_unused;
	uint16_t cs, __cs_unused;
	uint16_t ss, __ss_unused;
	uint16_t ds, __ds_unused;
	uint16_t fs, __fs_unused;
	uint16_t gs, __gs_unused;
	uint16_t ldt_selector, __ldt_sel_unused;
	uint16_t debug_flag, io_map;
} __attribute__ ((packed));

void gdt_init_desc(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

void gdt_init();
