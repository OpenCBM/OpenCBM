#ifndef CBM_LIBMISC_H
#define CBM_LIBMISC_H

extern char * cbmlibmisc_strdup(const char * const OldString);
extern char * cbmlibmisc_strndup(const char * const OldString, size_t Length);

extern void   cbmlibmisc_strfree(char * String);
extern char * cbmlibmisc_strcat(char * first, const char * second);

#endif /* #ifndef CBM_LIBMISC_H */
