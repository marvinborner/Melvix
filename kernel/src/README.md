# Kernel

The kernel is the interface between the computer hardware and the userspace. It handles everything from file and process management to resource allocation, computer interfaces like keyboards/mice and networking features. This interface is mainly established using syscalls (`features/syscall.c`).

While the main goal of Melvix is definitely not the security aspect, the kernel should be fairly secure. It implements memory protecting features like paging and address validation, SMEP/SMAP, ASLR, ring 3 processes, protected memory sharing, process based syscall validation (abstraction to default, super and kernel processes) and much more.

The directory structure consists of `drivers/`, the home of all hardware-interacting device drivers, and `features/`, the home of interfaces that are not directly tied to the hardware but rather to other features of the kernel or userspace. The `inc/` directory is the location of all kernel header files.
