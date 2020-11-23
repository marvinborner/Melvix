// MIT License, Copyright (c) 2020 Marvin Borner
// HTML parsing is mainly based on the XML parser

#include <print.h>
#include <str.h>

int html_self_closing(const char *tag)
{
	// TODO: Add 'meta'?
	const char *void_elements[] = { "area",	 "base", "br",	  "col",    "embed", "hr", "img",
					"input", "link", "param", "source", "track", "wbr" };

	for (u32 i = 0; i < sizeof(void_elements) / sizeof(void_elements[0]); ++i) {
		if (!strcmp(void_elements[i], tag))
			return 1;
	}
	return 0;
}
