include ../common.mk

.PHONY: all

all: $(OUTPUT)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTPUT): init.o main.o
	$(LD) $(LDFLAGS) -o $(OUTPUT) $^

init.o: init.asm
	$(NASM) $(NASM_FLAGS) $< -o $@

clean:
	-rm *.o $(OUTPUT)

