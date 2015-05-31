// TYX_API.cpp : Defines the entry point for the console application.
//
#include <afx.h>
#include <stdio.h>
#include "AdoAccess.h"

int main(int argc, char* argv[])
{
	ADOAccess *ADB;
	ADB = new ADOAccess();
	printf("%d\n", ADB->IdentityCheck("213120498","213120498"));
	printf("%s\n", ADB->GetPaoCao("213120498"));
	printf("%s\n", ADB->GetHealthScore("213121455"));
	delete ADB;
	return 0;
}

