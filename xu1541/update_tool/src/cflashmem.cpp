/*
  cflashmem.cpp - part of flashtool for AVRUSBBoot, an USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  Creation Date..: 2006-03-18
  Last change....: 2006-06-25
*/

#include "cflashmem.h"

CFlashmem::CFlashmem(unsigned int pagesize) {
  m_nPagesize = pagesize;
  m_nPagecount = 0;
  m_pFirstpage = NULL;
  m_pLastpage = NULL;
}

CFlashmem::~CFlashmem() {
  
}

CPage * CFlashmem::getFirstpage() {
  return m_pFirstpage;
}

CPage* CFlashmem::getPageToAddress(unsigned int nAddress) {
  unsigned int nBaseaddress = nAddress - (nAddress % m_nPagesize);
  
  if (m_pFirstpage == NULL) return NULL;

  if (m_pLastpage->getPageaddress() == nBaseaddress) return m_pLastpage;

  CPage* pPage = m_pFirstpage;
  while (pPage != NULL) {
    if (pPage->getPageaddress() == nBaseaddress) return pPage;
    pPage = pPage->getNext();
  }

  return NULL;
}

void CFlashmem::insertData(unsigned int nAddress, unsigned char bData) {
  
  CPage* pPage = getPageToAddress(nAddress);
  if (pPage == NULL) {

    pPage = new CPage(nAddress, m_nPagesize);
    m_nPagecount++;

    if (m_pLastpage != NULL) {
      m_pLastpage->setNext(pPage);
      pPage->setPrev(m_pLastpage);
    } else {
      m_pFirstpage = pPage;
    }
    m_pLastpage = pPage;

  }
  pPage->insert(nAddress, bData);
}

void CFlashmem::display() {
  CPage* pPage = m_pFirstpage;
  while (pPage != NULL) {
    //printf("Page Adresse: %d\n", pPage->getPageaddress());
    pPage->display();
    pPage = pPage->getNext();
  }
}


int sscanhex( unsigned char *str, unsigned int *hexout, int n )
{
  unsigned int hex = 0, x = 0;
  for(; n; n--){
    x = *str;
    if( x >= 'a' )
      x += 10 - 'a';
    else if( x >= 'A' )
      x += 10 - 'A';
    else
      x -= '0';
    if( x >= 16 )
      break;
    hex = hex * 16 + x;
    str++;
  }
  *hexout = hex;
  return n;					// 0 if all digits read
}


int readhex( FILE *fp, unsigned int *addr, unsigned char *data){
  /* Return value: 1..255	number of bytes
			0	end or segment record
		       -1	file end
		       -2	error or no HEX-File */
  char hexline[524];				// intel hex: max 255 byte
  unsigned char * hp = (unsigned char *) hexline;
  unsigned int byte;
  int i;
  unsigned int num;

  if( fgets( hexline, 524, fp ) == NULL )
    return -1;					// end of file
  if( *hp++ != ':' )
    return -2;                                  // no hex record
  if( sscanhex( hp, &num, 2 ))
    return -2;					// no hex number
  hp += 2;
  if( sscanhex( hp, addr, 4 ))
    return -2;
  hp += 4;
  if( sscanhex( hp, &byte, 2 ))
    return -2;
  if( byte != 0 )				// end or segment record
    return 0;
  for( i = num; i--; ){
    hp += 2;
    if( sscanhex( hp, &byte, 2 ))
      return -2;
    *data++ = byte;
  }
  return num;
}


void CFlashmem::readFromIHEX(char* filename) {
  assert(filename);
  
  FILE* fp;
  if ((fp = fopen(filename, "rb")) == NULL) {
    printf("File %s open failed!\n", filename);
    exit(1);
  };

  int i;
  unsigned int addr;
  unsigned char data[255];

  while( (i = readhex( fp, &addr, data )) >= 0 ){
    if ( i ) {
      for (int j = 0; j < i; j++) {
	insertData(addr + j, data[j]);
      }
    }
  }

  fclose(fp);
}
