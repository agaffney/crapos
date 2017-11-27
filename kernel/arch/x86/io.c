#include <arch/x86/io.h>

char inb(unsigned short port) {
	char data;
	asm volatile ("inb %%dx, %%al" : "=a" (data) : "d" (port));
	return data;
}

void outb(unsigned short port, unsigned char data) {
	asm volatile ("outb %%al, %%dx" :: "d" (port), "a" (data));
}
