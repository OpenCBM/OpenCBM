
int cbm_nib_write(char data, int toggle);
int cbm_mnib_write_track(int fd, unsigned char * buffer, int length, int mode);
int cbm_nib_read(int toggle);
int cbm_mnib_read_track(int fd, unsigned char * buffer, unsigned int length, int mode);

