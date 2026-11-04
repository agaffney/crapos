/*
 * Force-included (-include) into every libc source and every test so that the
 * libc under test gets distinct symbol names. Without this, our sprintf,
 * strlen, memcpy, etc. would collide with (and shadow) the host's libc inside
 * the test binary, including the harness's own use of them.
 *
 * Everything that is compiled with this header sees the renamed identifiers,
 * so source code and tests keep using the normal names.
 */
#pragma once

#define abort     crapos_abort
#define itoa      crapos_itoa
#define memcmp    crapos_memcmp
#define memcpy    crapos_memcpy
#define memmove   crapos_memmove
#define memset    crapos_memset
#define printf    crapos_printf
#define putchar   crapos_putchar
#define puts      crapos_puts
#define sprintf   crapos_sprintf
#define strcpy    crapos_strcpy
#define strlen    crapos_strlen
#define strncpy   crapos_strncpy
#define tolower   crapos_tolower
#define toupper   crapos_toupper
#define vsprintf  crapos_vsprintf
