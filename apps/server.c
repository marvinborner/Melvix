// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <mem.h>
#include <net.h>
#include <print.h>
#include <str.h>

#define PORT 8000
#define FILE "/res/index.html"

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
		memset(buf, 0, 4096);
		int l = http_response(HTTP_200, stat(FILE), read(FILE), buf);
		net_send(socket, buf, l);
		net_close(socket);
	}

	return 0;
}
