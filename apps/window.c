// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <gui.h>
#include <print.h>

int main(void)
{
	u32 win;
	assert((win = gui_new_window()) > 0);
	gui_loop();
	return 0;
}
