// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef SERIAL_H
#define SERIAL_H

#include <def.h>

void serial_install(void);
void serial_enable(void);
void serial_disable(void);
void serial_print(const char *data) NONNULL;
void serial_put(char ch);

#endif
