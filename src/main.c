unsigned char inb(unsigned short port)
{
	unsigned char value;
	__asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void outb(unsigned short port, unsigned char data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

int is_transmit_empty()
{
	return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (unsigned char)ch);
}

int main(int argc, char *argv[])
{
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);
	outb(0x3f8 + 0, 0x03);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xC7);
	outb(0x3f8 + 4, 0x0B);
	serial_put('a');
	while (1) {
	};
	return 0;
}
