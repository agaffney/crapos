bits 32

global cpuid_vendor_string

; void cpuid_vendor_string(char *)
cpuid_vendor_string:
	; Set EAX to 0 for CPU vendor string
	mov eax, 0x0
	cpuid
	; Put start address of 'char *' param into EAX
	mov eax, [esp + 4]
	; Put 4 bytes from each of EBX, EDX, and ECX into buffer
	mov [eax], ebx
	mov [eax + 4], edx
	mov [eax + 8], ecx
	; Add null byte to buffer
	mov byte [eax + 12], 0x0
	ret
