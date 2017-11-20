ARCH_CFLAGS=-m32
ARCH_LDFLAGS=-m elf_i386 --oformat elf32-i386 -T $(ARCHDIR)/link.ld
ARCH_NASM_FLAGS=-f elf32
