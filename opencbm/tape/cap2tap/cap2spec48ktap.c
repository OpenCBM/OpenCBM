/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012 Arnd Menge, arnd(at)jonnz(dot)de
*/

#include <arch.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include "cap.h"

// Define pulses.
#define ShortPulse 1
#define LongPulse  2
#define PausePulse 3

// Define waveforms.
#define HalfWave   0
#define ShortWave  1
#define LongWave   2
#define PauseWave  3
#define ErrorWave  4


// Convert CAP to Spectrum48K TAP format. *EXPERIMENTAL*
int CAP2SPEC48KTAP(HANDLE hCAP, FILE *TapFile)
{
	unsigned char    DBGFLAG = 0; // 1 = Debug output
	unsigned char    zb[10000000]; // Spectrum48K TAP image buffer. Should be dynamic size.
	unsigned char    ch = 0;
	unsigned __int64 ui64Delta, ui64Len;
	unsigned int     Timer_Precision_MHz;
	int              FuncRes;

	// Declare variables holding current/last pulse & wave information.
	unsigned int LastPulse  = PausePulse;
	unsigned int Pulse      = PausePulse;
	unsigned int Wave       = HalfWave;

	// Declare image pointers.
	unsigned int BlockStart = 0; // Data block start in image.
	unsigned int BlockPos   = 2; // Current position in image.

	// Declare bit & byte counters.
	unsigned int BitCount         = 0; // Data block bit counter.
	unsigned int ByteCount        = 0; // Data block data byte counter.
	unsigned int DataPulseCounter = 0; // Data block pulse counter.
	unsigned int BlockByteCounter = 0; // Data block byte counter.

	// Seek to start of image file and read image header, extract & verify header contents, seek to start of image data.
	Check_CAP_Error_TextRetM1(CAP_ReadHeader(hCAP));

	// Return timestamp precision from header.
	Check_CAP_Error_TextRetM1(CAP_GetHeader_Precision(hCAP, &Timer_Precision_MHz));

	// Skip first halfwave (time until first pulse starts).
	FuncRes = CAP_ReadSignal(hCAP, &ui64Delta, NULL);
	if (FuncRes == CAP_Status_OK_End_of_file)
	{
		printf("Error: Empty image file.");
		return -1;
	}
	else if (FuncRes == CAP_Status_Error_Reading_data)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	// While CAP 5-byte timestamp available.
	while ((FuncRes = CAP_ReadSignal(hCAP, &ui64Delta, NULL)) == CAP_Status_OK)
	{
		ui64Len = (ui64Delta+(Timer_Precision_MHz/2))/Timer_Precision_MHz;

		if (DBGFLAG == 1) printf("%d ",ui64Len);
		
		LastPulse = Pulse;

		// Evaluate current pulse width.
		if ((150 <= ui64Len) && (ui64Len <= 360))
		{
			Pulse = ShortPulse;
			if (DBGFLAG == 1) printf("(SP) ");
		}
		else if ((360 < ui64Len) && (ui64Len < 550))
		{
			Pulse = LongPulse;
			if (DBGFLAG == 1) printf("(LP) ");
		}
		else // <150 or >550
		{
			Pulse = PausePulse;
			if (DBGFLAG == 1) printf("(PP) ");
		}


		if (Pulse == PausePulse)
		{
			DataPulseCounter = 0;
			BlockByteCounter = 0;

			if (ByteCount > 0)
			{
				// Calculate block size and write to TAP image.
				zb[BlockStart  ] = ByteCount & 0xff;
				zb[BlockStart+1] = (ByteCount >> 8) & 0xff;
				if (DBGFLAG == 1) printf("Block size = %u",ByteCount);
				BlockStart = BlockPos;
				BlockPos += 2;
			}
			ByteCount = 0;
			BitCount = 0;

		}
		else DataPulseCounter++;


		// Evaluate waveform after every second data pulse.
		if ((DataPulseCounter > 0) && ((DataPulseCounter % 2) == 0))
		{

			if ((LastPulse == ShortPulse) && (Pulse == ShortPulse))
			{
				Wave = ShortWave;
				if (DBGFLAG == 1) printf("(SW) ");
			}
			else if ((LastPulse == LongPulse) && (Pulse == LongPulse))
			{
				Wave = LongWave;
				if (DBGFLAG == 1) printf("(LW) ");
			}
			else
			{
				Wave = ErrorWave;
				if (DBGFLAG == 1) printf("(EW) ");
			}

			if ((Wave == ShortWave) || (Wave == LongWave))
				BlockByteCounter++;
			else
				BlockByteCounter = 0;


			if (BlockByteCounter > 1)
			{
				// We found a bit.
				BitCount++;

				// Evaluate wave.
				if (Wave == ShortWave)
				{
					ch = (ch << 1);
					if (DBGFLAG == 1) printf("(0)");
				}
				else if (Wave == LongWave)
				{
					ch = (ch << 1) + 1;
					if (DBGFLAG == 1) printf("(1)");
				}

				if (BitCount == 8)
				{
					ByteCount++; // Increase byte counter.
					BitCount = 0; // Reset bit counter.

					zb[BlockPos++] = ch; // Store byte to image.
					if (DBGFLAG == 1) printf(" -----> 0x%.2x <%c>",ch,ch);

					if (ByteCount == 1)
					{
						// Evaluate first block byte.
						if (DBGFLAG == 1)
						{
							if (ch == 0)
								printf(" [Header]");
							else if (ch == 0xff)
								printf(" [Data]");
							else
								printf(" [Unknown block!]");
						}
					}
				} // if (BitCount == 8)

			} // if (BlockCounter > 1)
			else if (DBGFLAG == 1) printf("(x)");
		} // if (DataPulseCounter > 0)
	} // while ((numread = fread(buf5, 5, 1, CapFile)) != 0)

	if (FuncRes == CAP_Status_Error_Reading_data)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	// Handle final data block, if exists.
	if (ByteCount > 0)
	{
		// Calculate block size and write to TAP image.
		zb[BlockStart  ] = ByteCount & 0xff;
		zb[BlockStart+1] = (ByteCount >> 8) & 0xff;
		if (DBGFLAG == 1) printf("Block size = %u\n",ByteCount);
		BlockStart = BlockPos;
		BlockPos += 2;
	}

	if (BlockPos > 2)
		fwrite(zb, (BlockPos-2), 1, TapFile);
	else
	{
		printf("Error: Empty image file.\n");
		return -1;
	}

	return 0;
}
