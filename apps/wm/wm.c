// MIT License, Copyright (c) 2020 Marvin Borner
// Window manager / compositor (does everything basically)

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <input.h>
#include <libgui/gfx.h>
#include <libgui/gui.h>
#include <libgui/msg.h>
#include <libgui/vesa.h>
#include <libtxt/keymap.h>
#include <list.h>
#include <rand.h>

#define WINDOW_MOVE_TIMEOUT 20

struct client {
	u32 conn; // Bus conn
};

struct window {
	u32 id;
	u32 shid;
	struct context ctx;
	struct client client;
	u32 flags;
	vec2 pos;
	vec2 pos_prev;
};

struct rectangle {
	vec2 pos1; // Upper left
	vec2 pos2; // Lower right
	void *data;
};

// Global vars ftw!
static u8 bypp = 4;
static struct vbe screen = { 0 };
static struct list *windows = NULL; // THIS LIST SHALL BE SORTED BY Z-INDEX!
static struct window *direct = NULL;
static struct window *wallpaper = NULL;
static struct window *cursor = NULL;
static struct window *focused = NULL;
static struct keymap *keymap = NULL;
static struct client wm_client = { 0 };
static struct {
	u8 shift : 1;
	u8 alt : 1;
	u8 ctrl : 1;
} special_keys = { 0 };
static struct {
	vec2 pos;
	u8 left : 1;
	u8 mid : 1;
	u8 right : 1;
} mouse = { 0 };

/**
 * 5head algorithms
 * Thanks to @LarsVomMars for the help
 */

static void windows_at_rec(vec2 pos1, vec2 pos2, struct list *list)
{
	u32 width = pos2.x - pos1.x;
	u32 height = pos2.y - pos1.y;
	vec2 rec_corners[] = {
		pos1,
		vec2_add(pos1, vec2(width, 0)),
		pos2,
		vec2_add(pos1, vec2(0, height)),
	};

	u8 cursor_found = 0;

	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;

		if ((win->flags & WF_NO_WINDOW) != 0)
			goto next;

		vec2 corners[] = {
			win->pos,
			vec2_add(win->pos, vec2(win->ctx.size.x, 0)),
			vec2_add(win->pos, vec2(win->ctx.size.x, win->ctx.size.y)),
			vec2_add(win->pos, vec2(0, win->ctx.size.y)),
		};

		for (u8 i = 0; i < 4; i++) {
			vec2 corner = corners[i];
			if ((pos1.x <= corner.x && pos1.y <= corner.y) &&
			    (pos2.x >= corner.x && pos2.y >= corner.y)) {
				if (win == cursor)
					cursor_found = 1;
				else
					list_add(list, win);
				goto next;
			}
		}

		vec2 win_pos1 = win->pos;
		vec2 win_pos2 = vec2_add(win->pos, win->ctx.size);
		for (u8 i = 0; i < 4; i++) {
			vec2 corner = rec_corners[i];
			if ((win_pos1.x <= corner.x && win_pos1.y <= corner.y) &&
			    (win_pos2.x >= corner.x && win_pos2.y >= corner.y)) {
				if (win == cursor)
					cursor_found = 1;
				else
					list_add(list, win);
				goto next;
			}
		}

	next:
		iterator = iterator->next;
	}

	if (cursor_found)
		list_add(list, cursor);
}

static struct rectangle rectangle_at(vec2 pos1, vec2 pos2)
{
	u32 width = ABS(pos2.x - pos1.x);
	u32 height = ABS(pos2.y - pos1.y);
	u32 pitch = width * bypp;
	u8 *data = zalloc(width * height * bypp);

	struct list *windows_at = list_new();
	windows_at_rec(pos1, pos2, windows_at);
	struct node *iterator = windows_at->head;
	while (iterator) {
		struct window *win = iterator->data;

		s32 start_x = win->pos.x - pos1.x;
		u32 end_x = width;
		if (start_x <= 0) { // Either right side or background
			u32 right = start_x + win->ctx.size.x;
			if (right <= width) { // Right side
				start_x = 0;
				end_x = right;
			} else { // Background
				start_x = 0;
			}
		}

		s32 start_y = win->pos.y - pos1.y;
		u32 end_y = height;
		if (start_y <= 0) { // Either bottom side or background
			u32 bot = start_y + win->ctx.size.y;
			if (bot <= height) { // Bottom side
				start_y = 0;
				end_y = bot;
			} else { // Background
				start_y = 0;
			}
		}

		vec2 pos = vec2_sub(pos1, win->pos);

		u8 *srcfb =
			&win->ctx.fb[(pos.x + start_x) * bypp + (pos.y + start_y) * win->ctx.pitch];
		u8 *destfb = &data[start_x * bypp + start_y * pitch];

		// Copy window data to rectangle buffer
		for (u32 cy = start_y; cy < end_y; cy++) {
			u32 diff = 0;
			for (u32 cx = start_x; cx < end_x; cx++) {
				if (srcfb[bypp - 1])
					memcpy(destfb, srcfb, bypp);

				srcfb += bypp;
				destfb += bypp;
				diff += bypp;
			}
			srcfb += win->ctx.pitch - diff;
			destfb += pitch - diff;
		}

		iterator = iterator->next;
	}
	list_destroy(windows_at);

	return (struct rectangle){ .pos1 = pos1, .pos2 = pos2, .data = data };
}

static void rectangle_redraw(vec2 pos1, vec2 pos2)
{
	assert(pos1.x <= pos2.x && pos1.y <= pos2.y);
	struct rectangle rec = rectangle_at(pos1, pos2);

	u32 width = ABS(pos2.x - pos1.x);
	u32 height = ABS(pos2.y - pos1.y);

	if (!width || !height)
		return;

	u8 *srcfb = rec.data;
	u8 *destfb = &direct->ctx.fb[rec.pos1.x * bypp + rec.pos1.y * direct->ctx.pitch];
	for (u32 cy = 0; cy < height; cy++) {
		memcpy(destfb, srcfb, width * bypp);
		srcfb += width * bypp;
		destfb += direct->ctx.pitch;
	}

	free(rec.data);
}

/**
 * Window operations
 */

static struct window *window_new(struct client client, struct vec2 pos, struct vec2 size, u32 flags)
{
	struct window *win = malloc(sizeof(*win));
	static u32 id = 0;
	win->id = id++;
	win->ctx.size = size;
	win->ctx.bpp = screen.bpp;
	win->ctx.pitch = size.x * bypp;
	win->ctx.bytes = win->ctx.pitch * win->ctx.size.y;
	if ((flags & WF_NO_FB) != 0) {
		win->ctx.fb = NULL;
	} else {
		assert(shalloc(win->ctx.bytes, (u32 *)&win->ctx.fb, &win->shid) == EOK);
		memset(win->ctx.fb, COLOR_BLACK, win->ctx.bytes);
	}
	win->client = client;
	win->flags = flags;
	win->pos = pos;
	win->pos_prev = pos;
	list_add(windows, win);
	return win;
}

static struct window *window_find(u32 id)
{
	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if (win->id == id)
			return win;
		iterator = iterator->next;
	}
	return NULL;
}

static struct window *window_at(vec2 pos)
{
	struct window *ret = NULL;

	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if (!(win->flags & (WF_NO_WINDOW | WF_NO_FOCUS)) && pos.x >= win->pos.x &&
		    pos.x <= win->pos.x + win->ctx.size.x && pos.y >= win->pos.y &&
		    pos.y <= win->pos.y + win->ctx.size.y)
			ret = win;
		iterator = iterator->next;
	}
	return ret;
}

// Transparent windows can't use efficient rectangle-based redrawing (I think!)
static void window_redraw_alpha(struct window *win)
{
	vec2 pos1 = win->pos_prev;
	vec2 pos2 = vec2(pos1.x + win->ctx.size.x, pos1.y + win->ctx.size.y);

	rectangle_redraw(pos1, pos2);
	gfx_ctx_on_ctx(&direct->ctx, &win->ctx, win->pos, GFX_ALPHA);
}

// TODO: Make more efficient...
static void window_redraw_non_alpha(struct window *win)
{
	s32 diff_x = win->pos_prev.x - win->pos.x;
	s32 diff_y = win->pos_prev.y - win->pos.y;

	if (!diff_x && !diff_y) {
		gfx_ctx_on_ctx(&direct->ctx, &win->ctx, win->pos, GFX_NON_ALPHA);
		return;
	}

	vec2 pos1 = { 0 };
	vec2 pos2 = { 0 };

	/**
	 * Redraw left/right diff rectangle (only one!)
	 */

	if (diff_x <= 0) { // Right
		pos1.x = win->pos_prev.x;
		pos2.x = pos1.x - diff_x + 2;
	} else if (diff_x > 0) { // Left
		pos1.x = win->pos.x + win->ctx.size.x;
		pos2.x = pos1.x + diff_x + 2; // TODO: Why +2?
	}

	if (diff_y <= 0) { // Down
		pos1.y = win->pos_prev.y;
		pos2.y = win->pos.y + win->ctx.size.y;
	} else if (diff_y > 0) { // Up
		pos1.y = win->pos.y;
		pos2.y = win->pos_prev.y + win->ctx.size.y;
	}

	rectangle_redraw(pos1, pos2);

	/**
	 * Redraw bottom/top diff rectangle (only one!)
	 */

	if (diff_y <= 0) { // Down
		pos1.y = win->pos_prev.y;
		pos2.y = pos1.y - diff_y;
	} else if (diff_y > 0) { // Up
		pos1.y = win->pos.y + win->ctx.size.y;
		pos2.y = pos1.y + diff_y;
	}

	if (diff_x <= 0) { // Right
		pos1.x = win->pos_prev.x;
		pos2.x = win->pos.x + win->ctx.size.x;
	} else if (diff_x > 0) { // Left
		pos1.x = win->pos.x;
		pos2.x = win->pos_prev.x + win->ctx.size.x;
	}

	rectangle_redraw(pos1, pos2);

	// Redraw window on top of everything
	gfx_ctx_on_ctx(&direct->ctx, &win->ctx, win->pos, GFX_NON_ALPHA);
}

static void window_redraw(struct window *win)
{
	if (win->flags & WF_ALPHA)
		window_redraw_alpha(win);
	else
		window_redraw_non_alpha(win);
}

// TODO: Fix strange artifacts after destroying
static void window_destroy(struct window *win)
{
	memset(win->ctx.fb, 0, win->ctx.bytes);
	rectangle_redraw(win->pos, vec2_add(win->pos, win->ctx.size));
	list_remove(windows, list_first_data(windows, win));
	sys_free(win->ctx.fb);
	free(win);
}

/**
 * Event handlers
 */

static void handle_event_keyboard(struct event_keyboard *event)
{
	if (event->magic != KEYBOARD_MAGIC) {
		log("Keyboard magic doesn't match!\n");
		return;
	}

	if (event->scancode == KEY_LEFTSHIFT || event->scancode == KEY_RIGHTSHIFT)
		special_keys.shift ^= 1;
	else if (event->scancode == KEY_LEFTALT || event->scancode == KEY_RIGHTALT)
		special_keys.alt ^= 1;
	else if (event->scancode == KEY_LEFTCTRL || event->scancode == KEY_RIGHTCTRL)
		special_keys.ctrl ^= 1;

	if (event->scancode > KEYMAP_LENGTH)
		return;

	char ch;
	if (special_keys.shift)
		ch = keymap->shift_map[event->scancode];
	else if (special_keys.alt)
		ch = keymap->alt_map[event->scancode];
	else
		ch = keymap->map[event->scancode];

	UNUSED(ch);
}

static struct timer mouse_timer = { 0 };
static void handle_event_mouse(struct event_mouse *event)
{
	if (event->magic != MOUSE_MAGIC) {
		log("Mouse magic doesn't match!\n");
		return;
	}

	cursor->pos_prev = mouse.pos;

	if (event->rel) {
		mouse.pos.x += (s32)event->pos.x;
		mouse.pos.y -= (s32)event->pos.y;
	} else {
		// TODO: Support other absolute scaling than 0xffff (VMWare default)
		mouse.pos.x = event->pos.x * screen.width / 0xffff;
		mouse.pos.y = event->pos.y * screen.height / 0xffff;
	}

	// Fix x overflow
	if ((signed)mouse.pos.x < 0)
		mouse.pos.x = 0;
	else if (mouse.pos.x + cursor->ctx.size.x > (unsigned)screen.width - 1)
		mouse.pos.x = screen.width - cursor->ctx.size.x - 1;

	// Fix y overflow
	if ((signed)mouse.pos.y < 0)
		mouse.pos.y = 0;
	else if (mouse.pos.y + cursor->ctx.size.y > (unsigned)screen.height - 1)
		mouse.pos.y = screen.height - cursor->ctx.size.y - 1;

	cursor->pos = mouse.pos;

	struct window *win = window_at(mouse.pos);
	if (win && !(win->flags & WF_NO_FOCUS) && !event->but.left && !event->but.right &&
	    !event->but.middle)
		focused = win;

	if (focused && !(focused->flags & WF_NO_DRAG) && event->but.left && special_keys.alt) {
		struct timer timer = { 0 };
		io_read(IO_TIMER, &timer, 0, sizeof(timer));
		if (timer.time - mouse_timer.time > WINDOW_MOVE_TIMEOUT) {
			focused->pos_prev = focused->pos;
			focused->pos = mouse.pos;
			window_redraw(focused);
			mouse_timer = timer;
		}
		return;
	} else if (!vec2_eq(cursor->pos, cursor->pos_prev)) {
		window_redraw(cursor);
	}

	if (!win)
		return;

	struct message_mouse msg = { 0 };
	msg.header.state = MSG_GO_ON;
	msg.id = win->id;
	msg.pos = vec2_sub(mouse.pos, win->pos);
	msg.bits.click = event->but.left;

	if (msg_connect_conn(win->client.conn) == EOK)
		msg_send(GUI_MOUSE, &msg, sizeof(msg));
	else
		log("Failed to connect to window\n");
}

/**
 * Message handlers
 */

static void handle_message_new_window(struct message_new_window *msg)
{
	struct window *win = window_new((struct client){ .conn = msg->header.bus.conn },
					vec2(500, 600), vec2(600, 400), 0);
	msg->ctx = win->ctx;
	msg->shid = win->shid;
	msg->id = win->id;

	if (msg->header.state == MSG_NEED_ANSWER) {
		msg_connect_conn(msg->header.bus.conn);
		msg_send(GUI_NEW_WINDOW | MSG_SUCCESS, msg, sizeof(*msg));
	}
}

static void handle_message_redraw_window(struct message_redraw_window *msg)
{
	u32 id = msg->id;
	struct window *win = window_find(id);
	if (!win || win->client.conn != msg->header.bus.conn) {
		if (msg->header.state == MSG_NEED_ANSWER) {
			msg_connect_conn(msg->header.bus.conn);
			msg_send(GUI_REDRAW_WINDOW | MSG_FAILURE, msg, sizeof(msg->header));
		}
		return;
	}

	window_redraw(win);

	if (msg->header.state == MSG_NEED_ANSWER) {
		msg_connect_conn(msg->header.bus.conn);
		msg_send(GUI_REDRAW_WINDOW | MSG_SUCCESS, msg, sizeof(msg->header));
	}
}

static void handle_message_destroy_window(struct message_destroy_window *msg)
{
	u32 id = msg->id;
	struct window *win = window_find(id);
	if (!win || win->client.conn != msg->header.bus.conn) {
		if (msg->header.state == MSG_NEED_ANSWER) {
			msg_connect_conn(msg->header.bus.conn);
			msg_send(GUI_DESTROY_WINDOW | MSG_FAILURE, msg, sizeof(msg->header));
		}
		return;
	}

	window_destroy(win);

	if (msg->header.state == MSG_NEED_ANSWER) {
		msg_connect_conn(msg->header.bus.conn);
		msg_send(GUI_DESTROY_WINDOW | MSG_SUCCESS, msg, sizeof(msg->header));
	}
}

static void handle_message(void *msg)
{
	struct message_header *header = msg;

	switch (header->type) {
	case GUI_NEW_WINDOW:
		handle_message_new_window(msg);
		break;
	case GUI_REDRAW_WINDOW:
		handle_message_redraw_window(msg);
		break;
	case GUI_DESTROY_WINDOW:
		handle_message_destroy_window(msg);
		break;
	default:
		log("Message type %d not implemented!\n", header->type);
		msg_connect_conn(header->bus.conn);
		msg_send(GUI_DESTROY_WINDOW | MSG_SUCCESS, msg, sizeof(header));
	}
}

static void handle_exit(void)
{
	if (keymap)
		free(keymap);

	if (screen.fb)
		memset(screen.fb, COLOR_RED, screen.height * screen.pitch);

	if (windows) {
		struct node *iterator = windows->head;
		while (iterator) {
			struct window *win = iterator->data;
			if (win->ctx.fb)
				sys_free(win->ctx.fb);
			free(win);
			iterator = iterator->next;
		}
		list_destroy(windows);
	}
}

/**
 * Main loop
 */

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	atexit(handle_exit);

	assert(io_control(IO_FRAMEBUFFER, IOCTL_FB_GET, &screen, sizeof(screen)) == EOK);
	log("WM loaded: %dx%d\n", screen.width, screen.height);
	wm_client = (struct client){ .conn = 0 };
	bypp = (screen.bpp >> 3);

	windows = list_new();
	keymap = keymap_parse("/res/keymaps/en.keymap");

	direct = window_new(wm_client, vec2(0, 0), vec2(screen.width, screen.height),
			    WF_NO_WINDOW | WF_NO_FB | WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);
	direct->ctx.fb = screen.fb;
	direct->flags ^= WF_NO_FB;
	wallpaper = window_new(wm_client, vec2(0, 0), vec2(screen.width, screen.height),
			       WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);
	cursor = window_new(wm_client, vec2(0, 0), vec2(32, 32),
			    WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE | WF_ALPHA);

	/* gfx_write(&direct->ctx, vec2(0, 0), FONT_32, COLOR_FG, "Loading Melvix..."); */
	while (1)
		;
	gfx_load_wallpaper(&wallpaper->ctx, "/res/wall.png");
	memset(cursor->ctx.fb, 0, cursor->ctx.bytes);
	gfx_load_wallpaper(&cursor->ctx, "/res/cursor.png");
	window_redraw(wallpaper);

	assert(io_control(IO_BUS, IOCTL_BUS_REGISTER, "wm") == EOK);

	assert(exec("chess", NULL) == EOK);

	u8 msg[1024] = { 0 };
	struct event_keyboard event_keyboard = { 0 };
	struct event_mouse event_mouse = { 0 };
	enum io_type listeners[] = { IO_KEYBOARD, IO_MOUSE, IO_BUS, 0 };

	while (1) {
		res poll_ret = 0;
		if ((poll_ret = io_poll(listeners)) >= 0) {
			if (poll_ret == IO_KEYBOARD) {
				if (io_read(IO_KEYBOARD, &event_keyboard, 0,
					    sizeof(event_keyboard)) > 0) {
					handle_event_keyboard(&event_keyboard);
					continue;
				}
			} else if (poll_ret == IO_MOUSE) {
				if (io_read(IO_MOUSE, &event_mouse, 0, sizeof(event_mouse)) > 0) {
					handle_event_mouse(&event_mouse);
					continue;
				}
			} else if (poll_ret == IO_BUS) {
				if (msg_receive(msg, sizeof(msg)) > 0) {
					handle_message(msg);
					continue;
				}
			}
		}
		panic("Poll/read error: %s\n", strerror(errno));
	}

	return 1;
}
