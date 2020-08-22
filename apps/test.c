#include <def.h>
#include <gui.h>
#include <print.h>

int main()
{
	print("[test loaded]\n");
	printf("TIME: %d\n", time());

	/* struct window *win = gui_new_window(); */
	msg_send(1, MSG_NEW_WINDOW, NULL);
	struct message *msg = msg_receive_loop();
	struct window *win = (struct window *)msg->data;

	// TODO: Fix window transmitting
	printf("\nReceived %d from %d\n", win->x, msg->src);
	printf("Received %d from %d\n", win->y, msg->src);
	printf("Received %d from %d\n", win->width, msg->src);
	printf("Received %d from %d\n", win->height, msg->src);
	printf("Received %d from %d\n", win->fb, msg->src);

	while (1) {
	};
	return 0;
}
