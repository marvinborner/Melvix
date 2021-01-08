// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <http.h>
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
		if (!net_receive(socket, buf, 4096, NET_NO_TIMEOUT))
			break;

		char path[128] = { 0 };
		strcat(path, PATH);
		http_query_path(buf, path);
		if (strlen(path) == strlen(PATH) + 1)
			strcat(path, "index.html");

		memset(buf, 0, 4096);

		struct stat s_file = { 0 };
		int res_file = stat(path, &s_file);

		struct stat s_error = { 0 };
		stat(ERROR, &s_error);

		int len;
		if (res_file == 0 && s_file.size)
			len = http_response(HTTP_200, s_file.size, sread(path), buf);
		else
			len = http_response(HTTP_404, s_error.size, sread(ERROR), buf);

		net_send(socket, buf, len);
		net_close(socket);
	}

	print("Server closed!\n");

	return 1;
}
