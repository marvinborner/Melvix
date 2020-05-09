#ifndef MELVIX_COMMON_H
#define MELVIX_COMMON_H

// Syscalls
#define SYS_HALT 0 // Halt (debug)
#define SYS_EXIT 1 // Exit process
#define SYS_READ 2 // Read file
#define SYS_WRITE 3 // Write file
#define SYS_EXEC 4 // Execute file and kill parent
#define SYS_SPAWN 5 // Execute file and let parent alive
#define SYS_WAIT 6 // Wait for PID
#define SYS_GET_PID 7 // Get process id
#define SYS_MALLOC 8 // Allocate memory
#define SYS_FREE 9 // Free memory
#define SYS_GET 10 // Get kernel variable
#define SYS_MAP 11 // Map input to function

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