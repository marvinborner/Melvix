// MIT License, Copyright (c) 2020 Marvin Borner

int pow(int base, int exp)
{
	if (exp < 0)
		return 0;

	if (!exp)
		return 1;

	int ret = base;
	for (int i = 1; i < exp; i++)
		ret *= base;
	return ret;
}
