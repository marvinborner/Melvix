// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <keymap.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

static void map(struct keymap *keymap, int line, char ch, int index)
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

// Very ugly code but it should work for now
struct keymap *keymap_parse(const char *path)
{
	char *keymap_src = sread(path);
	if (!keymap_src)
		return NULL;
	struct keymap *keymap = malloc(sizeof(*keymap));

	int index = 0;
	int ch_index = 0;
	char ch;
	int escaped = 0;
	int line = 0;
	int skip = 0;
	while ((ch = keymap_src[index]) != '\0' || escaped) {
		if (ch == ' ' && !skip) {
			skip = 1;
			index++;
			continue;
		} else if (ch == '\n') {
			ch_index = 0;
			index++;
			line++;
			continue;
		} else if (ch == '\\' && !escaped) {
			escaped = 1;
			index++;
			continue;
		}
		skip = 0;

		if (ch == ' ' && !escaped)
			ch = 0;

		ch_index++;
		if (escaped) {
			switch (ch) {
			case 'b':
				ch = '\b';
				break;
			case 't':
				ch = '\t';
				break;
			case 'n':
				ch = '\n';
				break;
			case '\\':
				ch = '\\';
				break;
			case ' ':
				ch = ' ';
				break;
			default:
				print("Unknown escape!\n");
			}
			escaped = 0;
		}

		map(keymap, line, ch, ch_index);
		index++;
	}

	return keymap;
}
