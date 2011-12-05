
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include <unistd.h>
#ifdef WIN32
	#include <windows.h>
#endif

uint8_t hexdec(const char* pc)
{
	return (((pc[0] >= 'A') ? ( pc[0] - 'A' + 10 ) : ( pc[0] - '0' ) ) << 4 |
		((pc[1] >= 'A') ? ( pc[1] - 'A' + 10 ) : ( pc[1] - '0' ) )) & 0x0FF;
	
}

void dumpHex(uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;

	for (i = 0; i < len; i++){
		printf("%02X ", buf[i]);
	}
	printf("\n");
}

void *safe_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		exit(1);
	}

	return ptr;
}

#ifdef XXWIN32
int usleep(unsigned long x) {
	Sleep(x + 999 / 1000);
}
#endif
