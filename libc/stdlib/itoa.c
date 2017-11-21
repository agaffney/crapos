#include <stdlib.h>

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
	if(is_negative) {
		buf[idx++] = '-';
	}

	for(i = 0; i < idx; i++) {
		output[i] = buf[idx - i - 1];
	}
	output[i] = 0;
}
