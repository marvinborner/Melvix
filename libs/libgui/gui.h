// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <errno.h>

res gui_new_window(void);
res gui_redraw_window(u32 id);
void gui_loop(void);

#endif
