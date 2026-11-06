#include <core/cmdline.h>
#include <libk/string.h>
#include <core/kprint.h>

char _cmdline[KERNEL_CMDLINE_MAX_LEN + 1];
char _cmdline_args[KERNEL_CMDLINE_MAX_ARGS][KERNEL_CMDLINE_MAX_ARG_LEN + 1];

int _cmdline_args_len;

void cmdline_init(const char * input) {
	memset(_cmdline, 0, sizeof(_cmdline));
	if (input == NULL) {
		kprint("cmdline: empty kernel commandline received\n");
		return;
	}
	strncpy(_cmdline, input, KERNEL_CMDLINE_MAX_LEN);
}

void cmdline_parse(void) {
	kprint("Command line: %s\n", _cmdline);
	// Temporary buffer for current arg
	char tmp_arg[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	int tmp_arg_len = 0;
	memset(tmp_arg, 0, KERNEL_CMDLINE_MAX_ARG_LEN + 1);
	_cmdline_args_len = 0;
	int i, arg_trunc = 0;
	for (i = 0; i < KERNEL_CMDLINE_MAX_LEN; i++) {
		if (_cmdline[i] == '\0') {
			break;
		}
		if (_cmdline[i] == KERNEL_CMDLINE_ARG_SEP) {
			if (tmp_arg_len == 0) {
				continue;
			}
			if (_cmdline_args_len >= KERNEL_CMDLINE_MAX_ARGS) {
				kprint("cmdline: dropping kernel args after %d\n", KERNEL_CMDLINE_MAX_ARGS);
				tmp_arg_len = 0;
				break;
			}
			// Clear out cmdline args entry and write our current arg
			memset(_cmdline_args[_cmdline_args_len], 0, KERNEL_CMDLINE_MAX_ARG_LEN + 1);
			strncpy(_cmdline_args[_cmdline_args_len], tmp_arg, tmp_arg_len);
			tmp_arg_len = 0;
			arg_trunc = 0;
			_cmdline_args_len++;
			continue;
		}
		// Ignore any arg characters past max arg length
		if (tmp_arg_len >= KERNEL_CMDLINE_MAX_ARG_LEN) {
			if (arg_trunc == 0) {
				kprint("cmdline: truncating kernel arg longer than %d\n", KERNEL_CMDLINE_MAX_ARG_LEN);
			}
			arg_trunc = 1;
			continue;
		}
		// Add to current arg buffer
		tmp_arg[tmp_arg_len] = _cmdline[i];
		tmp_arg_len++;
	}
	if (tmp_arg_len > 0) {
		if (_cmdline_args_len >= KERNEL_CMDLINE_MAX_ARGS) {
			kprint("cmdline: dropping kernel args after %d\n", KERNEL_CMDLINE_MAX_ARGS);
			return;
		}
		// Clear out cmdline args entry and write our current arg
		memset(_cmdline_args[_cmdline_args_len], 0, KERNEL_CMDLINE_MAX_ARG_LEN + 1);
		strncpy(_cmdline_args[_cmdline_args_len], tmp_arg, tmp_arg_len);
		_cmdline_args_len++;
	}
}

void _cmdline_split_arg(const char * arg, char * dest_name, char * dest_value) {
	int i;
	dest_name[0] = 0;
	dest_value[0] = 0;
	for (i = 0; i < KERNEL_CMDLINE_MAX_ARG_LEN; i++) {
		if (arg[i] == '\0') {
			break;
		}
		if (arg[i] == KERNEL_CMDLINE_SPLIT_CHAR) {
			// Copy remainder after split char to value
			memcpy(dest_value, (arg+i+1), strlen(arg+i+1)+1);
			break;
		}
		dest_name[i] = arg[i];
		dest_name[i+1] = 0;
	}
}

int cmdline_has_arg(const char * arg) {
	char arg_value[KERNEL_CMDLINE_MAX_ARG_LEN+1];
	if (cmdline_arg(arg, arg_value) != 0) {
		return 1;
	}
	return 0;
}

int cmdline_arg(const char * arg, char * dest_value) {
	int arg_len = strlen(arg);
	if (arg_len > KERNEL_CMDLINE_MAX_ARG_LEN) {
		return 0;
	}
	char arg_name[KERNEL_CMDLINE_MAX_ARG_LEN+1], arg_value[KERNEL_CMDLINE_MAX_ARG_LEN+1];
	int i;
	for (i = 0; i < _cmdline_args_len; i++) {
		memset(arg_name, 0, KERNEL_CMDLINE_MAX_ARG_LEN+1);
		memset(arg_value, 0, KERNEL_CMDLINE_MAX_ARG_LEN+1);
		_cmdline_split_arg(_cmdline_args[i], arg_name, arg_value);
		if (arg_len != strlen(arg_name)) {
			continue;
		}
		if (memcmp(arg, arg_name, arg_len) != 0) {
			continue;
		}
		memcpy(dest_value, arg_value, strlen(arg_value)+1);
		return 1;
	}
	return 0;
}
