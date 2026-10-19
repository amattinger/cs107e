// Functions not included in the CS107E ff library

#include "ff.h" // types for files

typedef struct {
	FIL* fp;
	int idx, nchr;
	BYTE buf[64];
} putbuff;

int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */
