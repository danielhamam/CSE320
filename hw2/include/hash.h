/* Defines for the agef hashing functions.

   SCCS ID	@(#)hash.h	1.6	7/9/87
 */

#define BUCKETS		257	/* buckets per hash table */
#define TABLES		50	/* hash tables */
#define EXTEND		100	/* how much space to add to a bucket */

int h_enter(dev_t dev, ino_t ino); // located in hash.c @ 45 and vtree.c @ 467
void init_table();

// Arrays to hold the pointers from hash_table and bucketp.
struct htable *htable_holder[200];
ino_t *ino_holder[200];

// Hash table accessible by multiple .c files now
static struct htable *tables[TABLES];

struct hbucket {
    int             length;	/* key space allocated */
    int             filled;	/* key space used */
    ino_t          *keys;
};

struct htable {
    dev_t           device;	/* device this table is for */
    struct hbucket  buckets[BUCKETS];	/* the buckets of the table */
};

#define OLD	0		/* inode was in hash already */
#define NEW	1		/* inode has been added to hash */
