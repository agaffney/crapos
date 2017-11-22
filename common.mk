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
QEMU_SYSTEM ?= qemu-system-i386

ARCH ?= x86
CFLAGS ?= -ggdb -O0 -Wall

# Relative path to this file from our starting Makefile
BASEDIR := $(patsubst %/, %, $(shell dirname $(lastword $(MAKEFILE_LIST))))

KERNEL_ARCHDIR := $(BASEDIR)/kernel/arch/$(ARCH)

KERNEL_INCLUDE_DIR := $(BASEDIR)/kernel/include
LIBC_INCLUDE_DIR := $(BASEDIR)/libc/include

include $(KERNEL_ARCHDIR)/config.mk

KERNEL_CFLAGS := $(CFLAGS) -fno-stack-protector -ffreestanding -I $(KERNEL_INCLUDE_DIR) -I $(LIBC_INCLUDE_DIR) $(KERNEL_ARCH_CFLAGS)
# This is a hack to allow the use of the default gcc in the short term
KERNEL_CFLAGS += -D_FEATURES_H
LIBC_CFLAGS := $(CFLAGS) -I $(LIBC_INCLUDE_DIR)
