# Shared build rules for the test suites. Include from tests/<suite>/Makefile
# after setting:
#
#   TEST_SRCS   test sources and stubs in the suite directory
#   UNIT_SRCS   (optional) kernel/other sources under test, built like libc
#
# Every suite builds the libc sources (with renamed symbols, see rename.h) into
# its own archive, so suites don't depend on each other's build output.
#
#   make                     build and run the suite
#   make SAN=0               without AddressSanitizer/UBSan
#   make ARGS='--list'       pass arguments to the test runner (see --help);
#   make ARGS='itoa'         e.g. run only tests with "itoa" in their name

CC := cc
SAN ?= 1

ROOT := ../..
COMMON := ../common
BUILD := build
LIBC_DIR := $(ROOT)/libc
KERNEL_INC := $(ROOT)/kernel/include

# GCC's freestanding headers (stdarg.h, stddef.h, stdint.h, ...)
FREESTANDING_INC := $(shell $(CC) -print-file-name=include)

ifeq ($(SAN),1)
# -fno-sanitize-recover makes undefined behavior abort the test instead of
# only printing a message while the test carries on and passes.
SAN_FLAGS := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
SAN_DEFINE := -DTEST_SANITIZERS=1
else
SAN_FLAGS :=
SAN_DEFINE := -DTEST_SANITIZERS=0
endif

# Code under test and the tests themselves: same style of environment as the
# kernel build, plus the symbol renaming. Native word size on purpose.
LIB_CFLAGS := -std=gnu11 -g -O0 -Wall $(SAN_FLAGS) \
	-ffreestanding -nostdinc -isystem $(FREESTANDING_INC) \
	-I $(LIBC_DIR)/include -I $(KERNEL_INC) -I $(COMMON) \
	-include $(COMMON)/rename.h -D__is_libk $(SAN_DEFINE)

# Harness: ordinary hosted program, no renaming.
HOST_CFLAGS := -std=gnu11 -g -O0 -Wall $(SAN_FLAGS) -I $(COMMON)

COMMON_DEPS := $(COMMON)/rename.h $(COMMON)/harness.h

LIBC_SRCS := $(shell find $(LIBC_DIR) -name '*.c')
LIBC_OBJS := $(patsubst $(LIBC_DIR)/%.c,$(BUILD)/libc/%.o,$(LIBC_SRCS))
LIBC_ARCHIVE := $(BUILD)/libc-under-test.a

TEST_OBJS := $(patsubst %.c,$(BUILD)/%.o,$(TEST_SRCS))
UNIT_OBJS := $(patsubst $(ROOT)/%.c,$(BUILD)/unit/%.o,$(UNIT_SRCS))

RUNNER := $(BUILD)/run-tests

.PHONY: all test clean
ARGS ?=

all test: $(RUNNER)
	./$(RUNNER) $(ARGS)

# Test and unit objects are linked directly (tests self-register through
# constructors, which an archive would drop); libc comes from the archive.
$(RUNNER): $(TEST_OBJS) $(UNIT_OBJS) $(BUILD)/harness.o $(LIBC_ARCHIVE)
	$(CC) $(SAN_FLAGS) -o $@ $^

$(LIBC_ARCHIVE): $(LIBC_OBJS)
	rm -f $@
	ar rcs $@ $^

$(BUILD)/libc/%.o: $(LIBC_DIR)/%.c $(COMMON_DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(BUILD)/unit/%.o: $(ROOT)/%.c $(COMMON_DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c $(COMMON_DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c -o $@ $<

$(BUILD)/harness.o: $(COMMON)/harness.c $(COMMON)/harness.h
	@mkdir -p $(dir $@)
	$(CC) $(HOST_CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD)
