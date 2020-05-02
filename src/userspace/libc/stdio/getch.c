#include <stdint.h>
#include <syscall.h>

// TODO: Move keymaps somewhere more appropriate
char keymap[128] = {
	0 /*E*/,   27,	     '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',	  '=',
	'\b',	   '\t',     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',	  ']',
	'\n',	   17 /*C*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',	  '`',
	14 /*LS*/, '\\',     'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 14 /*RS*/, '*',
	0, // Alt key
	' ', // Space bar
	15, // Caps lock
	0,	   0,	     0,	  0,   0,   0,	 0,   0,   0,	0, // F keys
	0, // Num lock
	0, // Scroll lock
	0, // Home key
	0, // Up arrow
	0, // Page up
	'-',
	0, // Left arrow
	0,
	0, // Right arrow
	'+',
	0, // End key
	0, // Down arrow
	0, // Page down
	0, // Insert key
	0, // Delete key
	0,	   0,	     0,
	0, // F11
	0, // F12
	0, // Other keys
};

char shift_keymap[128] = {
	0 /*E*/,   27,	     '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',	  '+',
	'\b',	   '\t',     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{',	  '}',
	'\n',	   17 /*C*/, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',	  '~',
	14 /*LS*/, '|',	     'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 14 /*RS*/, '*',
	0, // Alt key
	' ', // Space bar
	15, // Caps lock
	0,	   0,	     0,	  0,   0,   0,	 0,   0,   0,	0, // F keys
	0, // Num lock
	0, // Scroll lock
	0, // Home key
	0, // Up arrow
	0, // Page up
	'-',
	0, // Left arrow
	0,
	0, // Right arrow
	'+',
	0, // End key
	0, // Down arrow
	0, // Page down
	0, // Insert key
	0, // Delete key
	0,	   0,	     0,
	0, // F11
	0, // F12
	0, // Other keys
};

char *getch()
{
	// TODO: Add shift support
	// TODO: Implement keyboard dev driver
	u8 scancode = 42; //syscall_scancode();
	if ((scancode & 0x80) == 0) { // Press
		return keymap[scancode];
	} else { // Release
		return 0;
	}
}