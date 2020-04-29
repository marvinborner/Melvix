int strncmp(char *s1, char *s2, int c)
{
	int result = 0;

	while (c) {
		result = *s1 - *s2++;

		if ((result != 0) || (*s1++ == 0)) {
			break;
		}

		c--;
	}

	return result;
}