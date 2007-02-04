/*
  s2.h - xu1541 serial1 driver interface
*/

#ifndef S2_H
#define S2_H

#define S2_VERSION_MAJOR 1
#define S2_VERSION_MINOR 0

extern unsigned char s2_read(unsigned char *data, unsigned char len);
extern unsigned char s2_write(unsigned char *data, unsigned char len);

#endif // S2_H
