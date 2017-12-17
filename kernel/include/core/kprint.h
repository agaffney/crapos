#pragma once

void kprint(const char* restrict format, ...);

#define KDEBUG_RESET_BOLD "\033[0;1m"
#define KDEBUG_RESET "\033[0m"
#define KDEBUG_BG_BLUE "\033[44m"
#define KDEBUG_BG_GREEN "\033[42m"
#define KDEBUG_BG_MAGENTA "\033[45m"

#define kdebug(fmt, ...) kprint(KDEBUG_RESET_BOLD "[DEBUG] (" KDEBUG_BG_BLUE "%s" KDEBUG_RESET_BOLD ":" KDEBUG_BG_GREEN "%d" KDEBUG_RESET_BOLD "):" KDEBUG_BG_MAGENTA "%s()" KDEBUG_RESET_BOLD ": " KDEBUG_RESET fmt,  __FILE__, __LINE__, __func__, ##__VA_ARGS__)
