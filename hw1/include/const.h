/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef CONST_H
#define CONST_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <utime.h>
#include <dirent.h>
#include <sys/stat.h>

#include "sequitur.h"

#define USAGE(program_name, retcode) do { \
fprintf(stderr, "USAGE: %s %s\n", program_name, \
"[-h] -c|-d [-b]\n" \
"   -h       Help: displays this help menu.\n" \
"   -c       Compress: read bytes from standard input, output compressed data to standard output.\n" \
"   -d       Decompress: read compressed data from standard input, output raw data to standard output.\n" \
"            Optional additional parameter for -c (not permitted with -d):\n" \
"               -b           BLOCKSIZE is the blocksize (in Kbytes, range [1, 1024])\n" \
"                            to be used in compression.\n"); \
exit(retcode); \
} while(0)

/*
 * The following global variables have been provided for you.
 * You MUST use them for their stated purposes, because you are not permitted
 * to declare any arrays (or use any array brackets at all) in your own code.
 * Also, some of the tests we make on your program may rely on being able to
 * inspect the contents of these variables.
 */

/* Options info, set by validargs. */
int global_options;

/* Statically allocated storage for symbols. */
SYMBOL symbol_storage[MAX_SYMBOLS];

/* Total number of symbols allocated from symbol_storage. */
int num_symbols;

/* Storage for the digram hash table, which maps pairs of symbol values to digrams. */
SYMBOL *digram_table[MAX_DIGRAMS];

/*
 * The "main rule", which heads the list of rules generated by the compression algorithm
 * (during compression) or read in as input (during decompression).
 */
SYMBOL *main_rule;

/*
 * Array, used during decompression, that maps symbol values to nonterminal symbols.
 */
SYMBOL *rule_map[SYMBOL_VALUE_MAX];
/*
 * Below this line are prototypes for functions that MUST occur in your program.
 * Non-functioning stubs for all these functions have been provided in the various source
 * files, where detailed specifications for the required behavior of these functions have
 * been given.
 *
 * Your implementations of these functions MUST have exactly the specified interfaces and
 * behave exactly according to the specifications, because we may choose to test any or all
 * of them independently of your main program.
 */

int validargs(int argc, char **argv);

int decompress(FILE *in, FILE *out);
int compress(FILE *in, FILE *out, int bsize);

void init_symbols(void);
SYMBOL *new_symbol(int value, SYMBOL *rule);
void recycle_symbol(SYMBOL *s);

void init_rules(void);
SYMBOL *new_rule(int v);
void add_rule(SYMBOL *rule);
void delete_rule(SYMBOL *rule);
SYMBOL *ref_rule(SYMBOL *rule);
void unref_rule(SYMBOL *rule);

void init_digram_hash(void);
SYMBOL *digram_get(int v1, int v2);
int digram_delete(SYMBOL *first);
int digram_put(SYMBOL *first);

#endif