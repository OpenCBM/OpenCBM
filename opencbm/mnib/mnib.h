#ifndef __MNIB_H__
#define __MNIB_H__

#define FD 1                /* (unused) file number for cbm_routines */

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

#define IMAGE_NIB      0    /* destination image format */
#define IMAGE_D64      1
#define IMAGE_G64      2

#define BM_MATCH       0x10
#define BM_NO_SYNC     0x40
#define BM_FF_TRACK    0x80

// tested extensively with Pete's Newtronics mech-based 1541.
//#define CAPACITY_MARGIN 11	// works with FL_WRITENOSYNC
//#define CAPACITY_MARGIN 13	// works with both FL_WRITESYNC and FL_WRITENOSYNC
#define CAPACITY_MARGIN 16	// safe value


#define MODE_READ_DISK     	0
#define MODE_WRITE_DISK    	1
#define MODE_UNFORMAT_DISK 	2
#define MODE_WRITE_RAW	   	3
#define MODE_TEST_ALIGNMENT 4

int start_track;
int end_track;
int track_inc;
int reduce_syncs;
int reduce_weak;
int reduce_gaps;
int fix_gcr;
int aggressive_gcr;
int current_track;
int align;
int force_align;
int read_killer;
int error_retries;
unsigned int lpt[4];
int lpt_num;
int drivetype;
unsigned int floppybytes;
int disktype;
int imagetype;
int mode;

FILE *fplog;
BYTE diskbuf[84 * 0x2000];
int track_length[84];
int track_density[84];
char diskid[3];

/* prototypes */

/* read.c */
int read_halftrack(CBM_FILE fd, int halftrack, BYTE *buffer);
int paranoia_read_halftrack(CBM_FILE fd, int halftrack, BYTE *buffer);
int read_d64(CBM_FILE fd, FILE *fpout);
void readdisk(CBM_FILE fd, FILE *fpout, char *track_header);

/* write.c */
void write_halftrack(int halftrack, int density, unsigned int length, BYTE *gcrdata);
void master_disk(CBM_FILE fd);
void write_raw(CBM_FILE fd);
void unformat_disk(CBM_FILE fd);
void unformat_track(CBM_FILE fd, int track);
void parse_disk(CBM_FILE fd, FILE *fpin, char *track_header);
void write_d64(CBM_FILE fd, FILE *fpin);

/* mnib.c  */
int compare_extension(char *filename, char *extension);
void upload_code(CBM_FILE fd);
int test_par_port(CBM_FILE fd);
int verify_floppy(CBM_FILE fd);
int find_par_port(CBM_FILE fd);
void set_full_track(CBM_FILE fd);
void motor_on(CBM_FILE fd);
void motor_off(CBM_FILE fd);
void step_to_halftrack(CBM_FILE fd, int halftrack);
unsigned int track_capacity(CBM_FILE fd);
void reset_floppy(CBM_FILE fd);
int set_bitrate(CBM_FILE fd, int density);
int set_default_bitrate(CBM_FILE fd, int track);
int scan_track(CBM_FILE fd, int track);
int scan_density(CBM_FILE fd);
void adjust_target(CBM_FILE fd);
void file2disk(CBM_FILE fd, char *filename);
void disk2file(CBM_FILE fd, char *filename);
void usage(void);

/* read.c */
void read_nib(CBM_FILE fd, FILE *fpout, char *track_header);

/*linux send_par_cmd is needed here, because we haven't it in the kernel */
void send_par_cmd(CBM_FILE fd, BYTE cmd);

#endif
