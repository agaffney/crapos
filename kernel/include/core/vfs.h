#pragma once

#include <stddef.h>
#include <stdint.h>

// Size of array holding references to filesystem drivers
#define VFS_MAX_FILESYSTEMS 256

#define VFS_DIR_SEPARATOR '/'

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

#define VFS_REGISTER_FUNC(__func) static vfs_register_func __attribute__((section(".register_func.vfs." #__func))) __attribute((unused)) _register_func_vfs_##__func = __func

// Used to access start/end symbols defined using above macro
extern vfs_register_func _register_func_vfs_start[], _register_func_vfs_end[];

struct _vfs_mount_args;
struct _vfs_mount;

struct _vfs_filesystem {
	char name[16];
	struct _vfs_mount * (*mount_func)(struct _vfs_mount_args *);
	void (*readdir_func)();
};
typedef struct _vfs_filesystem vfs_filesystem;

void vfs_init();
void vfs_add_filesystem(vfs_filesystem * fs);

enum vfs_file_types {
	VFS_FILE_TYPE_REGULAR = 1,
	VFS_FILE_TYPE_DIRETORY,
	VFS_FILE_TYPE_DEVICE,
	VFS_FILE_TYPE_SYMLINK,
	VFS_FILE_TYPE_NAMED_PIPE,
	VFS_FILE_TYPE_SOCKET,
};

// Directory entry
struct _vfs_dirent {
	uint32_t inode_no;
	char     name[];
};

typedef struct _vfs_dirent vfs_dirent;

// Inode
struct _vfs_file {
	uint8_t  type;
	uint32_t uid;
	uint32_t gid;
	uint16_t mode;
};

typedef struct _vfs_file vfs_file;

// Mounted filesystem instance
struct _vfs_mount {
	vfs_filesystem * fs;
	void * mount_data;
};

typedef struct _vfs_mount vfs_mount;

// Used to pass mount args to filesystem driver
struct _vfs_mount_args {
	char device[256];
	char mountpoint[1024];
	char options[4096];
};

typedef struct _vfs_mount_args vfs_mount_args;
