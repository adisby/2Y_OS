/*
 * =====================================================================================
 *
 *       Filename:  seekdir.c
 *
 *    Description:  set the position of the next readdir() call in the directory
 *
 *        Version:  1.0
 *        Created:  2012年05月13日 21时58分07秒
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
 *         Name:  seekdir
 *  Description:  set the position of the next readdir() call in the directory
 *  @param __dirp:the DIR pointer
 *  @param __pos: the position should be change to
 * =====================================================================================
 */
PUBLIC	int
seekdir (DIR * __dirp, off_t __pos)
{
	if (NULL == __dirp) {
		printf("%s %d null __dirp pointer\n", __FILE__, __LINE__);
		return -1;
	}

	__dirp->_count = 0;

	if (-1 == lseek(__dirp->_fd, __pos, SEEK_SET)) {
		printf("%s %d lseek failed\n", __FILE__, __LINE__);
		return -1;
	}

	return 0;
}		/* -----  end of function seekdir  ----- */
