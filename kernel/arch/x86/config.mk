# The kernel does not enable SSE or MMX, so the compiler must not emit them
# (it likes to for struct copies and memset). Doing so raises #UD.
KERNEL_ARCH_CFLAGS=-m32 -mno-sse -mno-mmx
KERNEL_ARCH_LDFLAGS=-m elf_i386 --oformat elf32-i386 -T $(KERNEL_ARCHDIR)/link.ld
