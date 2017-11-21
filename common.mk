# Generate list of object files in directory tree by finding all source files
# and appending a suffix to them. This function takes two optional arguments:
#
# 1 - path to search in, defaults to current directory
# 2 - suffix to add, defaults to '.o'
define generate_obj_list
	$(patsubst %, %$(or $(2),.o), $(shell find $(or $(1),.) -type f -a \( -name '*.c' -o -name '*.asm' -o -name '*.s' -o -name '*.S' \)))
endef

CC ?= gcc
LD ?= ld
NASM ?= nasm
OBJCOPY ?= objcopy
STRIP ?= strip

ARCH ?= x86
CFLAGS ?= -ggdb -O0 -Wall

# Relative path to this file from our starting Makefile
BASEDIR := $(patsubst %/, %, $(shell dirname $(lastword $(MAKEFILE_LIST))))

KERNEL_ARCHDIR := $(BASEDIR)/kernel/arch/$(ARCH)

KERNEL_INCLUDE_DIR := $(BASEDIR)/kernel/include
LIBC_INCLUDE_DIR := $(BASEDIR)/libc/include

include $(KERNEL_ARCHDIR)/config.mk

KERNEL_CFLAGS := $(CFLAGS) -fno-stack-protector -nostdinc -fno-builtin -I $(KERNEL_INCLUDE_DIR) -I $(LIBC_INCLUDE_DIR) $(KERNEL_ARCH_CFLAGS)
LIBC_CFLAGS := $(CFLAGS) -I $(LIBC_INCLUDE_DIR)