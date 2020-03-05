/* vtree

   +=======================================+
   | This program is in the public domain. |
   +=======================================+

   This program shows the directory structure of a filesystem or
   part of one.  It also shows the amount of space taken up by files
   in each subdirectory.

   Call via

	vtree fn1 fn2 fn3 ...

   If any of the given filenames is a directory (the usual case),
   vtree will recursively descend into it, and the output line will
   reflect the accumulated totals for all files in the directory.

   This program is based upon "agef" written by David S. Hayes at the
   Army Artificial Intelligence Center at the Pentagon.

   This program is dependent upon the new directory routines written by
   Douglas A. Gwyn at the US Army Ballistic Research Laboratory at the
   Aberdeen Proving Ground in Maryland.
*/
/*
** Patches were received from the following people:
**
**	1.	Mike Howard, (...!uunet!milhow1!how)
**		Mike's patches included changes to the Makefile to
**		customize vtree to SCO Xenix for the 286 as well as the
**		386.  He also added external definitions to hash.c
**
**	2.	Andrew Weeks, (...!uunet!mcvax!doc.ic.ac.uk!aw)
**		Andrew sent me diffs to make vtree work properly under BSD
**		He also pointed out that you will need one of the PD getopt
**		packages for BSD.
**
**	3.	Ralph Chapman, (...uunet!ihnp4!ihuxy!chapman)
**		Ralph sent me changes (not diffs unfortunately) to make
**		vtree work properly under the SYS_III option.  His changes
**		were in direct.c and vtree.c
**
**	4.	David Eckelkamp notified me of a bug when printing the
**		visual tree.  The bug occured when a directory name
**		was too long.  It caused vtree to mess up the tree
**		being printed.
*/

#include "patchlevel.h"
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#ifdef	BSD
#include <strings.h>
#else
#include <string.h>
#endif

#include "customize.h"
#include "hash.h"


#ifdef	SYS_III
	#define	rewinddir(fp)	rewind(fp)
#endif

#define SAME		0	/* for strcmp */
#define BLOCKSIZE	512	/* size of a disk block */

#define K(x)		((x * BLOCKSIZE)/1024)	/* convert stat(2) blocks into
					 * k's.  On my machine, a block
					 * is 512 bytes. */

#define	TRUE	1
#define	FALSE	0
#define	V_CHAR	"|"	/*	Vertical character	*/
#define	H_CHAR	"-"	/*	Horizontal character	*/
#define	A_CHAR	">"	/*	Arrow char		*/
#define	T_CHAR	"+"	/*	Tee char		*/
#define	L_CHAR	"\\"	/*	L char, bottom of a branch	*/

#define	MAX_COL_WIDTH	15
#define	MAX_V_DEPTH	256		/* max depth for visual display */

#ifdef	MEMORY_BASED
struct RD_list {
	READ		entry;
	struct RD_list	*fptr;
	struct RD_list	*bptr;
};
#endif

// Functions that appear later in class, let's define here
static void get_data(char *path, int cont); // declare function prototype (STATIC)
static int chk_4_dir(char *path); // declare function prototype (STATIC)

int		indent = 0,		/* current indent */
		depth = 9999,		/* max depth */
		cur_depth = 0,
		sum = FALSE,		/* sum the subdirectories */
		dup_inodes = FALSE,		/* use duplicate inodes */
		floating = FALSE,	/* floating column widths */
		sort = FALSE,
		cnt_inodes = FALSE,	/* count inodes */
		quick = FALSE,		/* quick display */
		visual = FALSE,		/* visual display */
		version = 0,		/* = 1 display version, = 2 show options */
		sw_follow_links = TRUE,	/* follow symbolic links */
		sub_dirs[MAX_V_DEPTH],
		sub_dirs_indents[MAX_V_DEPTH];

struct	stat	stb;			/* Normally not a good idea, but */
					/* this structure is used through- */
					/* out the program		   */

extern char    *optarg;			/* from getopt(3) */
extern int      optind,
                opterr;


char           *Program;		/* our name */
short           sw_summary;		/* print Grand Total line */

int             total_inodes, inodes;	/* inode count */
long            total_sizes, sizes;	/* block count */

char            topdir[NAMELEN];	/* our starting directory */

/*
** Find the last field of a string.
*/
char *lastfield(char *p, int c) {

/* p = Null-terminated string to scan */
/* c = Separator char, usually '/' */
	char *r;
	r = p;
	while (*p)			/* Find the last field of the name */
		if (*p++ == c)
			r = p;
	return r;
} /* lastfield */

 /*
  * We ran into a subdirectory.  Go down into it, and read everything
  * in there.
  */
int	indented = FALSE;	/* These had to be global since they */
int	last_indent = 0;	/* determine what gets displayed during */
int	last_subdir = FALSE;	/* the visual display */

void down(char *subdir) {

OPEN	*dp;			/* stream from a directory */
OPEN	*opendir ();
char	cwd[NAMELEN], tmp[NAMELEN];
// char	*sptr;
READ	*file;			/* directory entry */
READ	*readdir ();
int	i, x;
// struct	stat stb;

#ifdef	MEMORY_BASED
struct RD_list	*head, *tail, *tmp_RD, *tmp1_RD;		/* head and tail of directory list */
struct RD_list	sz;
READ		tmp_entry;
#endif

	if ( (cur_depth == depth) && (!sum) )
		return;

/* display the tree */

	if (cur_depth < depth) {
		if (visual) {
			if (!indented) {
				for (i = 1; i <cur_depth; i++) {
					if (floating) x = sub_dirs_indents[i] + 1;
						else x = MAX_COL_WIDTH - 3;
					if (sub_dirs[i]) {
						printf("%*s%s   ",x - 1," ",V_CHAR);
					} else printf("%*s   ",x," ");
				}
				if (cur_depth>0) {
					if (floating) x = sub_dirs_indents[cur_depth] + 1;
						else x = MAX_COL_WIDTH - 3;
					if (sub_dirs[cur_depth] == 0) {
						printf("%*s%s%s%s ",x - 1," ",L_CHAR,H_CHAR,A_CHAR);
						last_subdir = cur_depth;
					}
					else printf("%*s%s%s%s ",x - 1," ",T_CHAR,H_CHAR,A_CHAR);
				}
			} else {
				if (!floating)
					for (i = 1; i<MAX_COL_WIDTH-last_indent-3; i++)
						printf("%s",H_CHAR);
				printf("%s%s%s ",T_CHAR,H_CHAR,A_CHAR);
			}

	/* This is in case a subdir name is too big.  It is then displayed on
	** two lines, the first line is the full name, the second line is
	** truncated.  Any subdirs displayed for the current subdir will be
	** appended to the second line.  This keeps the columns in order
	*/

#ifndef	ONEPERLINE
			if (  ( strlen(subdir) > MAX_COL_WIDTH - 3 && !floating )	) {
#else
			if (  ( strlen(subdir) > MAX_COL_WIDTH - 3 && !floating ) ||
			    lastfield(subdir,'/') != subdir) {
#endif

				printf("%s\n",subdir);
				for (i = 1; i <=cur_depth; i++) {
					if (sub_dirs[i]) {
						printf("%*s%s   ",MAX_COL_WIDTH-4," ",V_CHAR);
					}
					else printf("%*s   ",MAX_COL_WIDTH-3," ");
				}
				strcpy(tmp,lastfield(subdir,'/'));
				tmp[MAX_COL_WIDTH - 4] = 0;
				printf("%s",tmp);
#ifdef	ONEPERLINE
				if (floating || strlen(tmp) < MAX_COL_WIDTH - 4) printf(" ");
#endif
				sub_dirs_indents[cur_depth + 1] = last_indent = strlen(tmp) + 1;
			}
			else {
				printf("%s",subdir);
				sub_dirs_indents[cur_depth + 1] = last_indent = strlen(subdir)+1;
				if (floating || strlen(subdir) < MAX_COL_WIDTH - 4)
					printf(" ");
			}
			indented = TRUE;
		}
		else printf("%*s%s",indent," ",subdir);
	}

/* open subdirectory */

	if ((dp = opendir(subdir)) == NULL) {
		printf(" - can't read %s\n", subdir);
		indented = FALSE;
		return;
	}

	cur_depth++;
	indent+=3;

#ifdef BSD
	getwd(cwd);				/* remember where we are */
#else
	getcwd(cwd, sizeof(cwd));		/* remember where we are */
#endif
	chdir(subdir);				/* go into subdir */


#ifdef	MEMORY_BASED

	head = NULL;
	tail = NULL; // inserted
	// Initialized start and end of linked list, now let's go:
	for (file = readdir(dp); file != NULL; file = readdir(dp)) {

		if ((!quick && !visual ) ||
 		    ( strcmp(NAME(*file), "..") != SAME &&
		     strcmp(NAME(*file), ".") != SAME &&
		     chk_4_dir(NAME(*file)) ) ) {

			tmp_RD = malloc(sizeof(struct RD_list));
			memcpy(&tmp_RD->entry, file, sizeof(tmp_RD->entry));

			tmp_RD->bptr = head;
			tmp_RD->fptr = NULL;
			if (head == NULL) head = tmp_RD;
				else tail->fptr = tmp_RD;
			tail = tmp_RD;
		}
	}
				/* screwy, inefficient, bubble sort	*/
				/* but it works			*/

	if (sort) {
		tmp_RD = head;
		while (tmp_RD) {
			// tmp1_RD = tmp_RD->fptr;
			tmp1_RD = tmp_RD;
			while (tmp1_RD) {
				if (strcmp(tmp_RD->entry.d_name, tmp1_RD->entry.d_name) > 0) { // CHANGED TO STRCMP()
					/* swap the two */
						memcpy(&tmp_entry, &tmp_RD->entry, sizeof(tmp_entry));
						memcpy(&tmp_RD->entry, &tmp1_RD->entry, sizeof(tmp_entry));
						memcpy(&tmp1_RD->entry, &tmp_entry, sizeof(tmp_entry));
				}
				tmp1_RD = tmp1_RD->fptr;
			}
			tmp_RD = tmp_RD->fptr;
		}
	}

#endif

	if ( (!quick) && (!visual) ) {

		/* accumulate total sizes and inodes in current directory */


#ifdef	MEMORY_BASED
		tmp_RD = head;
		while (tmp_RD) {
			file = &tmp_RD->entry;
			tmp_RD = tmp_RD->fptr;
#else

		for (file = readdir(dp); file != NULL; file = readdir(dp)) {
#endif
			if (strcmp(NAME(*file), "..") != SAME)
				get_data(NAME(*file),FALSE);
		}

		if (cur_depth<depth) {
			if (cnt_inodes) printf("   %d",inodes);
			printf(" : %ld\n",sizes);
			total_sizes += sizes;
			total_inodes += inodes;
			sizes = 0;
			inodes = 0;
		}
#ifndef	MEMORY_BASED
		rewinddir(dp);
#endif
	} else if (!visual) printf("\n");

	if (visual) {

/* count subdirectories */


#ifdef	MEMORY_BASED
		tmp_RD = head;
		while (tmp_RD) {
			file = &tmp_RD->entry;
			tmp_RD = tmp_RD->fptr;
#else
		for (file = readdir(dp); file != NULL; file = readdir(dp)) {
			if ( (strcmp((NAME(*file)), "..") != SAME) &&
			     (strcmp(NAME(*file), ".") != SAME) ) {
				if (chk_4_dir(NAME(*file))) {
#endif
					sub_dirs[cur_depth]++;
#ifndef	MEMORY_BASED
				}
			}
#endif
		}
#ifndef	MEMORY_BASED
		rewinddir(dp);
#endif
	}

/* go down into the subdirectory */

#ifdef	MEMORY_BASED
	tmp_RD = head;
	while (tmp_RD) {
		file = &tmp_RD->entry;
		tmp_RD = tmp_RD->fptr;
#else
	for (file = readdir(dp); file != NULL; file = readdir(dp)) {
#endif
		if ( (strcmp(NAME(*file), "..") != SAME) &&
		     (strcmp(NAME(*file), ".") != SAME) ) {
			if (chk_4_dir(NAME(*file)))
				sub_dirs[cur_depth]--;
			get_data(NAME(*file),TRUE);
		}
	}

	if ( (!quick) && (!visual) ) {

/* print totals */

		if (cur_depth == depth) {
			if (cnt_inodes) printf("   %d",inodes);
			printf(" : %ld\n",sizes);
			total_sizes += sizes;
			total_inodes += inodes;
			sizes = 0;
			inodes = 0;
		}
	}

#ifdef	MEMORY_BASED
				/* free the allocated memory */
	tmp_RD = head;
	while (tmp_RD) {
		struct RD_list *holder = tmp_RD->fptr;
		free(tmp_RD); // was originally bugged, not properly freeing (was reaccessing it after free)
		tmp_RD = holder;
	}

#endif

	if (visual && indented) {
		printf("\n");
		indented = FALSE;
		if (last_subdir>=cur_depth-1) {
			for (i = 1; i <cur_depth; i++) {
				if (sub_dirs[i]) {
					if (floating)
						printf("%*s%s   ",sub_dirs_indents[i]," ",V_CHAR);
					else printf("%*s%s   ",MAX_COL_WIDTH-4," ",V_CHAR);
				} else {
					if (floating)
/*ZZZ*/						printf("%*s   ",sub_dirs_indents[i] + 1," ");
 					else printf("%*s   ",MAX_COL_WIDTH-3," ");
 				}
			}
			printf("\n");
			last_subdir = FALSE;
		}
	}
	indent-=3;
	sub_dirs[cur_depth] = 0;
	cur_depth--;

	chdir(cwd);			/* go back where we were */

	closedir(dp);
} /* down */

/* Is the specified path a directory ? */

int	is_directory(char *path) {

#ifdef LSTAT
	if (sw_follow_links) {
		stat(path, &stb);	/* follows symbolic links */
	}
	else {
		lstat(path, &stb);	/* doesn't follow symbolic links */
	}
#else
	stat(path, &stb);
#endif

	if ((stb.st_mode & S_IFMT) == S_IFDIR)
		return TRUE;
	else return FALSE;
} /* is_directory */

int	chk_4_dir(char *path) {

	if (is_directory(path)) return TRUE;
	else return FALSE;

} /* chk_4_dir */

void clear_hash() {

	// Free keys of bucketp
	int limit;
	for (limit = 0; limit < 200; limit++) {
		if (ino_holder != NULL) free(ino_holder[limit]);
	}

	// Free the memory of hash table
	int limit2;
	for (limit2 = 0; limit2 < 200; limit2++) {
		if (htable_holder != NULL) free(htable_holder[limit2]);
	}

} // end of clear_hash()

 /*
  * Get the aged data on a file whose name is given.  If the file is a
  * directory, go down into it, and get the data from all files inside.
  */

void get_data(char *path, int cont) {
/* struct	stat	stb; */
	// int i

	if (cont) {

		if (is_directory(path)) {
			inodes = inodes + 1; // takes into account directories themselves (1)
			sizes+= 4; // chage to number of blocks actually used.
			down(path);
		}
	}
	else {

		if (is_directory(path)) {
			// inodes += 1; // takes into account directories themselves (2)
			return;
		}

		    /* Don't do it again if we've already done it once. */

		if ( (h_enter(stb.st_dev, stb.st_ino) == OLD) && (!dup_inodes) )
			return;
		inodes++;
		sizes+= K(stb.st_blocks); // chage to number of blocks actually used.
	}
} /* get_data */
int vtree_main(int argc, char *argv[]) {
	init_table();
	int	i, err = FALSE;
	int	option;
	int	user_file_list_supplied = 0;
	Program = *argv;		/* save our name for error messages */


	// Let's set up for getopt_long
	static struct option case_options[] = {
		{"duplicates", no_argument, 0, 'd'},
		{"floating-column-widths", no_argument, 0, 'f'},
		{"height", no_argument, 0, 'h'},
		{"inodes", required_argument, 0, 'i'},
		{"sort-directories", no_argument, 0, 'o'},
		{"totals", no_argument, 0, 't'},
		{"quick-display", no_argument, 0, 'q'},
		{"visual-display", no_argument, NULL, 'v'},
		{"version", no_argument, 0, 'V'},
		{"no-follow-symlinks", no_argument, 0, 'l'}, // new option 'l'
		{ 0, 0, 0, 0,}
	};

    /* Pick up options from command line */

	while ((option = getopt_long(argc, argv, "dfh:iostlqvV", case_options, NULL)) != EOF) {
		switch (option) {
			case 'f':	floating = TRUE; break;
			case 'h':	depth = atoi(optarg);
					while (*optarg) {
						if (!isdigit(*optarg)) {
							err = TRUE;
							break;
						}
						optarg++;
					}
					break;
			// -------------------------------------------------------
			#ifdef LSTAT
				case 'l':   sw_follow_links = FALSE; // inserted
					break;
			#endif
			// -------------------------------------------------------

			case 'd':	dup_inodes = TRUE;
					break;
			case 'i':	cnt_inodes = TRUE;
					break;

			// -------------------------------------------------------
			#ifdef MEMORY_BASED
				case 'o':	sort = TRUE;
					break;
			#endif
			// -------------------------------------------------------
			case 's':	sum = TRUE;
					break;
			case 't':	sw_summary = TRUE;
					break;
			case 'q':	quick = TRUE;
					dup_inodes = FALSE;
					sum = FALSE;
					cnt_inodes = FALSE;
					break;
			case 'v':	visual = TRUE;
					break;
			case 'V':	version++;
					break;
			default:	err = TRUE;
		}
		if (err) {
			fprintf(stderr,"%s: [ -d ] [ -h # ] [ -i ] [ -o ] [ -s ] [ -q ] [ -v ] [ -V ]\n",Program);
			fprintf(stderr,"	-d	count duplicate inodes\n");
			fprintf(stderr,"	-f	floating column widths\n");
			#ifdef LSTAT
				fprintf(stderr,"	-l	do not follow symbolic links\n");
			#endif
			fprintf(stderr,"	-h #	height of tree to look at\n");
			fprintf(stderr,"	-i	count inodes\n");
			#ifdef MEMORY_BASED
				fprintf(stderr,"	-o	sort directories before processing\n");
			#endif
			fprintf(stderr,"	-s	include subdirectories not shown due to -h option\n");
			fprintf(stderr,"	-t	totals at the end\n");
			fprintf(stderr,"	-q	quick display, no counts\n");
			fprintf(stderr,"	-v	visual display\n");
			fprintf(stderr,"	-V	show current version\n");
			fprintf(stderr,"		(2 Vs shows specified options)\n");
			exit(-1);
		}

	}

	if (version > 0 ) {

#ifdef	MEMORY_BASED
		printf("%s memory based\n",VERSION);
#else
		printf("%s disk based\n",VERSION);
#endif

		if (version>1) {
			printf("Tree height:	%d\n",depth);
			if (dup_inodes) printf("Include duplicate inodes\n");
			if (cnt_inodes) printf("Count inodes\n");
			if (sum) printf("Include unseen subdirectories in totals\n");
			if (sw_summary) printf("Print totals at end\n");
			if (quick) printf("Quick display only\n");
			if (visual) printf("Visual tree\n");
			if (sort) printf("Sort directories before processing\n");
		}
	}

    /* If user didn't specify targets, inspect current directory. */

	if (optind >= argc) {
		user_file_list_supplied = 0;
	} else {
		user_file_list_supplied = 1;
	}

#ifdef BSD
	getwd(topdir);				/* find out where we are */
#else
	getcwd(topdir, sizeof (topdir));	/* find out where we are */
#endif

    /* Zero out grand totals */
	total_inodes = total_sizes = 0;
    /* Zero out sub_dirs */
	for (i=0; i<=MAX_V_DEPTH; i++) {
		sub_dirs[i] = 0;
		sub_dirs_indents[i] = 0;
	}


    /* Inspect each argument */
	for (i = optind; i < argc || (!user_file_list_supplied && i == argc); i++) {
		cur_depth = inodes = sizes = 0;
		chdir(topdir);		/* be sure to start from the same place */
		get_data(user_file_list_supplied?argv[i] : topdir, TRUE);/* this may change our cwd */


		total_inodes += inodes;
		total_sizes += sizes;
	}

	if (sw_summary) {
		printf("\n\nTotal space used: %ld\n",total_sizes);
		if (cnt_inodes) printf("Total inodes: %d\n",inodes);
	}

#ifdef HSTATS
	fflush(stdout);
	h_stats();
#endif
	clear_hash();
	exit(0);
} /* main */