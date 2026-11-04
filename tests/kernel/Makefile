# Unit tests for the hardware-independent parts of the kernel. Each kernel
# source under test is listed explicitly in UNIT_SRCS: most of kernel/ talks to
# hardware or the arch layer and can only be tested once that's stubbed.
# (Recursive "=" so $(ROOT), defined by common.mk, is expanded after the include.)
TEST_SRCS := $(wildcard test_*.c) stubs.c
UNIT_SRCS = $(ROOT)/kernel/core/cmdline.c

include ../common/common.mk
