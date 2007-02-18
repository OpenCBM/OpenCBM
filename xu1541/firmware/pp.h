/*
  pp.h - xu1541 parallel driver interface
*/

#ifndef PP_H
#define PP_H

#define PP_VERSION_MAJOR 1
#define PP_VERSION_MINOR 0

extern unsigned char pp_read(unsigned char *data, unsigned char len);
extern unsigned char pp_write(unsigned char *data, unsigned char len);

#endif // PP_H
