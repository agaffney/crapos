define generate_obj_list
	$(patsubst %, %.o, $(shell find $(or $(1),.) -type f -a \( -name '*.c' -o -name '*.asm' -o -name '*.s' -o -name '*.S' \)))
endef

CC=gcc
LD=ld
NASM=nasm
OBJCOPY=objcopy
STRIP=strip

ARCH=x86
