/*
  s1.h - xu1541 serial1 driver interface
*/

#ifndef S1_H
#define S1_H

#define S1_VERSION_MAJOR 1
#define S1_VERSION_MINOR 0

extern unsigned char s1_read(unsigned char *data, unsigned char len);
extern unsigned char s1_write(unsigned char *data, unsigned char len);

#endif // S1_H
