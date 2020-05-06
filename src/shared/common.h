#ifndef MELVIX_COMMON_H
#define MELVIX_COMMON_H

// Syscalls
#define SYS_HALT 0 // Halt (debug)
#define SYS_EXIT 1 // Exit process
#define SYS_FORK 2 // Fork process
#define SYS_READ 3 // Read file
#define SYS_WRITE 4 // Write file
#define SYS_EXEC 5 // Execute file
#define SYS_GET_PID 6 // Get process id
#define SYS_MALLOC 7 // Allocate memory
#define SYS_FREE 8 // Free memory
#define SYS_GET 9 // Get kernel variable
#define SYS_MAP 10 // Map input to function

// Get
#define GET_FRAMEBUFFER 0

// Mappings
#define MAP_KEYBOARD 0
#define MAP_MOUSE 1

// Common event structs
struct keyboard_event {
	int scancode;
};

struct mouse_event {
	int mouse_x;
	int mouse_y;
};

#endif