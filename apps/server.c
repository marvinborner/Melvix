// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <net.h>
#include <print.h>
#include <str.h>

#define PORT 8000
#define RESP "HTTP/1.1 200\r\nContent-Length: 14\r\nConnection: close\r\n\r\n<h1>Hallo</h1>"

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
		printf("%s\n", buf);
		net_send(socket, strdup(RESP), strlen(RESP));
		/* net_close(socket); // TODO: Fix */
	}

	return 0;
}
