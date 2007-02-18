/*
  cpage.h - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#ifndef _H_CPAGE_
#define _H_CPAGE_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

class CPage {
 public:
  CPage(unsigned int pageaddress, unsigned int pagesize);
  ~CPage();

  unsigned int getPageaddress();
  unsigned int getPagesize();
  unsigned char* getData();
  CPage* getPrev();
  CPage* getNext();
  CPage* insert(unsigned int nAddress, unsigned char bValue);
  void display();
  void setPrev(CPage* pPage);
  void setNext(CPage* pPage);

 protected:
  int m_nPageaddress;
  int m_nPagesize;
  unsigned char * m_pData;
  CPage* m_pPrevpage;
  CPage* m_pNextpage;
};

#endif
