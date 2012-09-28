/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012 Arnd Menge, arnd(at)jonnz(dot)de
*/

#ifndef __CAP2CBMTAP_H_
#define __CAP2CBMTAP_H_

#include <Windows.h>

// Convert CAP to CBM TAP format.
int CAP2CBMTAP(HANDLE hCAP, HANDLE hTAP);

#endif
