// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef KEYMAP_H
#define KEYMAP_H

#define KEYMAP_LENGTH 90

struct keymap {
	char map[KEYMAP_LENGTH];
	char shift_map[KEYMAP_LENGTH];
	char alt_map[KEYMAP_LENGTH];
};

struct keymap *keymap_parse(const char *path);

#endif
