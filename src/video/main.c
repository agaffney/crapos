/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

enum video_colors {
	VIDEO_COLOR_BLACK,
	VIDEO_COLOR_BLUE,
	VIDEO_COLOR_GREEN,
	VIDEO_COLOR_CYAN,
	VIDEO_COLOR_RED,
	VIDEO_COLOR_MAGENTA,
	VIDEO_COLOR_BROWN,
	VIDEO_COLOR_LIGHT_GREY,
	VIDEO_COLOR_DARK_GREY,
	VIDEO_COLOR_LIGHT_BLUE,
	VIDEO_COLOR_LIGHT_GREEN,
	VIDEO_COLOR_LIGHT_CYAN,
	VIDEO_COLOR_LIGHT_RED,
	VIDEO_COLOR_LIGHT_MAGENTA,
	VIDEO_COLOR_LIGHT_BROWN,
	VIDEO_COLOR_WHITE
};

char video_create_attribute_byte(enum video_colors, enum video_colors);

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		putch(str[i++]);
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_BLACK);
	}
	current_loc = 0;
}

char video_create_attribute_byte(enum video_colors bg, enum video_colors fg) {
	return (bg << 4) + fg;
}

void putch(char c) {
	vidptr[current_loc++] = c;
	vidptr[current_loc++] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_WHITE);
}
