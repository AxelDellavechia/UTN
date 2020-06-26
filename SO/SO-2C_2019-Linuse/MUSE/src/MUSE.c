/*
 ============================================================================
 Name        : MUSE.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int tiempo = muse_init(0,0,0);
	puts(tiempo);
	free(tiempo);
	return EXIT_SUCCESS;
}
