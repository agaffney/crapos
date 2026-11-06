.section .text

.global cpuid_vendor_string
.type cpuid_vendor_string, @function
# void cpuid_vendor_string(char *)
cpuid_vendor_string:
	# CPUID overwrites EBX, so we preserve it
	push %ebx
	# Set EAX to 0 for CPU vendor string
	mov $0x0, %eax
	cpuid
	# Put start address of 'char *' param into EAX
	mov 8(%esp), %eax
	# Put 4 bytes from each of EBX, EDX, and ECX into buffer
	mov %ebx, (%eax)
	mov %edx, 4(%eax)
	mov %ecx, 8(%eax)
	# Add null byte to buffer
	movb $0x0, 12(%eax)
	pop %ebx
	ret
