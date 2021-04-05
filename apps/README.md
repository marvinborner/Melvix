# Userspace

All files in this directory run in the userspace. By default, apps will run in ring 3 and can be executed using the `exec` syscall. The default process that runs after the kernel is done initializing everything is `init.c`. This initial process may call GUI programs or other processes that can reproduce themself. The `idle.c` app however is the special idling process which runs if no other process is running (called in the process scheduler).
