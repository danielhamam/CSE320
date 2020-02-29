/* direct.c

   SCCS ID	@(#)direct.c	1.6	7/9/87

 *
 *	My own substitution for the berkeley reading routines,
 *	for use on System III machines that don't have any other
 *	alternative.
 */

#include "customize.h" // for h file
#include <sys/stat.h> // for ino_t type

#define NAMELENGTH	14
#ifdef	SYS_III
	FILE	*opendir(name)	{ return (fopen(name,"r") ); }
#else
	#define opendir(name)	fopen(name, "r")
#endif
#define closedir(fp)	fclose(fp)

struct dir_entry {		/* What the system uses internally. */
    ino_t d_ino; // figure out what library ino_t
    char d_name[NAMELENGTH];
};

struct direct {			/* What these routines return. */
    ino_t           d_ino;
    char            d_name[NAMELENGTH];
    char            terminator;
};

#ifdef SYS_III

 /*
  * Read a directory, returning the next (non-empty) slot.
  */

READ *readdir(OPEN *dp) {

    static READ direct;

    /* This read depends on direct being similar to dir_entry. */

    while (fread(&direct, sizeof(struct dir_entry), 1, dp) != 0) {
	direct.terminator = '\0';
	if (INO(direct) != 0)
	    return &direct;
    };

    return (READ *) NULL;
}

#endif
