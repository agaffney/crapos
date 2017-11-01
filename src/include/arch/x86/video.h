/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define VIDEO_MEM_BASE_ADDR 0xb8000

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

void x86_video_init();

void clear_screen(void);

char video_create_attribute_byte(enum video_colors, enum video_colors);

void putch(char c);
