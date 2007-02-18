/*
  p2.h - xu1541 parallel driver interface
*/

#ifndef P2_H
#define P2_H

#define P2_VERSION_MAJOR 1
#define P2_VERSION_MINOR 0

extern unsigned char p2_read(unsigned char *data, unsigned char len);
extern unsigned char p2_write(unsigned char *data, unsigned char len);

#endif // P2_H
