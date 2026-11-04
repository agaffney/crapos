# Host-side unit tests. Each suite is independent and has its own Makefile:
#
#   make              run all suites
#   make libc         run only the libc suite
#   make kernel       run only the kernel suite
#   make clean        clean all suites (or: make -C libc clean)

SUITES := libc kernel

.PHONY: all test $(SUITES) clean

# Run every suite even if an earlier one fails, then report overall status.
all test:
	@rc=0; for s in $(SUITES); do $(MAKE) -C $$s || rc=1; done; exit $$rc

$(SUITES):
	$(MAKE) -C $@

clean:
	@for s in $(SUITES); do $(MAKE) -C $$s clean; done
