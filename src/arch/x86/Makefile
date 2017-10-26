include ../../common.mk

$(OUTPUT): init.o main.o
	$(LD) $(LDFLAGS) -r -o $(OUTPUT) $^

init.o: init.asm
	$(NASM) $(NASM_FLAGS) $< -o $@
