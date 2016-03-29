/*
 ============================================================================
 Name        : TallerTP1.c
 Author      : Freddy
 Version     :
 Copyright   : Do not copy
 Description : TP1 Taller 1C2016
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "rSync.h"

#define h_addr h_addr_list[0] /* backwards compatibility */
#define BUFFER_LEN 1024
#define EXIT_BAD_INVOCATION 1

int main(int argc, char** argv) {
	//llamado como server
	if ((argc == 3) && (strncmp(argv[1], "server", 6) == 0)){
		return startServerAndWaitRequestForFile(argv[2]);
	}
	//llamado como cliente
	if ((argc == 8) && (strncmp(argv[1], "client", 6) == 0)){
		return requestFileFromServer(argv[2], argv[3], argv[4],
				argv[5], argv[6], argv[7]);
	}

	return EXIT_BAD_INVOCATION;
}
