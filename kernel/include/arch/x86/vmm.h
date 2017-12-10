#pragma once

#include <stdint.h>

#define ARCH_PAGE_SIZE 4096

// This should match the value with the same name defined in boot.s
#define KERN_OFFSET 0xC0000000
// This should match the size of the .bootstrap_heap section in boot.s
#define INITIAL_HEAP_SIZE 16384

#define PAGE_DIR_ENTRIES 1024
#define PAGE_DIR_ENTRY_SIZE 4
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_TABLE_ENTRY_SIZE 4

#define PAGE_FLAG_PRESENT       1
#define PAGE_FLAG_WRITABLE      2
#define PAGE_FLAG_USER          4
#define PAGE_FLAG_WRITE_THROUGH 8
#define PAGE_FLAG_CACHE_DISABLE 16
#define PAGE_FLAG_SIZE_4MB      128

#define PAGE_FLAG_ACCESSED      32
#define PAGE_FLAG_DIRTY         64

// External symbols defined in boot.s, used only to get their address in memory
// They are defined as 'char' to make pointer arithmetic work sanely for memory
// offsets
extern char _boot_pagedir;
extern char _boot_pagetab1;
extern char _bootstrap_heap_start;
extern char _bootstrap_heap_end;

typedef struct {
	void * pages[1024];
} x86_page_table;

typedef struct {
	x86_page_table * page_tables[1024];
} x86_page_dir;

void vmm_init();
void reload_page_dir();
void * map_page();
void * get_next_phys_page();
uint16_t get_page_dir_index(void * addr);
uint16_t get_page_table_index(void * addr);
