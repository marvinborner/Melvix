// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CORE_PROTECTION_H
#define CORE_PROTECTION_H

void clac(void);
void stac(void);

// Quite an ugly hack but it works
#define EXPOSE(func, ...)                                                                          \
	__extension__({                                                                            \
		stac();                                                                            \
		__auto_type __ret = func(__VA_ARGS__);                                             \
		clac();                                                                            \
		__ret;                                                                             \
	})

#endif
