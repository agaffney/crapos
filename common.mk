# Generate list of object files in directory tree by finding all source files
# and appending a suffix to them. This function takes two optional arguments:
#
# 1 - path to search in, defaults to current directory
# 2 - suffix to add, defaults to '.o'
define generate_obj_list
	$(patsubst %, %$(or $(2),.o), $(shell find $(or $(1),.) -type f -a \( -name '*.c' -o -name '*.s' -o -name '*.S' \)))
endef

CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
STRIP ?= strip
QEMU_SYSTEM ?= qemu-system-i386

ARCH ?= x86
CFLAGS ?= -ggdb -O0 -Wall

# Compiler flags that also write foo.c.d next to each object: a Makefile
# fragment listing the headers the file included. The Makefiles read these back
# in with -include, so changing a header rebuilds everything that includes it.
# -MP adds an empty rule for each header, so deleting or renaming one doesn't
# stop the build with "no rule to make target".
DEPFLAGS := -MMD -MP

# The first line of each tool's --version output. It goes into the flags files
# below, so that upgrading the compiler or the linker rebuilds what it built even
# when its path and the flags are the same.
CC_VERSION := $(shell $(CC) --version 2>&1 | head -n 1)
LD_VERSION := $(shell $(LD) --version 2>&1 | head -n 1)

# Plain `make` builds 'all', which every Makefile including this one defines.
# Without this, the first rule a Makefile happens to define, such as the flags
# file rule from flags_stamp below, would become the default target.
.DEFAULT_GOAL := all

# Objects depend on a "flags file" whose name contains a checksum of the flags
# they are built with, so changing a flag rebuilds them:
#
#   $(eval $(call flags_stamp,<stamp variable>,<file name prefix>,<flags variable>))
#
# sets <stamp variable> to a name like <prefix>.flags-<sha1 of the flags> and adds a rule
# that creates that file, deleting flags files from earlier flags. A new set of
# flags means a file that doesn't exist yet, so it is created newer than the
# objects and they rebuild. Nothing is written when make reads the Makefile, so
# make -n and make -q are accurate, and the rule can always recreate the file
# (for example after make clean). The flags are passed to the shell in single
# quotes, with any quote inside them escaped.
define flags_stamp
$(1) := $(2).flags-$$(shell printf '%s' '$$(subst ','\'',$$($(3)))' | sha1sum | cut -d' ' -f1)
$$($(1)):
	@rm -f $(2).flags-*
	@touch $$@
endef

# Relative path to this file from our starting Makefile
BASEDIR := $(patsubst %/, %, $(shell dirname $(lastword $(MAKEFILE_LIST))))

KERNEL_ARCHDIR := $(BASEDIR)/kernel/arch/$(ARCH)

KERNEL_INCLUDE_DIR := $(BASEDIR)/kernel/include
LIBC_INCLUDE_DIR := $(BASEDIR)/libc/include

include $(KERNEL_ARCHDIR)/config.mk

KERNEL_CFLAGS := $(CFLAGS) -fno-stack-protector -ffreestanding -I $(KERNEL_INCLUDE_DIR) $(KERNEL_ARCH_CFLAGS)
# Ignore host headers, but include compiler headers
# This allows us to use the host gcc rather than a dedicated (cross-)arch toolchain
KERNEL_CFLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)
LIBC_CFLAGS := $(CFLAGS) -I $(LIBC_INCLUDE_DIR)
