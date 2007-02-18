/*
  cflashmem.h - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cpage.h"

class CFlashmem {
 public:
  CFlashmem(unsigned int pagesize);
  ~CFlashmem();
  CPage* getPageToAddress(unsigned int nAddress);
  void insertData(unsigned int nAddress, unsigned char bData);
  void display();
  void readFromIHEX(char* filename);
  CPage * getFirstpage();

 protected:
  unsigned int m_nPagesize;
  unsigned int m_nPagecount;
  CPage* m_pFirstpage;
  CPage* m_pLastpage;
  
};
