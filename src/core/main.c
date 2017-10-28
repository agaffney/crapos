extern void arch_init(void);

void kmain(void)
{
	arch_init();
	video_init();

	const char *str = "my first kernel with keyboard support";
	clear_screen();
	kprint(str);
	kprint_newline();
	kprint_newline();

	while(1);
}

void itoa(int input, char *output, int base) {
	char buf[50];
	int idx = 0, i;
	int remainder;
	int is_negative = 0;
	unsigned int num;

	if(input < 0 && base == 10) {
		is_negative = 1;
		num = -input;
	} else {
		num = (unsigned int)input;
	}

	while(num > 0) {
		remainder = num % base;
		buf[idx++] = remainder > 9 ? (remainder - 10 + 'a') : (remainder + '0');
		num /= base;
	}
	// Pad out a binary number to 8 bytes
	if(base == 2) {
		for(i = (idx - 2) % 8; i > 0; i--) {
			buf[idx++] = '0';
		}
	}
	// Add '0' prefix to octal number
	if(base == 8) {
		buf[idx++] = '0';
	}
	// Add '0x' prefix to hex number
	if(base == 16) {
		if(idx % 2 != 0) {
			buf[idx++] = '0';
		}
		buf[idx++] = 'x';
		buf[idx++] = '0';
	}
	if(is_negative) {
		buf[idx++] = '-';
	}

	for(i = 0; i < idx; i++) {
		output[i] = buf[idx - i - 1];
	}
	output[i] = 0;
}
