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

# Flags used for every object. Changing them (SAN=0, say) must rebuild the
# objects, so each object depends on a file whose name contains a checksum of
# the flags. A new set of flags is a file that doesn't exist yet, which the rule
# below creates (newer than the objects) after deleting the old ones. Nothing is
# written while make reads the Makefile, so make -n is accurate. The flags go to
# the shell in single quotes, with any quote inside them escaped. The headers
# each object includes are tracked through the .d files that -MMD -MP write next
# to the objects.
FLAGS_CONTENT := $(CC) $(LIB_CFLAGS) | $(HOST_CFLAGS)
FLAGS_FILE := $(BUILD)/build.flags-$(shell printf '%s' '$(subst ','\'',$(FLAGS_CONTENT))' | sha1sum | cut -d' ' -f1)
DEPFLAGS := -MMD -MP

# The rule below must not become the default target
.DEFAULT_GOAL := all

$(FLAGS_FILE):
	@mkdir -p $(BUILD)
	@rm -f $(BUILD)/build.flags-*
	@touch $@

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

$(BUILD)/libc/%.o: $(LIBC_DIR)/%.c $(FLAGS_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD)/unit/%.o: $(ROOT)/%.c $(FLAGS_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c $(FLAGS_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(BUILD)/harness.o: $(COMMON)/harness.c $(FLAGS_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(HOST_CFLAGS) $(DEPFLAGS) -c -o $@ $<

-include $(LIBC_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(UNIT_OBJS:.o=.d) $(BUILD)/harness.d

clean:
	rm -rf $(BUILD)
