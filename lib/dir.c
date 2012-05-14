/*
 * =====================================================================================
 *
	struct stat st;
 *       Filename:  dir.c
 *
 *    Description:  the directory operation
 *
 *        Version:  1.0
 *        Created:  2012年01月09日 01时27分24秒
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
 *         Name		:	opendir
 *  Description		:	open a directory
 *  @param __d_name :	the dir name
 *  @return			:	the directory stream
 * =====================================================================================
 */
PUBLIC	DIR *
opendir (const char * __d_name)
{
	int fd;
	DIR *dp;
	struct stat	st;

	printl("{lib} Begin to open directory!\n");

	if (-1 == stat(__d_name, &st)) {
		return NULL;
	}

	/* Directory only */
	if (I_DIRECTORY != st.st_mode) {
		return NULL;
	}

	printl("{lib} Stat OK\n");
	/* open the directory */
	if (-1 == (fd = open(__d_name, O_RDWR)) ) {
		return NULL;
	}

	printl("{lib} File descriptor opened\n");

	//add after the mm module is done
	dp = (DIR *)malloc(sizeof(DIR) );
	if (NULL == dp) {
		printl("{lib}Memory allocate failed\n");
		return NULL;
	}

	printl("{lib} Memory allocated\n");

	dp->_fd = fd;
//	printl("dp adress : %d\n", dp);
	dp->_count = 0;
	printl("dp _count: %d\n", dp->_count);
	dp->_pos = 0;
	printl("dp _pos: %d\n", dp->_pos);

	return dp;
}		/* -----  end of function opendir  ----- */
