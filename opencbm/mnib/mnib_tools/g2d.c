/* g2d.c - Converts G64 disk images to D64 format

   (C) 1999-2003 Markus Brenner <markus@brenner.de>

    Based on code by Andreas Boose <boose@unixserv.rz.fh-hannover.de>

    V 1.00   old monolithic version
    V 1.10   rewritten version using gcr.c helper functions
    V 1.20   adjusted to changed gcr functions
    V 1.21   error conversion improved (SYNC NOT FOUND)
    V 0.35   synced versioning with other mnib tools
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "../gcr.h"
#include "../version.h"


void usage(void)
{
    fprintf(stderr, "Usage: g2d g64image [d64image]\n\n");
    exit (-1);
}


int main(int argc, char **argv)
{
    FILE *fp_g64, *fp_d64;
    char g64name[1024], d64name[1024];
    int track, sector;
    BYTE id[3];
    BYTE gcr_track[7930], rawdata[260];
    BYTE *gcr_start, *gcr_cycle;
    BYTE errorinfo[MAXBLOCKSONDISK];
    BYTE errorcode;
    int save_errorinfo;
    unsigned long blockindex;
    int cycle_len;

    fprintf(stdout,
        "\ng2d - converts a G64 disk image into a standard D64 disk image.\n"
        "(C) 1999-2003 Markus Brenner.\n"
        "Version "VERSION"\n\n");

    id[0]=id[1]=id[2] = '\0';

    if (argc == 2)
    {
        char *dotpos;
        strcpy(g64name, argv[1]);
        strcpy(d64name, g64name);
        dotpos = strrchr(d64name, '.');
        if (dotpos != NULL)
            *dotpos = '\0';
        strcat(d64name, ".d64");
    }
    else if (argc == 3)
    {
        strcpy(g64name, argv[1]);
        strcpy(d64name, argv[2]);
    }
    else usage();

    fp_g64 = fp_d64 = NULL;

    fp_g64 = fopen(g64name, "rb");
    if (fp_g64 == NULL)
    {
        fprintf(stderr, "Cannot open G64 image %s.\n", g64name);
        goto fail;
    }

    if (fread(gcr_track, sizeof(BYTE), 8, fp_g64) < 8)
    {
        fprintf(stderr, "Cannot read header from G64 image.\n");
        goto fail;
    }

    if (memcmp(gcr_track, "GCR-1541", 8) != 0)
    {
        fprintf(stderr, "input file %s isn't a G64 image file!\n", g64name);
        goto fail;
    }

    fp_d64 = fopen(d64name, "wb");
    if (fp_d64 == NULL)
    {
        fprintf(stderr, "Cannot open D64 image %s.\n", d64name);
        goto fail;
    }

    /* skip header and track offset information */
    fseek(fp_g64, 12 + MAX_TRACKS_1541 * 16, SEEK_SET);

    /* first, try to figure out ID from track 18 */
    fseek(fp_g64, 17 * 7930, SEEK_CUR);
    if (fread(gcr_track, 7930, 1, fp_g64) < 1)
    {
        fprintf(stderr, "Cannot read track from G64 image.\n");
        goto fail;
    }
    if (!extract_id(gcr_track, id))
    {
        fprintf(stderr, "Cannot find directory sector.\n");
        goto fail;
    }
    printf("ID: %02x%02x\n",id[0],id[1]);

    /* skip header and track offset information */
    fseek(fp_g64, 12 + MAX_TRACKS_1541 * 16, SEEK_SET);

    blockindex = 0;
    save_errorinfo = 0;
    for (track = 0; track < 35; track++)
    {

    	/* read in one track */
        if (fread(gcr_track, 7930, 1, fp_g64) < 1)
        {
            fprintf(stderr, "Cannot read track from G64 image.\n");
            goto fail;
        }

        cycle_len = (gcr_track[1] << 8) | gcr_track[0];
        printf("\nTrack: %2d - Sector: ",track+1);

        gcr_start = gcr_track+2;
        gcr_cycle = gcr_track+2+cycle_len;

        for (sector = 0; sector < sector_map_1541[track + 1]; sector++)
        {
	        printf("%d",sector);

            errorcode = convert_GCR_sector(gcr_start, gcr_cycle,
                                           rawdata,
                                           track + 1, sector, id);

            errorinfo[blockindex] = errorcode;	/* OK by default */
            if (errorcode != OK) save_errorinfo = 1;

            if (errorcode == OK)
                printf(" ");
            else
                printf("%d",errorcode);

            if (fwrite((char *) rawdata+1, 256, 1, fp_d64) != 1)
            {
                fprintf(stderr, "Cannot write sector data.\n");
                goto fail;
            }

            blockindex++;
        }
    }

    /* Missing: Track 36-40 detection */

    if (save_errorinfo)
    {
        if (fwrite((char *) errorinfo, BLOCKSONDISK, 1, fp_d64) != 1) {
            fprintf(stderr, "Cannot write error information.\n");
            goto fail;
        }
    }

fail:
    if (fp_g64 != NULL) fclose(fp_g64);
    if (fp_d64 != NULL) fclose(fp_d64);
    return -1;
}
