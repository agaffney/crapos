#include <arch/x86/idt.h>
#include <arch/x86/int.h>
#include <arch/x86/io.h>
#include <core/kprint.h>

// ktest_int_hook only exists in KTEST=1 builds; weak, so it is null otherwise
#pragma weak ktest_int_hook

/* Log every field of an interrupt frame on one debug line */
void arch_x86_int_debug_frame(const arch_x86_int_frame * f) {
	kdebug("frame info: gs=0x%x, fs=0x%x, es=0x%x, ds=0x%x, edi=0x%x, esi=0x%x, ebp=0x%x, ebx=0x%x, edx=0x%x, ecx=0x%x, eax=0x%x, eip=0x%x, cs=0x%x, eflags=0x%x\n", f->gs, f->fs, f->es, f->ds, f->edi, f->esi, f->ebp, f->ebx, f->edx, f->ecx, f->eax, f->eip, f->cs, f->eflags);
}

/* Handler for hardware IRQs that have no handler of their own. It logs them. */
void arch_x86_irq_handler_default(const arch_x86_int_frame * f) {
	if (ktest_int_hook) {
		ktest_int_hook(f->vector, f);
	}
	kdebug("received interrupt %d, with error %d\n", f->vector, f->error_code);
	arch_x86_int_debug_frame(f);
	if(f->vector >= PIC2_OFFSET_ADDR) {
		kdebug("IRQ %d\n", f->vector - PIC2_OFFSET_ADDR + 8);
	} else if(f->vector >= PIC1_OFFSET_ADDR) {
		kdebug("IRQ %d\n", f->vector - PIC1_OFFSET_ADDR);
	}
}

// Names of the CPU exceptions, indexed by vector
static const char * const exception_names[32] = {
	"divide error", "debug", "non-maskable interrupt", "breakpoint",
	"overflow", "bound range exceeded", "invalid opcode", "device not available",
	"double fault", "coprocessor segment overrun", "invalid TSS", "segment not present",
	"stack-segment fault", "general protection fault", "page fault", "reserved",
	"x87 floating-point error", "alignment check", "machine check", "SIMD floating-point error",
	"virtualization exception", "control protection exception", "reserved", "reserved",
	"reserved", "reserved", "reserved", "reserved",
	"hypervisor injection exception", "VMM communication exception", "security exception", "reserved",
};

/*
 * Exceptions that are traps, or are used on purpose, and are fine to return
 * from: debug (1), NMI (2), breakpoint (3) and overflow (4). Every other CPU
 * exception is a fault or an abort. Returning from a fault runs the same
 * failing instruction again (the divide error at vector 0 included), and an
 * abort cannot be resumed at all. Vectors above 31 are not CPU exceptions and
 * are never fatal.
 */
int arch_x86_cpu_exception_is_fatal(uint32_t vector) {
	if (vector >= 32) {
		return 0;
	}
	return !(vector >= 1 && vector <= 4);
}

/* The name of a CPU exception vector, for reports. Only vectors 0-31 have one. */
const char * arch_x86_cpu_exception_name(uint32_t vector) {
	if (vector >= 32) {
		return "not a CPU exception";
	}
	return exception_names[vector];
}

/*
 * Report a fatal CPU exception and stop this CPU for good. Does not return.
 */
static void __attribute__((noreturn)) fatal_exception(const arch_x86_int_frame * f) {
	kprint("\n*** fatal CPU exception %d (%s) ***\n", f->vector, arch_x86_cpu_exception_name(f->vector));
	kprint("error code=0x%x eip=0x%x cs=0x%x eflags=0x%x\n", f->error_code, f->eip, f->cs, f->eflags);
	if (f->vector == 14) {
		// Page fault: the address that was accessed is in cr2
		uint32_t cr2;
		asm volatile ("mov %%cr2, %0" : "=r" (cr2));
		kprint("faulting address (cr2)=0x%x\n", cr2);
	}
	kprint("eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x esi=0x%x edi=0x%x ebp=0x%x\n", f->eax, f->ebx, f->ecx, f->edx, f->esi, f->edi, f->ebp);
	kprint("ds=0x%x es=0x%x fs=0x%x gs=0x%x\n", f->ds, f->es, f->fs, f->gs);
	kprint("System halted\n");
	// hlt wakes up for an NMI, and returning would run the fault again
	for (;;) {
		asm volatile ("cli; hlt");
	}
}

/*
 * Handler for CPU exceptions (vectors 0-31). It logs the exception and returns
 * from the ones that are safe to return from. A fatal exception is reported and
 * halts the machine, unless the in-kernel tests asked for it to be returned
 * from.
 */
void arch_x86_cpu_exception_handler_default(const arch_x86_int_frame * f) {
	int expected_by_test = 0;
	if (ktest_int_hook) {
		expected_by_test = ktest_int_hook(f->vector, f);
	}
	if (arch_x86_cpu_exception_is_fatal(f->vector) && !expected_by_test) {
		fatal_exception(f);
	}
	kdebug("received CPU exception %d, with error %d\n", f->vector, f->error_code);
	arch_x86_int_debug_frame(f);
}
