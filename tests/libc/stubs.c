/*
 * Stand-ins for kernel functions that libc code (currently only the linked
 * list) calls. Built like the libc sources: -nostdinc, rename.h force-included.
 */
#include <stddef.h>
#include <stdint.h>
#include <core/vmm.h>

extern void *malloc(size_t);

/*
 * Mirror the kernel's contract: memory is only zeroed when asked to be.
 * Otherwise fill it with a pattern so code that reads uninitialized memory
 * misbehaves visibly instead of getting lucky zeroes from calloc().
 */
void *kmalloc(size_t len, uint8_t flags)
{
	unsigned char *p = malloc(len);
	size_t i;
	if (p == NULL) {
		return NULL;
	}
	for (i = 0; i < len; i++) {
		p[i] = (flags & KMALLOC_ZERO) ? 0x00 : 0xA5;
	}
	return p;
}
