/*
 * MNIB
 * Copyright 2001-2005 Markus Brenner <markus@brenner.de>
 * and Pete Rittwage <peter@rittwage.com>
 */

#define FL_STEPTO      0x00
#define FL_MOTOR       0x01
#define FL_RESET       0x02
#define FL_READNORMAL  0x03
#define FL_WRITESYNC   0x04
#define FL_DENSITY     0x05
#define FL_SCANKILLER  0x06
#define FL_SCANDENSITY 0x07
#define FL_READWOSYNC  0x08
#define FL_READMOTOR   0x09
#define FL_TEST        0x0a
#define FL_WRITENOSYNC 0x0b
#define FL_CAPACITY    0x0c
#define FL_INITTRACK   0x0d
#define FL_FINDSYNC    0x0e
#define FL_READMARKER  0x0f
#define FL_VERIFY_CODE 0x10

#define DISK_NORMAL    0

#define IMAGE_NIB      0	/* destination image format */
#define IMAGE_D64      1
#define IMAGE_G64      2

#define BM_MATCH       0x10
#define BM_NO_SYNC     0x40
#define BM_FF_TRACK    0x80

// tested extensively with Pete's Newtronics mech-based 1541.
//#define CAPACITY_MARGIN 11	// works with FL_WRITENOSYNC
//#define CAPACITY_MARGIN 13	// works with both FL_WRITESYNC and
								// FL_WRITENOSYNC
#define CAPACITY_MARGIN 16		// safe value

#define MODE_READ_DISK     	0
#define MODE_WRITE_DISK    	1
#define MODE_UNFORMAT_DISK 	2
#define MODE_WRITE_RAW	   	3
#define MODE_TEST_ALIGNMENT 4

#ifndef DJGPP
# include <opencbm.h>
#endif

/* global variables */
extern char bitrate_range[4];
extern char bitrate_value[4];
extern char density_branch[4];
extern int mode;
extern FILE * fplog;
extern int read_killer;
extern unsigned int error_retries;
extern int align;
extern int force_align;
extern BYTE start_track, end_track, track_inc;
extern int fix_gcr, reduce_syncs, reduce_gaps, reduce_weak, verify;
extern int imagetype, auto_density_adjust;

/* function prototypes */

/* read.c */
BYTE read_halftrack(CBM_FILE fd, int halftrack, BYTE * buffer);
int read_d64(CBM_FILE fd, FILE * fpout);
void read_nib(CBM_FILE fd, FILE * fpout, char * track_header);

/* write.c */
void write_raw(CBM_FILE fd);
void unformat_disk(CBM_FILE fd);
void parse_disk(CBM_FILE fd, FILE * fpin, char * track_header);
int write_d64(CBM_FILE fd, FILE * fpin);

/* mnib.c  */
int test_par_port(CBM_FILE fd);
void send_mnib_cmd(CBM_FILE fd, unsigned char cmd);
void set_full_track(CBM_FILE fd);
void motor_on(CBM_FILE fd);
void motor_off(CBM_FILE fd);
void step_to_halftrack(CBM_FILE fd, int halftrack);
unsigned int track_capacity(CBM_FILE fd);
void reset_floppy(CBM_FILE fd, BYTE drive);
int set_density(CBM_FILE fd, int density);
int set_bitrate(CBM_FILE fd, int density);
BYTE set_default_bitrate(CBM_FILE fd, int track);
BYTE scan_track(CBM_FILE fd, int track);
BYTE scan_density(CBM_FILE fd);
void adjust_target(CBM_FILE fd);

char char_fgetc(FILE *stream);
