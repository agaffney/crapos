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
CFLAGS=-m32 -fno-stack-protector
LD=ld
LDFLAGS=-m elf_i386 --oformat elf32-i386
NASM=nasm
NASM_FLAGS=-f elf32

OUTPUT=$(call mkfile_dirname).o

.PHONY: default

default: all

clean:
	-rm *.o
