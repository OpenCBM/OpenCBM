/* mtool - Commodore 64 disk image tool

    (C) 2004-05 Pete Rittwage

    V 0.10   moved from main mnib code
*/

#define VERSION "1.0 BETA"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gcr.h"
#include "version.h"
#include "mnib.h"


BYTE diskbuf[84 * 0x2000];
BYTE diskbuf2[84 * 0x2000];

int length[84];
int length2[84];
int density[84];
int density2[84];
int fat_tracks[84];
int rapidlok_tracks[84];
int vmax_tracks[84];
int badgcr_tracks[84];
int fixgcr = 1;
int advanced_info = 0;
char bitrate_range[4] = { 43*2, 31*2, 25*2, 18*2 };


void usage(void)
{
    fprintf(stderr, "usage: mtool <filename> <filename>\n");

    exit(1);
}

int main(int argc, char *argv[])
{
    char file1[80];
    char file2[80];

    fprintf(stdout,
        "\nmtool - Commodore G64 disk image tool.\n"
        "(C) 2004-2004 Pete Rittwage\n"
        "Version "VERSION"\n\n");

	memset(file1,0,80);
	memset(file2,0,80);

    start_track = 1*2;
    end_track = 41*2;
    track_inc = 2;
    disktype = DISK_NORMAL;
    align = ALIGN_NONE;
    force_align = ALIGN_NONE;

    mode = 0;

    while (--argc && (*(++argv)[0] == '-'))
    {
        switch (tolower((*argv)[1]))
        {
			case 's':
				printf("ARG: Force sector 0 alignment\n");
			    force_align = ALIGN_SEC0;
            	break;

   			case 'f':
				printf("ARG: Do not fix weak bits\n");
			    fixgcr = 0;
            	break;

			case 'p':
				// custom protection handling
				printf("ARG: Custom copy protection mode = ");
				if( (*argv)[2] == 'x')
				{
					printf("V-MAX!\n");
					force_align = ALIGN_VMAX;
				}
				else if( (*argv)[2] == 'v')
				{
					printf("Epyx Vorpal\n");
					force_align = ALIGN_VORPAL;
				}
				else if( (*argv)[2] == 'r')
				{
					printf("Rapidlok\n");
					force_align = ALIGN_RAPIDLOK;
				}
				else
					printf("Unknown protection handler\n");
                break;

			case 'a':
				printf("ARG: Advanced info\n");
			    advanced_info = 1;
            	break;


			default:
                break;
        }
    }

    if (argc < 0) usage();
    strcpy(file1, argv[0]);
    printf("file1: %s\n",file1);

    if(argc>1)
    {
		mode = 1;  //compare
		strcpy(file2, argv[1]);
		printf("file2: %s\n",file2);
	}
	printf("\n");

	// clear buffers
 	memset(diskbuf, 0, sizeof(diskbuf) );
 	memset(diskbuf2, 0, sizeof(diskbuf2) );
 	memset(length, 0, sizeof(length) );
 	memset(length2, 0, sizeof(length2) );
 	memset(density, 0, sizeof(density) );
 	memset(density2, 0, sizeof(density2) );

	// compare
    if(mode == 1)
    {
		load_image(file1,1);
		load_image(file2,2);
		getch();
    	compare_disks();
	}
	// just scan for errors, etc.
	else
	{
		load_image(file1,1);
		getch();
		scandisk();
	}

	return 1;
}

int load_image(char *filename, int disknum)
{
	int i;
	FILE *fpin;
	char mnibheader[0x100];
	char g64header[0x2a0];

 	if ((fpin = fopen(filename,"rb")) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s!\n", filename);
		exit(2);
	}

	if (compare_extension(filename, "D64"))
	{
		imagetype = IMAGE_D64;
		load_d64(fpin, disknum);
  	}
	else if (compare_extension(filename, "G64"))
	{
		imagetype = IMAGE_G64;
  		fread(g64header, 1, 0x2ac, fpin);
  		load_gcr(fpin, g64header+0x9, disknum);
  	}
	else if (compare_extension(filename, "NIB"))
	{
		imagetype = IMAGE_NIB;
  		fread(mnibheader, 1, 0x100, fpin);
 		load_gcr(fpin, mnibheader+0x10, disknum);
  	}
	else
	{
		printf("Unknown image type = %s!\n", filename);
		exit(2);
	}

    fclose(fpin);
}

int load_d64(FILE *fpin, int disknum)
{
    int track, sector, sector_ref, i;
    BYTE buffer[256];
    BYTE gcrdata[0x2000];
    BYTE errorinfo[MAXBLOCKSONDISK];
    BYTE id[3] = { 0, 0, 0 };
    int error;
    int d64size;
    int last_track;
    char errorstring[256], tmpstr[8];

    /* here we get to rebuild tracks from scratch */
    memset(errorinfo, OK, MAXBLOCKSONDISK);

    /* determine d64 image size */
    fseek(fpin, 0, SEEK_END);
    d64size = ftell(fpin);
    switch (d64size)
    {
      case (BLOCKSONDISK * 257):       /* 35 track image with errorinfo */
        fseek(fpin, BLOCKSONDISK * 256, SEEK_SET);
        fread(errorinfo, sizeof(BYTE), BLOCKSONDISK, fpin);
      case (BLOCKSONDISK * 256):       /* 35 track image w/o errorinfo */
        last_track = 35;                /* fall through for both cases  */
        break;

      case (MAXBLOCKSONDISK * 257):    /* 40 track image with errorinfo */
        fseek(fpin, MAXBLOCKSONDISK * 256, SEEK_SET);
        fread(errorinfo, sizeof(BYTE), MAXBLOCKSONDISK, fpin);
      case (MAXBLOCKSONDISK * 256):    /* 40 track image w/o errorinfo */
        last_track = 40;                /* fall through for both cases  */
        break;

      default:
        rewind(fpin);
        fprintf(stderr, "Bad d64 image size.\n");
        return 0;
    }

    // determine disk id from track 18
    // $165A2, $165A3
    fseek(fpin, 0x165a2, SEEK_SET);
    fread(id, sizeof(BYTE), 2, fpin);
    printf("disk id: %s\n",id);

    rewind(fpin);

    sector_ref = 0;
    for (track = 1; track <= last_track; track ++)
    {
		// clear buffers
		memset(gcrdata, 0x55, 0x2000);
		errorstring[0] = '\0';

		for (sector = 0; sector < sector_map_1541[track]; sector++)
		{
			// get error and increase reference pointer in errorinfo
			error = errorinfo[sector_ref++];
			if(error != OK)
			{
				sprintf(tmpstr," E%dS%d",error,sector);
				strcat(errorstring,tmpstr);
			}

			// read sector from file
			fread(buffer, sizeof(BYTE), 256, fpin);

			// convert to gcr
			convert_sector_to_GCR(buffer, gcrdata + (sector * 361), track, sector, id, error);
		}

		if(disknum == 1)
		{
			length[track*2] = sector_map_1541[track] * 361;
			for (density[track*2] = 3; (track*2) >= bitrate_range[density[track*2]]; density[track*2]--);
			memcpy(diskbuf + (track * 2 * 0x2000), gcrdata, length[track*2]);
		}
		else
		{
			length2[track*2] = sector_map_1541[track] * 361;
			for (density2[track*2] = 3; (track*2) >= bitrate_range[density2[track*2]]; density2[track*2]--);
			memcpy(diskbuf2 + (track * 2 * 0x2000), gcrdata, length2[track*2]);
		}
		//printf("%s",errorstring);
    }
	return;
}

int load_gcr(FILE *fpin, char *track_header, int disknum)
{
    int track;
    int dens_pointer;
    int header_entry;
    int g64tracks, g64maxtrack;
    BYTE buffer[0x2000];
    BYTE gcrdata[0x2000];

    if (imagetype == IMAGE_NIB)
    {
		printf("Loading GCR image [");

        header_entry = 0;

        for (track = start_track; track <= end_track; track += track_inc)
        {
			// clear buffers
			memset(buffer, 0, 0x2000);
            memset(gcrdata, 0, 0x2000);

			// skip missing tracks
			if(track_header[header_entry*2] == 0)
			{
				printf("x");
				continue;
			}

			// halftrack skip
			if(track_header[header_entry*2] < track)
			{
				fseek(fpin, 0x2000, SEEK_CUR);
				header_entry++;
			}

           	// get track from file
           	align = ALIGN_NONE;
            fread(buffer,0x2000,1,fpin);
			printf("o");

            if(disknum == 1)
            {
				density[track] = (track_header[header_entry*2+1]);
            	length[track] = extract_GCR_track(gcrdata, buffer, &align, force_align);
				memcpy(diskbuf + (track * 0x2000), gcrdata, length[track]);
			}
			else
			{
				density2[track] = (track_header[header_entry*2+1]);
				length2[track] = extract_GCR_track(gcrdata, buffer, &align, force_align);
				memcpy(diskbuf2 + (track * 0x2000), gcrdata, length2[track]);
			}
			header_entry++;
		}
    	printf("] loaded\n");
    }
    else if (imagetype == IMAGE_G64)
    {

        g64tracks   = track_header[0];
        g64maxtrack = (BYTE)track_header[2] << 8 | (BYTE)track_header[1];

		// no 42 track images exist
		if(g64tracks > 82) g64tracks = 82;

        printf("G64: %d tracks, %d bytes each\n",g64tracks,g64maxtrack);

        dens_pointer=0;
        for (track = start_track; track <= g64tracks; track += track_inc)
        {
			// clear buffers
			memset(buffer, 0, 0x2000);
            memset(gcrdata, 0, 0x2000);

            /* get density from header or use default */
           	if(disknum == 1)
           		density[track] = track_header[0x153+dens_pointer];
            else
            	density2[track] = track_header[0x153+dens_pointer];

            dens_pointer += 8;

            /* get length */
            buffer[0] = fgetc(fpin);
            buffer[1] = fgetc(fpin);

            if(disknum == 1)
	           	length[track] = buffer[1] << 8 | buffer[0];
			else
				length2[track] = buffer[1] << 8 | buffer[0];

            /* get track from file */
            fread(gcrdata,1,g64maxtrack,fpin);

   			// store track in disk buffer
			if(disknum == 1)
				memcpy(diskbuf + (track * 0x2000), gcrdata, length[track]);
			else
				memcpy(diskbuf2 + (track * 0x2000), gcrdata, length2[track]);
    	}
    }
}


int compare_disks(void)
{
	int track;
	int numtracks = 0;
	int numsecs = 0;
	int sector, error, error2, i;
	BYTE secbuf[260];
	BYTE secbuf2[260];
	BYTE id[3];
	BYTE id2[3];
	char errorstring[256];

	int gcr_match, sec_match;
	int gcr_total = 0;
	int sec_total = 0;
	int trk_total = 0;
	char gcr_mismatches[256];
	char sec_mismatches[256];
	char tmpstr[16];

	start_track = (1*2);
	end_track = (41*2);

	gcr_mismatches[0] = '\0';
	sec_mismatches[0] = '\0';

	// extract disk id's from track 18
	memset(id, 0, 3);
	extract_id( diskbuf + (36 * 0x2000), id);
	memset(id2, 0, 3);
	extract_id( diskbuf2 + (36 * 0x2000), id2);

	printf("disk ID #1: %s\n",id);
	printf("disk ID #2: %s\n\n",id2);

	for (track = start_track; track <= end_track; track += track_inc)
	{
		printf("Track %2d, Disk 1: length:%d density:%d\n",
			track/2,length[track],(density[track]&3));

		printf("Track %2d, Disk 2: length:%d density:%d\n",
			track/2,length2[track],(density2[track]&3));

		if((length[track]>0) && (length2[track]>0))
		{
			numtracks ++;

			// check for gcr match (unlikely)
			gcr_match = compare_tracks(diskbuf+(track*0x2000), diskbuf2+(track*0x2000),
					length[track], length2[track], 0, errorstring);

			printf("%s\n",errorstring);

			if(gcr_match)
			{
				gcr_total ++;
				printf("[*GCR MATCH*] ");
			}
			else
			{
				printf("[*NO GCR MATCH*] ");
				sprintf(tmpstr,"%d,",track/2);
				strcat(gcr_mismatches,tmpstr);
			}

			// check for sector matches
			sec_match = compare_sectors(diskbuf+(track*0x2000), diskbuf2+(track*0x2000),
					length[track], length2[track], track, errorstring);

			printf("%s\n",errorstring);

			sec_total += sec_match;
			numsecs += sector_map_1541[(track/2)+1];

			if(sec_match)
			{
				trk_total++;
				printf("[*DATA MATCH*]");
			}
			else
			{
				printf("[*NO DATA MATCH*]");
				sprintf(tmpstr,"%d,",track/2);
				strcat(sec_mismatches,tmpstr);
				getch();
			}
			printf("\n");
		}
		printf("\n");
	}

	printf("%d/%d tracks had a perfect GCR match\n", gcr_total, numtracks);
	printf("Mismatches (%s)\n",gcr_mismatches);
	printf("%d/%d tracks matched all sector data\n", trk_total, numtracks);
	printf("Mismatches (%s)\n",sec_mismatches);
	printf("%d/%d sectors matched\n", sec_total, numsecs);
}

int compare_extension(char *filename, char *extension)
{
    char *dot;

    dot = strrchr(filename, '.');
    if (dot == NULL) return (0);

    for (++dot; *dot != '\0'; dot++, extension++)
        if (tolower(*dot) != tolower(*extension)) return (0);

    if (*extension == '\0') return (1);
    else return (0);
}


int scandisk(void)
{
	BYTE id[3];
	int i, j, k, l;
	int toggle = 0;
	int track = 0;
	int totalfat = 0;
	int totalrl = 0;
	int totalgcr = 0;
	int errors = 0, temp_errors;
	int defdensity;
	char *errorstring[256];

	// clear buffers
	memset(badgcr_tracks, 0, sizeof(badgcr_tracks) );
	memset(fat_tracks, 0, sizeof(fat_tracks) );
	memset(rapidlok_tracks, 0, sizeof(rapidlok_tracks) );
	errorstring[0] = '\0';

	// extract disk id from track 18
	memset(id, 0, 3);
	extract_id( diskbuf + (36 * 0x2000), id);

	printf("Scanning...\n");

	// check each track for various things
	for (track = start_track; track <= end_track; track += track_inc)
	{
		printf("\nTrack %2d: %d ", track/2, length[track]);

		if(length[track] > 0)
		{
			density[track] = check_sync_flags(diskbuf + (track * 0x2000),
								density[track]&3, length[track]);

			printf("(%d", density[track]&3);

			if(density[track] & BM_NO_SYNC)
				printf(":NOSYNC");
			else if (density[track] & BM_FF_TRACK)
				printf(":KILLER");

			// establish default density and warn
			for (defdensity = 3; track >= bitrate_range[defdensity]; defdensity--);

			if((density[track] & 3) != defdensity)
				printf("!=%d?",defdensity);

			printf(") ");

			if(density[track] & BM_FF_TRACK)
			{
				printf("\n");
				continue;
			}

			// detect bad GCR '000' bits
			if(fixgcr)
			{
				badgcr_tracks[track] = check_bad_gcr(diskbuf+(0x2000 * track),
					length[track], 1);

				if(badgcr_tracks[track])
				{
					printf("weak:%d ",badgcr_tracks[track]);
					totalgcr += badgcr_tracks[track];
				}
			}

			// check for rapidlok tracks
			rapidlok_tracks[track] = check_rapidlok(track);

			if(rapidlok_tracks[track]) totalrl++;

			if((totalrl) && (track == 72))
			{
				printf("RAPIDLOK KEYTRACK ");
				rapidlok_tracks[track] = 1;
			}

			// check for FAT track
			if(track < end_track + track_inc)
			{
				fat_tracks[track] = check_fat(track);
				if(fat_tracks[track]) totalfat++;
			}

			// check for regular disk errors
			// "second half" of fat track will always have header errors since it's encoded
			// for the wrong track number.
			// rapidlok tracks are not standard gcr
			if((!fat_tracks[track-2]) && (!rapidlok_tracks[track]))
				temp_errors = check_errors(diskbuf+(0x2000 * track),length[track],track,id,errorstring);

			if(advanced_info)
				raw_track_info(diskbuf+(0x2000 * track),length[track],errorstring);

			if(temp_errors)
			{
				errors += temp_errors;
				printf("%s",errorstring);
				getch();
			}
	  	}


		// dump to disk for manual compare
		char testfilename[16];
		sprintf(testfilename,"raw/tr%dd%d",track/2,(density[track] & 3));
		FILE *trkout = fopen(testfilename,"w");
		fwrite(diskbuf+(track*0x2000), length[track], 1, trkout);
		fclose(trkout);

	}
	printf("\n\ndisk id: %s\n",id);
	printf("%d disk errors detected.\n",errors);
	printf("%d bad/weak gcr bytes detected.\n",totalgcr);
	printf("%d fat tracks detected.\n",totalfat);
	printf("%d rapidlok tracks detected.\n",totalrl);
}

int raw_track_info(BYTE *gcrdata, int length, char *outputstring)
{
	int sync_cnt = 0;
	int sync_len[0x2000];
	int gap_cnt = 0;
	int gap_len[0x2000];
	int weak_cnt = 0;
	int weak_len[0x2000];
	int i,locked;

	memset(sync_len, 0, sizeof(sync_len));
	memset(gap_len, 0, sizeof(gap_len));
	memset(weak_len, 0, sizeof(weak_len));

	// count syncs/lengths
	for(locked=0,i=0; i<length-1; i++)
	{
		if(locked)
		{
			if(gcrdata[i] == 0xff) sync_len[sync_cnt]++;
			else locked = 0;
		}
		else if( ((gcrdata[i] & 0x03) == 0x03) && (gcrdata[i+1] == 0xff) )
		{
			locked = 1;
			sync_cnt++;
			sync_len[sync_cnt]=2;
		}
	}

	printf("\nSYNCS:%d (",sync_cnt);
	for(i=1;i<=sync_cnt;i++) printf("%d-",sync_len[i]);
	printf(")");

	// count gaps/lengths
	for(locked=0,i=0; i<length-1; i++)
	{
		if(locked)
		{
			if(gcrdata[i] == 0x55) gap_len[gap_cnt]++;
			else locked = 0;
		}
		else if((gcrdata[i] == 0x55) && (gcrdata[i+1] == 0x55))
		{
			locked = 1;
			gap_cnt++;
			gap_len[gap_cnt]=2;
		}
	}

	printf("\nGAPS :%d (",gap_cnt);
	for(i=1;i<=gap_cnt;i++) printf("%d-",gap_len[i]);
	printf(")");

	// count weaks/lengths
	for(locked=0,i=0; i<length-1; i++)
	{
		if(locked)
		{
			if(is_bad_gcr(gcrdata, length, i)) weak_len[weak_cnt]++;
			else locked = 0;
		}
		else if(is_bad_gcr(gcrdata, length, i))
		{
			locked = 1;
			weak_cnt++;
			weak_len[weak_cnt]=1;
		}
	}

	printf("\nWEAKS:%d (",weak_cnt);
	for(i=1;i<=weak_cnt;i++) printf("%d-",weak_len[i]);
	printf(")");



}

int check_fat(int track)
{
	int fat = 0;
	char errorstring[256];

	if((length[track] > 0) && (length[track+2] > 0))
	{
		fat = compare_tracks(diskbuf+(track*0x2000), diskbuf+((track+2)*0x2000),
				length[track], length[track+2], 1, errorstring );
	}

	if(fat)
		printf("*FAT* ");

	return fat;
}

int check_rapidlok(int track)
{
	// tries to detect and fixup rapidlok track, as the gcr routines
	// don't assemble them quite right.

	int i;
	int end_key = 0;
	int end_sync = 0;
	int locked = 0;

	int synclen = 0;
	int keylen = 0;  // extra sector with # of 0x7b
	BYTE extra_sector[0x200];
	char testfilename[0x10];

	int tlength = length[track];
	BYTE *gcrdata = diskbuf+(track * 0x2000);

	// extra sector is at the end.
	// count the extra-sector (key) bytes.
	for(i=0;i<200;i++)
	{
		if(gcrdata[tlength-i] == 0x7b)
		{
			keylen++;
			if(end_key) end_key=tlength-i;  // move marked end of key bytes
		}
		else if(keylen) break;
	}

	if(gcrdata[tlength-i] != 0xff) { keylen++; end_key++; }

	if(keylen < 0x8)
		return 0;  // only rapidlok tracks contain lots of these at start


	for(i=end_key+1; i<end_key+0x100; i++)
	{
		if(gcrdata[i] == 0xff)
		{
			synclen++;
			end_sync=i+1;  // mark end of sync
		}
		else if(synclen) break;
	}


	printf("RAPIDLOK! ");
	printf("key:%d, sync:%d...",keylen,synclen);


	/*
	// recreate key sector
	memset(extra_sector,0xff,0x14);
	memset(extra_sector+0x14, 0x7b, keylen);

	// create initial sync, then copy all sector data
	if(halftrack != 0x24) // if directory track no fancy stuff

	{

		memset(gcrdata, 0xff, 0x29);

		memcpy(gcrdata+0x29, gcrdata+end_sync, length-end_sync);
		memcpy(gcrdata+0x29+(length-end_sync), extra_sector, keylen+0x14);
	}
	else
	{
		memcpy(gcrdata, gcrdata+end_key, length-end_key);
		memcpy(gcrdata+(length-end_key), extra_sector, keylen+0x14);
	}

	*/

	return keylen;
}

