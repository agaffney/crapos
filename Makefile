.PHONY: default

default: all

%:
	$(MAKE) -C kernel $@
