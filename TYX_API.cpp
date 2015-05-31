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
	// ADOAccess *ADB;
	// ADB = new ADOAccess();
	// printf("%d\n", ADB->IdentityCheck("213120498","213120498"));
	// printf("%s\n", ADB->GetPaoCao("213120498"));
	// printf("%s\n", ADB->GetHealthScore("213121455"));
	// delete ADB;
	printf("input exit to end services.\n");
	while( strcmp( p, "exit") ){
		printf(">");
		scanf("%s", p);
	}
	close_socket();
	return 0;
}

