// TYX_API.cpp : Defines the entry point for the console application.
//
#include <afx.h>
#include <stdio.h>
#include <string.h>
#include "ListenSocket.h"

int main(int argc, char* argv[])
{
	char p[1024];
	printf("init socket ...");
	init_socket();
	printf(" finished\nlistening ...\n\n");
	socket_listen();
	printf("input exit to end services.\n");
	while( strcmp( p, "exit") ){
		printf(">");
		scanf("%s", p);
	}
	close_socket();
	return 0;
}

