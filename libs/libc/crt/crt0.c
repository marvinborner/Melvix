// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <sys.h>

#ifdef USER

extern int main(int, char **);

void _start(void);
void _start(void)
{
	exit(main(0, NULL));
	while (1)
		;
}

#endif
