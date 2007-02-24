/*
  Flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  modified for xu1541 by Till Harbaum
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cflashmem.h"
#include "cbootloader.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("usage: xu1541_update firmware.hex\n");
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }

  CBootloader * bootloader = new CBootloader();
  unsigned int pagesize = bootloader->getPagesize();
  
  printf("Found xu1541 in boot loader mode, updating:\n");
  if(pagesize != 64) {
    printf("Unexpected page size: %d\n", pagesize);
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }

  CFlashmem * flashmem = new CFlashmem(pagesize);

  flashmem->readFromIHEX(argv[1]);

  /* make sure image is relocated correctly */
  if(flashmem->getFirstpage()->getPageaddress()) {
    printf("Error: Image starts at address $%x != 0\n", 
	    flashmem->getFirstpage()->getPageaddress());
#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }
    
  /* make sure image fits into chip (8k - 2k boot loader) */
  if(flashmem->getLastpage()->getPageaddress() + pagesize - 1 > (8192-2048-1)) {
    printf("Error: Image too long, ends at $%x > $%x\n", 
	    flashmem->getLastpage()->getPageaddress() + pagesize - 1,
	    (8192-2048-1));

#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(1);
  }

  CPage* pPage = flashmem->getFirstpage();
  while (pPage != NULL) {
    printf("."); fflush(stdout);
    
    bootloader->writePage(pPage);
    pPage = pPage->getNext();
  } 

  printf(" done\nPlease remove jumper switch and replug USB cable to return to normal operation!\n");

#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif

  return 0;
}
