BASEDIR=$(patsubst %/, %, $(dir $(lastword $(MAKEFILE_LIST))))
CWD=$(patsubst %/, %, $(dir $(firstword $(MAKEFILE_LIST))))
MKFILE_DIRNAME=$(shell basename $(abspath $(dir $(firstword $(MAKEFILE_LIST)))))

CC=gcc
CFLAGS=-ggdb -O0 -m32 -fno-stack-protector -nostdinc -Wall -fno-builtin -I $(BASEDIR)/include
LD=ld
LDFLAGS=-m elf_i386 --oformat elf32-i386 $(LD_EXTRA_FLAGS)
NASM=nasm
NASM_FLAGS=-f elf32
OBJCOPY=objcopy
STRIP=strip

# Default target is a .o file named after the current dir
OUTPUT ?= $(MKFILE_DIRNAME).o

.PHONY: all
all: $(OUTPUT)

ifdef SUBDIRS
.PHONY: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
endif

.PHONY: clean clean-subdir-%

clean-subdir-%:
	$(MAKE) -C $(subst clean-subdir-,,$@) clean

clean: $(addprefix clean-subdir-, $(SUBDIRS))
	rm -f $(OUTPUT) *.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
