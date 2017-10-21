define base_dir
	$(dir $(lastword $(MAKEFILE_LIST)))
endef

define cwd
	$(dir $(firstword $(MAKEFILE_LIST)))
endef

define mkfile_dirname
	$(shell basename $(abspath $(dir $(firstword $(MAKEFILE_LIST)))))
endef

CC=gcc
CFLAGS=-m32 -fno-stack-protector -O0
LD=ld
LDFLAGS=-m elf_i386 -T $(call base_dir)link.ld
NASM=nasm
NASM_FLAGS=-f elf32

OUTPUT=$(call mkfile_dirname).o

