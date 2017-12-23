#pragma once

#include <stddef.h>

/*
 * The following typedef and #define are used to allow filesystem drivers to
 * "register" themselves, so that we don't need to hard-code a list anywhere.
 * This works by defining a function pointer that will end up in a special
 * section in the resulting .o file. The linker script then gathers the
 * pointers in the special section in each .o file and puts them together in
 * the resulting binary, with a start/end symbol to denote the boundaries. The
 * block of function pointers is then iterated over as an array, and each
 * registered function called.
 */
typedef void (*vfs_register_func)();

#define VFS_REGISTER_FUNC(__func) static vfs_register_func  __attribute__((section(".register_func.vfs." #__func))) _register_func_vfs_##__func = __func

// Used to access start/end symbols defined using above macro
extern vfs_register_func _register_func_vfs_start[], _register_func_vfs_end[];

#define VFS_MAX_FILESYSTEMS 256

typedef struct {
	char name[16];
} vfs_filesystem;

void vfs_add_filesystem(vfs_filesystem * fs);

struct _vfs_dir;
struct _vfs_file;
struct _vfs_inode;

struct _vfs_dir {
	struct _vfs_dir * parent;
};

typedef struct _vfs_dir vfs_dir;

struct _vfs_file {

};

typedef struct _vfs_file vfs_file;

struct _vfs_inode {

};

typedef struct _vfs_file vfs_file;
