/*
  cpage.cpp - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include "cpage.h"

CPage::CPage(unsigned int pageaddress, unsigned int pagesize) {

  assert(pagesize > 0);

  m_nPagesize = pagesize;
  m_nPageaddress = pageaddress - (pageaddress % m_nPagesize);

  m_pData = new unsigned char[m_nPagesize];
  memset(m_pData, 0xff, m_nPagesize);

  m_pPrevpage = NULL;
  m_pNextpage = NULL;
}

CPage::~CPage() {
  assert(m_pData);
  delete m_pData;
}

unsigned int CPage::getPageaddress() {
  return m_nPageaddress;
}

unsigned int CPage::getPagesize() {
  return m_nPagesize;
}

unsigned char * CPage::getData() {
  return m_pData;
}

CPage* CPage::getPrev() {
  return m_pPrevpage;
}

CPage* CPage::getNext() {
  return m_pNextpage;
}

void CPage::setPrev(CPage* pPage) {
  m_pPrevpage = pPage;
}

void CPage::setNext(CPage* pPage) {
  m_pNextpage = pPage;
}

CPage* CPage::insert(unsigned int nAddress, unsigned char bValue) {
  assert(m_nPageaddress == (nAddress - (nAddress % m_nPagesize)));
  m_pData[nAddress % m_nPagesize] = bValue;
}

void CPage::display() {
  int n;
   printf("Page Adresse: %d\n", getPageaddress());

  for (n = 0; n < 64; n++)
    printf("%2X ", m_pData[n]);

  printf("\n----END OF PAGE----\n");
}
