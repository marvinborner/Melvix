// MIT License, Copyright (c) 2020 Marvin Borner
// HTML parsing is mainly based on the XML parser

#ifndef HTML_H
#define HTML_H

#include <def.h>
#include <list.h>

struct dom {
	char *tag;
	char *content;
	struct dom *parent;
	struct list *children;
};

struct html_element {
	u32 x_offset;
	u32 y_offset;
	struct dom *dom;
	struct element *obj;
};

int html_render(struct element *container, char *data, u32 length);

#endif
