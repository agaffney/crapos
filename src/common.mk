BASEDIR=$(dir $(lastword $(MAKEFILE_LIST)))
CWD=$(dir $(firstword $(MAKEFILE_LIST)))
MKFILE_DIRNAME=$(shell basename $(abspath $(dir $(firstword $(MAKEFILE_LIST)))))

CC=gcc
CFLAGS=-ggdb -O0 -m32 -fno-stack-protector -nostdinc -Wall -fno-builtin
LD=ld
LDFLAGS=-m elf_i386 --oformat elf32-i386
NASM=nasm
NASM_FLAGS=-f elf32
OBJCOPY=objcopy
STRIP=strip

OUTPUT ?= $(MKFILE_DIRNAME).o

.PHONY: all
all: $(OUTPUT)

clean-local:
	rm -f *.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
