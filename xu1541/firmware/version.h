#ifndef VERSION_H
#define VERSION_H

#define XU1541_VERSION_MAJOR_BASE   1
#define XU1541_VERSION_MINOR        6

#ifdef USBTINY
/* usbtiny version has even major number */ 
#define XU1541_VERSION_MAJOR  (XU1541_VERSION_MAJOR_BASE + 1)
#else
/* avrusb has the odd number */
#define XU1541_VERSION_MAJOR  (XU1541_VERSION_MAJOR_BASE)
#endif

#endif /* #ifndef VERSION_H */