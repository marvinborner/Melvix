#include <def.h>
#include <gui.h>
#include <print.h>

int main()
{
	print("[test loaded]\n");

	/* struct window *win = gui_new_window(); */
	msg_send(1, MSG_NEW_WINDOW, NULL);
	struct message *msg = msg_receive_loop();
	struct window *win = (struct window *)msg->data;

	const u32 color[3] = { 0xff, 0, 0 };
	gui_fill(win, color);

	while (1) {
	};
	return 0;
}
