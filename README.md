# crapos

The Crappy Redundant Asynchronous Processing Operating System (CrapOS, for short) is
a crappy kernel/userland written in C, mostly for my amusement. It's originally based
off of:

https://github.com/arjun024/mkeykernel

but is also heavily influenced by the docs at:

http://wiki.osdev.org/Tutorials
http://www.flingos.co.uk/docs/#conceptual-docs

## Features

* It boots!
* basic keyboard handling via BIOS
* basic VGA text-mode output
* combined libc/libk
* mostly sane and maintanable build system
* serial output
* rudimentary "file" support
* rudimentary VFS subsystem
* basic memory/page allocator
