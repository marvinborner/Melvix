// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <net.h>
#include <print.h>
#include <str.h>
#include <sys.h>

int main(int argc, char **argv)
{
	(void)argc;
	/* printf("ARGC: %d\n", argc); */
	/* printf("[%s loaded]\n", argv[0]); */

	int wm = exec("/bin/wm", "wm", argv[1], NULL);
	int exec = exec("/bin/exec", "test", NULL);

	/* #define http_req "GET / HTTP/1.1\r\nHost: google.de\r\n\r\n" */
	/* 	struct socket *socket = net_open(S_TCP); */
	/* 	if (socket && net_connect(socket, ip(91, 89, 253, 227), 80)) */
	/* 		net_send(socket, strdup(http_req), strlen(http_req)); */
	/* 	else */
	/* 		print("Couldn't connect!\n"); */
	dns_request("google", "de");

	return wm + exec;
}
