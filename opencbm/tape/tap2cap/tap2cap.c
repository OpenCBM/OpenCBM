/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012 Arnd Menge, arnd(at)jonnz(dot)de
*/

#include <arch.h>
#include <stdio.h>
#include <stdlib.h>

#include "cbmtap2cap.h"
#include "cap.h"
#include "tap-cbm.h"

void usage(void)
{
	printf("\nUsage:   tap2cap <input.tap> <output.cap>\n\n");
	printf("Example: tap2cap myfile.tap myfile.cap\n");
}


int Evaluate_Commandline_Params(int argc, char *argv[])
{
	if (argc == 3) return 0;
	else return -1;
}


// Main routine.
//   Return values:
//    0: conversion finished ok
//   -1: an error occurred
int ARCH_MAINDECL main(int argc, char *argv[])
{
	HANDLE hCAP, hTAP;
	int    FuncRes, ret = -1;

	printf("\nTAP2CAP v1.00 - TAP image to ZoomTape CAP image conversion\n");
	printf("Copyright 2012 Arnd Menge\n\n");

	if (Evaluate_Commandline_Params(argc, argv) == -1)
	{
		usage();
		goto exit;
	}

	// Open specified TAP image file for reading.
	Check_TAP_CBM_Error_TextGoto(TAP_CBM_OpenFile(&hTAP, argv[1]), exit);

	// Check if specified CAP image file is already existing.
	if (CAP_isFilePresent(argv[2]) == CAP_Status_OK)
	{
		printf("Overwrite existing file? (y/N)");
		if (getchar() != 'y')
		{
			TAP_CBM_CloseFile(&hTAP);
			goto exit;
		}
		printf("\n");
	}

	// Create specified CAP image file for writing.
	FuncRes = CAP_CreateFile(&hCAP, argv[2]);
	if (FuncRes != CAP_Status_OK)
	{
		CAP_OutputError(FuncRes);
		TAP_CBM_CloseFile(&hTAP);
		goto exit;
	}

	printf("Converting: %s -> %s\n\n", argv[1], argv[2]);

	// Convert CBM TAP to CAP format.
	ret = CBMTAP2CAP(hCAP, hTAP);

	TAP_CBM_CloseFile(&hTAP);

	FuncRes = CAP_CloseFile(&hCAP);
	if (FuncRes != CAP_Status_OK)
		CAP_OutputError(FuncRes);

	if (ret == 0)
		printf("Conversion successful.");

    exit:
   	printf("\n");
   	return ret;
}
