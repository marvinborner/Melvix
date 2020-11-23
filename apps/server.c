// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <mem.h>
#include <net.h>
#include <print.h>
#include <str.h>

#define PORT 8000
#define PATH "/res/www"
#define ERROR PATH "/404.html"

int main()
{
	printf("Server running on port %d\n", PORT);

	while (1) {
		struct socket *socket = net_open(S_TCP);
		assert(socket);
		socket->src_port = PORT;
		socket->state = S_CONNECTED;

		char buf[4096] = { 0 };
		net_receive(socket, buf, 4096);

		char path[128] = { 0 };
		strcat(path, PATH);
		http_query_path(buf, path);
		if (strlen(path) == strlen(PATH) + 1)
			strcat(path, "index.html");

		memset(buf, 0, 4096);

		u32 len = 0;
		if ((len = stat(path)))
			len = http_response(HTTP_200, len, read(path), buf);
		else
			len = http_response(HTTP_404, stat(ERROR), read(ERROR), buf);

		net_send(socket, buf, len);
		net_close(socket);
	}

	return 0;
}
