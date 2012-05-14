/*
 * =====================================================================================
 *
 *       Filename:  getcwd.c
 *
 *    Description:  get the name of he current working directory
 *
 *        Version:  1.0
 *        Created:  2012年05月13日 20时56分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */


#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "mm.h"
#include "dirent.h"
#include "unistd.h"
#include "stdlib.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  addpath
 *  Description:  Add the name of a directory entry at the 
 *				  front of the path being built.Note that 
 *				  the result always starts with a slash.
 *	@param path:  the path should be add to
 *	@param ap:	  the current pointer in path
 *	@param entry: the entry whose name should be added to the path
 * =====================================================================================
 */
PRIVATE int
addpath (const char *path, char **ap, const char *entry)
{
	const char *e= entry;
	char *p= *ap;

	while (*e != 0) e++;

	while (e > entry && p > path) *--p = *--e;

	if (p == path) return -1;
	*--p = '/';
	*ap= p;
	return 0;
}		/* -----  end of function addpath  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  recover
 *  Description:  Undo all those chdir("..")'s that have been recorded 
 *				  by addpath.  This has to be done entry by entry,
 *				  because the whole pathname may be too long.
 *	@param p:	  the path to recover to 
 * =====================================================================================
 */
PRIVATE int
recover ( char * p )
{
	int slash;
	char *p0;

	while (*p != 0) {
		p0= ++p;

		do p++; while (*p != 0 && *p != '/');
		slash= *p; *p= 0;

		if (chdir(p0) != 0) return -1;
		*p= slash;
	}
	return 0;
}		/* -----  end of function recover  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getcwd
 *  Description:  get the name of he current working directory
 *  @param path:  the path of the current working dircotory
 *  @param size:  the size of the path
 * =====================================================================================
 */
PUBLIC	char *
getcwd ( char *path, size_t size)
{
	printf("%s %d getcwd started\n", __FILE__, __LINE__);
	struct stat above, current, tmp;
	struct dirent *entry;
	DIR *d;
	char *p, *up, *dotdot;
	int cycle;

	if (NULL == path || size <= 1) {
		printf("%s %d null path pointer\n", __FILE__, __LINE__);
		return NULL;
	}

	p = path + size;
	*--p = '\0';

	if (stat(".", &current) < 0) {
		printf("%s %d stat current failed\n", __FILE__, __LINE__);
		return NULL;
	}

	while (1) {
		dotdot = "..";
		if (stat(dotdot, &above) < 0) {
			printf("%s %d stat dotdot failed\n", __FILE__, __LINE__);
			recover(p);
			return NULL;
		}

		if (above.st_dev == current.st_dev
				&& above.st_ino == current.st_ino) {
			//found the ROOT_INODE
			printf("%s %d found the root inode : d_ino %d\n", __FILE__, __LINE__, current.st_ino);
			break;
		}

		if (NULL == (d = opendir(dotdot)) ) {
			printf("%s %d open dotdot failed\n", __FILE__, __LINE__);
			recover(p);
			return NULL;
		}

		/*  Cycle is 0 for a simple inode nr search, or 1 for a search
		 *  for inode *and* device nr.
		*/
		cycle= above.st_dev == current.st_dev ? 0 : 1;

		do {
			char name[3 + MAX_FILENAME_LEN + 1];

			tmp.st_ino = 0;
			if (NULL == (entry = readdir(d))) {
				switch (++cycle) {
					case 1:
						rewinddir(d);
						continue;
					case 2:
						printf("%s %d read dir failed\n", __FILE__, __LINE__);
						closedir(d);
						recover(p);
						return NULL;
				}
			}

			if (0 == strcmp(entry->d_name, ".")) continue;
			if (0 == strcmp(entry->d_name, "..")) continue;

			switch (cycle) {
				case 0:
					if (entry->d_ino != current.st_ino) continue;
				case 1:
					strcpy(name, "../");
					strcpy(name + 3, entry->d_name);
					if (stat(name, &tmp) < 0)
						continue;
					break;
			}
		} while (tmp.st_ino != current.st_ino
				|| tmp.st_dev != current.st_dev);

		up = p;
		if (addpath(path, &up, entry->d_name) < 0) {
			printf("%s %d add path failed\n", __FILE__, __LINE__);
			closedir(d);
			recover(p);
			return NULL;
		}
		closedir(d);

		if (chdir(dotdot) != 0) {
			printf("%s %d chdir dotdot failed\n", __FILE__, __LINE__);
			recover(p);
			return NULL;
		}

		p = up;

		current = above;
	}
	if (recover(p) != 0) {
		printf("%s %d fail to undo all the chdir dotdot\n", __FILE__, __LINE__);
		return NULL;
	}
	if (*p == 0) {
		*--p = '/';
	}
	if (p > path) {
		//move string to the start of path
		strcpy(path, p);
	}
	return path;
}		/* -----  end of function getcwd  ----- */
