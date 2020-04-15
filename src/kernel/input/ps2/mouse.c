#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/lib/stdio.h>

char mouse_cycle = 0;
char mouse_byte[3];
int mouse_x = 0;
int mouse_y = 0;
int mouse_but_1 = 0;
int mouse_but_2 = 0;
int mouse_but_3 = 0;

void mouse_handler(struct regs *a_r)
{
	switch (mouse_cycle) {
	case 0:
		mouse_byte[0] = inb(0x60);
		if (((mouse_byte[0] >> 3) & 1) == 1)
			mouse_cycle++;
		else
			mouse_cycle = 0;
		break;
	case 1:
		mouse_byte[1] = inb(0x60);
		mouse_cycle++;
		break;
	case 2:
		mouse_byte[2] = inb(0x60);
		mouse_x += mouse_byte[1];
		mouse_y -= mouse_byte[2];
		mouse_but_1 = mouse_byte[0] & 1;
		mouse_but_2 = (mouse_byte[0] >> 1) & 1;
		mouse_but_3 = (mouse_byte[0] >> 2) & 1;
		mouse_cycle = 0;

		if (mouse_x < 0)
			mouse_x = 0;
		if (mouse_y < 0)
			mouse_y = 0;
		if (mouse_x > vbe_width - 1)
			mouse_x = vbe_width - 1;
		if (mouse_y > vbe_height - 1)
			mouse_y = vbe_height - 1;
		vesa_draw_cursor(mouse_x, mouse_y);
		break;
	default:
		break;
	}
}

void mouse_wait(unsigned char a_type)
{
	unsigned int time_out = 100000;
	if (a_type == 0) {
		while (time_out--)
			if ((inb(0x64) & 1) == 1)
				return;
		return;
	} else {
		while (time_out--)
			if ((inb(0x64) & 2) == 0)
				return;
		return;
	}
}

void mouse_write(unsigned char a_write)
{
	mouse_wait(1);
	outb(0x64, 0xD4);
	mouse_wait(1);
	outb(0x60, a_write);
}

char mouse_read()
{
	mouse_wait(0);
	return inb(0x60);
}

void mouse_install()
{
	unsigned char status;

	// Enable auxiliary mouse device
	mouse_wait(1);
	outb(0x64, 0xA8);

	// Enable interrupts
	mouse_wait(1);
	outb(0x64, 0x20);
	mouse_wait(0);
	status = (unsigned char)(inb(0x60) | 3);
	mouse_wait(1);
	outb(0x64, 0x60);
	mouse_wait(1);
	outb(0x60, status);

	// Enable mousewheel
	mouse_write(0xF2);
	mouse_read();
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(100);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(80);
	mouse_read();
	mouse_write(0xF2);
	mouse_read();
	status = (unsigned char)mouse_read();
	if (status == 3)
		log("Scrollwheel support!");

	// Activate 4th and 5th mouse buttons
	mouse_write(0xF2);
	mouse_read();
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(80);
	mouse_read();
	mouse_write(0xF2);
	mouse_read();
	status = (unsigned char)mouse_read();
	if (status == 4)
		log("4th and 5th mouse button support!");

	/* TODO: Fix mouse laggyness
    mouse_write(0xE8);
    mouse_read();
    mouse_write(0x03);
    mouse_read();

    mouse_write(0xF3);
    mouse_read();
    mouse_write(200);
    mouse_read(); */

	// Enable mouse
	mouse_write(0xF4);
	mouse_read();

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);
	info("Installed mouse handler");
}