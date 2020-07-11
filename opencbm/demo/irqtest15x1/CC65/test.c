/* vim: set noexpandtab tabstop=4 shiftwidth=4 autoindent smartindent: */

#include "opencbm.h"

#include <stdio.h>
#include <string.h>

#include <cbm.h>

void getstatus()
{
	unsigned char buffer[45];
	int read;

	cbm_talk(0, 8,15);

	read = cbm_raw_read(0, buffer, sizeof buffer);

	if (read >= 0) {
		buffer[read] = 0;
	}
	else {
		buffer[0] = 0;
	}

	printf("Read %d byte: '%s'\n", read, buffer);

	cbm_untalk(0);
}

#define MAX_COL 8
void dump(unsigned char * buffer, unsigned int len, unsigned int base)
{
	unsigned int offset = 0;

	for (offset = 0; offset < len; ) {
		unsigned int col;

		printf("%04X: ", base + offset);
		for (col = 0; offset < len && col < MAX_COL; offset++, col++) {
			printf("%02X ", (unsigned int) buffer[offset] );
		}
		printf("\n");
	}
}

#define DRV_BASE 0xc000
#define DRV_LEN 0x200
static unsigned char buffer[2048];

int main()
{
	int read;

	getstatus();
	getstatus();

#if 0
	cbm_exec_command(1, 8, "uj", 0);

	cbm_k_basin();
	printf("Continuing...\n");

#else

	read = cbm_download(0, 8, DRV_BASE, buffer, DRV_LEN);
	printf("read %d byte:\n", read);

	if (read == DRV_LEN) {
		read = cbm_upload(0, 8, 0x300, buffer, DRV_LEN);
		printf("uploaded %d byte.\n", read);
	}

	if (read == DRV_LEN) {
		read = cbm_download(0, 8, 0x300, &buffer[0x400], DRV_LEN);
		printf("read %d byte:\n", read);
	}

	if (read >= 0) {
		dump(&buffer[0x400], read, DRV_BASE);
	}
#endif

	getstatus();
	getstatus();

	return 0;
}
