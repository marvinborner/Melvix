int main()
{
	char *vga = (char *)0x000B8000;
	for (long i = 0; i < 80 * 25; i++) {
		*vga++ = 0;
		*vga++ = 0;
	}
	while (1) {
	};
	return 0;
}
