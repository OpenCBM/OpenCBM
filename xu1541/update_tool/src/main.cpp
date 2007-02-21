/*
  Flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "cflashmem.h"
#include "cbootloader.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "usage: xu1541_update firmware.hex\n");
    exit(1);
  }

  CBootloader * bootloader = new CBootloader();
  unsigned int pagesize = bootloader->getPagesize();
  
  printf("Found xu1541 in boot loader mode, updating:\n");
  if(pagesize != 64) {
    fprintf(stderr, "Unexpected page size: %d\n", pagesize);
    exit(1);
  }

  CFlashmem * flashmem = new CFlashmem(pagesize);

  flashmem->readFromIHEX(argv[1]);

  /* make sure image is relocated correctly */
  if(flashmem->getFirstpage()->getPageaddress()) {
    fprintf(stderr, "Error: Image starts at address $%x != 0\n", 
	    flashmem->getFirstpage()->getPageaddress());
    exit(1);
  }
    
  /* make sure image fits into chip (8k - 2k boot loader) */
  if(flashmem->getLastpage()->getPageaddress() + pagesize - 1 > (8192-2048-1)) {
    fprintf(stderr, "Error: Image too long, ends at $%x > $%x\n", 
	    flashmem->getLastpage()->getPageaddress() + pagesize - 1,
	    (8192-2048-1));

    exit(1);
  }

  CPage* pPage = flashmem->getFirstpage();
  while (pPage != NULL) {
    printf("."); fflush(stdout);
    

    bootloader->writePage(pPage);
    pPage = pPage->getNext();
  } 

  printf(" done\nPlease remove jumper switch and replug USB cable to return to normal operation!\n");

  return 0;
}
