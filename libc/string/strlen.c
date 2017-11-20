unsigned int strlen(const char *str) {
	unsigned int idx;
	for(idx = 0; idx < 32768; idx++) {
		if(str[idx] == 0) {
			return idx;
		}
	}
	return -1;
}
