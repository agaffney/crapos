include ../../common.mk

$(OUTPUT): $(OBJECTS)
	$(LD) $(LDFLAGS) -r -o $(OUTPUT) $^

init.o: init.asm
	$(NASM) $(NASM_FLAGS) $< -o $@
