/*
    gcr.h - Group Code Recording helper functions

    (C) 2001-05 Markus Brenner <markus@brenner.de>
    	and Pete Rittwage <peter@rittwage.com>
        based on code by Andreas Boose

    V 0.33   improved sector extraction, added find_track_cycle() function
    V 0.34   added MAX_SYNC_OFFSET constant, approximated to 800 GCR bytes
    V 0.35   modified find_track_cycle() interface
    V 0.36   added bad GCR code detection
    V 0.36a  added find_sector_gap(), find_sector0(), extract_GCR_track()
    V 0.36d  Untold number of additions and consequent bugfixes. (pjr)

*/

#define BYTE unsigned char
#define DWORD unsigned int
#define MAX_TRACKS_1541 42

/* D64 constants */
#define BLOCKSONDISK 683
#define BLOCKSEXTRA 85
#define MAXBLOCKSONDISK (BLOCKSONDISK+BLOCKSEXTRA)
#define MAX_TRACK_D64 40

/* G64 constants */
#define G64_TRACK_MAXLEN 7928
//#define G64_TRACK_MAXLEN 8192 /* crashes VICE, works in CCS64 properly */
#define G64_TRACK_LENGTH (G64_TRACK_MAXLEN+2)

/* NIB format constants */
#define GCR_TRACK_LENGTH 0x2000

#define MATCH_LENGTH 7		// bytes to match to determine track cycle

/*
    number of GCR bytes until NO SYNC error
    timer counts down from $d000 to $8000 (20480 cycles)
    until timeout when waiting for a SYNC signal
    This is approx. 20.48 ms, which is approx 1/10th disk revolution
    8000 GCR bytes / 10 = 800 bytes
*/
//#define MAX_SYNC_OFFSET 800
/* this was too small for Lode Runner original (805), so increase to 820 */
//#define MAX_SYNC_OFFSET 820
#define MAX_SYNC_OFFSET 0x1500

#define SIGNIFICANT_GAPLEN_DIFF 0x20

#define GCR_BLOCK_HEADER_LEN 24
#define GCR_BLOCK_DATA_LEN   337
#define GCR_BLOCK_LEN (GCR_BLOCK_HEADER_LEN + GCR_BLOCK_DATA_LEN)

/* minimum amount of good sequential GCR for formatted track */
#define GCR_MIN_FORMATTED 64	// chessmaster track 29 is shortest so far

/* Disk Controller error codes */
#define SECTOR_OK           0x01
#define HEADER_NOT_FOUND    0x02
#define SYNC_NOT_FOUND      0x03
#define DATA_NOT_FOUND      0x04
#define BAD_DATA_CHECKSUM   0x05
#define BAD_GCR_CODE        0x06
#define VERIFY_ERROR        0x07
#define WRITE_PROTECTED     0x08
#define BAD_HEADER_CHECKSUM 0x09
#define ID_MISMATCH         0x0b
#define DISK_NOT_INSERTED   0x0f

#define BM_MATCH       0x10
#define BM_NO_CYCLE	   0x20
#define BM_NO_SYNC     0x40
#define BM_FF_TRACK    0x80

#define ALIGN_NONE			0
#define ALIGN_GAP			1
#define ALIGN_SEC0			2
#define ALIGN_LONGSYNC		3
#define ALIGN_WEAK			4
#define ALIGN_VMAX			5
#define ALIGN_AUTOGAP		6

#define GCR_MASK_BAD_FIRST 0
#define GCR_MASK_BAD_LAST 1

/* global variables */
extern char sector_map_1541[];
extern int speed_map_1541[];
extern int capacity[];
extern int capacity_min[];
extern int capacity_max[];

/* prototypes */
int find_sync(BYTE ** gcr_pptr, BYTE * gcr_end);
void convert_4bytes_to_GCR(BYTE * buffer, BYTE * ptr);
int convert_4bytes_from_GCR(BYTE * gcr, BYTE * plain);
int extract_id(BYTE * gcr_track, BYTE * id);
int find_track_cycle(BYTE ** cycle_start, BYTE ** cycle_stop, int cap_min,
  int cap_max);
int find_nondos_track_cycle(BYTE ** cycle_start, BYTE ** cycle_stop,
  int cap_min, int cap_max);
BYTE convert_GCR_sector(BYTE * gcr_start, BYTE * gcr_end,
  BYTE * d64_sector, int track, int sector, BYTE * id);
void convert_sector_to_GCR(BYTE * buffer, BYTE * ptr,
  int track, int sector, BYTE * diskID, int error);
BYTE * find_sector_gap(BYTE * work_buffer, int tracklen, int * p_sectorlen);
BYTE * find_sector0(BYTE * work_buffer, int tracklen, int * p_sectorlen);
int extract_GCR_track(BYTE * destination, BYTE * source, int * align,
  int force_align, int cap_min, int cap_max);
int replace_bytes(BYTE * buffer, int length, BYTE srcbyte, BYTE dstbyte);
int check_bad_gcr(BYTE * gcrdata, int length, int fix);
int check_sync_flags(BYTE * gcrdata, int density, int length);
void bitshift(BYTE * gcrdata, int length, int bits);
int check_errors(BYTE * gcrdata, int length, int track, BYTE * id,
  char * errorstring);
int check_empty(BYTE * gcrdata, int length, int track, BYTE * id,
  char * errorstring);
int compare_tracks(BYTE * track1, BYTE * track2, int length1, int length2,
  int same_disk, char * outputstring);
int compare_sectors(BYTE * track1, BYTE * track2, int length1, int length2,
  BYTE * id1, BYTE * id2, int track, char * outputstring);
int strip_runs(BYTE * buffer, int length, int minrun, BYTE target);
int reduce_runs(BYTE * buffer, int length, int length_max, int minrun,
  BYTE target);
int is_bad_gcr(BYTE * gcrdata, int length, int pos);
int check_formatted(BYTE * gcrdata);
int check_valid_data(BYTE * data, int length);
