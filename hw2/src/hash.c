/* hash.c

   SCCS ID	@(#)hash.c	1.6	7/9/87

 * Hash table routines for AGEF.  These routines keep the program from
 * counting the same inode twice.  This can happen in the case of a
 * file with multiple links, as in a news article posted to several
 * groups.  The use of a hashing scheme was suggested by Anders
 * Andersson of Uppsala University, Sweden.  (enea!kuling!andersa)
 */

/* hash.c change history:
 28 March 1987		David S. Hayes (merlin@hqda-ai.UUCP)
	Initial version.
*/

#include <stdio.h>
#include <sys/types.h>
#include "hash.h"
#include <string.h>
#include <stdlib.h>

static struct htable *tables[TABLES];
// extern char *malloc();		/* added 6/17/88 */
// extern char *realloc();		/* added 6/17/88 */
// extern char *calloc();		/* added 6/17/88 */

/* These are for statistical use later on. */
static int      hs_tables = 0,	/* number of tables allocated */
                hs_duplicates = 0,	/* number of OLD's returned */
                hs_buckets = 0,	/* number of buckets allocated */
                hs_extensions = 0,	/* number of bucket extensions */
                hs_searches = 0,/* number of searches */
                hs_compares = 0,/* total key comparisons */
                hs_longsearch = 0;	/* longest search */


void init_table() {

  // For bucketp array:
  int highest;
  for (highest = 0; highest < 200; highest++) {
    ino_holder[highest] = NULL;
  }

  // For hash table array:
  int highest2;
  for (highest2 = 0; highest2 < 200; highest2++) {
    htable_holder[highest] = NULL;
  }

}

 /*
  * This routine takes in a device/inode, and tells whether it's been
  * entered in the table before.  If it hasn't, then the inode is added
  * to the table.  A separate table is maintained for each major device
  * number, so separate file systems each have their own table.
  */

int h_enter(dev_t dev, ino_t ino) {
    static struct htable *tablep = (struct htable *) 0;
    register struct hbucket *bucketp;
    register ino_t *keyp;
    int             i;

    hs_searches++;		/* stat, total number of calls */

    /*
     * Find the hash table for this device. We keep the table pointer
     * around between calls to h_enter, so that we don't have to locate
     * the correct hash table every time we're called.  I don't expect
     * to jump from device to device very often.
     */
    if (!tablep || tablep->device != dev) {
	for (i = 0; tables[i] && tables[i]->device != dev;)
	    i++;
	if (!tables[i]) {
	    tables[i] = (struct htable *)  malloc(sizeof(struct htable));

// -----------------------------------------------------
      // Add to holder array:
      int free_index1 = 0;
      while (1) {
        if (htable_holder[free_index1] == NULL) {
          htable_holder[free_index1] = tables[i];
          break; // use this free_index
        }
        free_index1++;
      } // end of while loop
// -----------------------------------------------------

	    if (tables[i] == NULL) {
		perror("can't malloc hash table");
		return NEW; // NEW = 0, defined in hash.h
	    };
#ifdef BSDi
	    bzero(tables[i], sizeof(struct htable));
#else
	    memset((char *) tables[i], '\0', sizeof (struct htable));
#endif
	    tables[i]->device = dev;
	    hs_tables++;	/* stat, new table allocated */
	};
	tablep = tables[i];
    };

    /* Which bucket is this inode assigned to? */
    bucketp = &tablep->buckets[ino % BUCKETS];

    /*
     * Now check the key list for that bucket.  Just a simple linear
     * search.
     */
    keyp = bucketp->keys;
    for (i = 0; i < bucketp->filled && *keyp != ino;)
	i++, keyp++;

    hs_compares += i + 1;	/* stat, total key comparisons */

    if (i && *keyp == ino) {
	hs_duplicates++;	/* stat, duplicate inodes */
	return OLD;
    };

    /* Longest search.  Only new entries could be the longest. */
    if (bucketp->filled >= hs_longsearch)
	hs_longsearch = bucketp->filled + 1;

    /* Make room at the end of the bucket's key list. */
    if (bucketp->filled == bucketp->length) {
	/* No room, extend the key list. */
	if (!bucketp->length) {
	    bucketp->keys = (ino_t *) calloc(EXTEND, sizeof(ino_t));

// -----------------------------------------------------
      // Add to holder array:
      int free_index = 0;
      while (1) {
        if (ino_holder[free_index] == NULL) {
          ino_holder[free_index] = bucketp->keys;
          break; // use this free_index
        }
        free_index++;
      } // end of while loop
// -----------------------------------------------------

	    if (bucketp->keys == NULL) {
		perror("can't malloc hash bucket");
		return NEW;
	    };
	    hs_buckets++;
	} else {
	    bucketp->keys = (ino_t *)
		realloc(bucketp->keys,
			(EXTEND + bucketp->length) * sizeof(ino_t));
	    if (bucketp->keys == NULL) {
		perror("can't extend hash bucket");
		return NEW;
	    };
	    hs_extensions++;
	};
	bucketp->length += EXTEND;
    };

    bucketp->keys[++(bucketp->filled)] = ino;
    return NEW;
}

 /* Buffer statistics functions.  Print 'em out. */

#ifdef HSTATS
void h_stats()
{
    fprintf(stderr, "\nHash table management statistics:\n");
    fprintf(stderr, "  Tables allocated: %d\n", hs_tables);
    fprintf(stderr, "  Buckets used: %d\n", hs_buckets);
    fprintf(stderr, "  Bucket extensions: %d\n\n", hs_extensions);
    fprintf(stderr, "  Total searches: %d\n", hs_searches);
    fprintf(stderr, "  Duplicate keys found: %d\n", hs_duplicates);
    if (hs_searches)
	fprintf(stderr, "  Average key search: %d\n",
		hs_compares / hs_searches);
    fprintf(stderr, "  Longest key search: %d\n", hs_longsearch);
    fflush(stderr);
}

#endif