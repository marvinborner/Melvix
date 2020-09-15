// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <keymap.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

void map(struct keymap *keymap, int line, char ch, int index)
{
	switch (line) {
	case 0:
		keymap->map[index] = ch;
		break;
	case 1:
		keymap->shift_map[index] = ch;
		break;
	case 2:
		keymap->alt_map[index] = ch;
		break;
	default:
		break;
	}
}

struct keymap *keymap_parse(const char *path)
{
	char *keymap_src = read(path);
	if (!keymap_src)
		return NULL;
	printf("%c\n", keymap_src[0]);
	struct keymap *keymap = malloc(sizeof(*keymap));

	int index = 0;
	char ch;
	int is_start = 0;
	int line = 0;
	while ((ch = keymap_src[index]) != '\0') {
		if (ch == '"') {
			if (keymap_src[index + 1] == '"')
				map(keymap, line, '\0', index);
			is_start ^= 1;
			index++;
			continue;
		} else if ((ch == ' ' || ch == ',') && !is_start) {
			index += 2;
			continue;
		}
		printf("\"%c\"\n", ch);

		if (ch == '\\') {
			map(keymap, line, ch, index + 1);
		} else if (ch == '\n') {
			line++;
		}
		index++;
	}

	loop();
	return NULL;
}
