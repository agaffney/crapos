#pragma once

#include <stdint.h>

// This should match the value with the same name defined in boot.s
#define KERN_OFFSET 0xC0000000

#define PAGE_FLAG_PRESENT 1
#define PAGE_FLAG_WRITE   2

extern void * boot_pagedir;
extern void * boot_pagetab1;

typedef struct {
	void * pages[1024];
} x86_page_table;

typedef struct {
	x86_page_table * page_tables[1024];
} x86_page_dir;

void vmm_init();
void reload_page_dir();
