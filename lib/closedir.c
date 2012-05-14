/*
 * =====================================================================================
 *
 *       Filename:  closedir.c
 *
 *    Description:  close a directory
 *
 *        Version:  1.0
 *        Created:  2012年05月13日 15时19分16秒
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
 *         Name:  closedir
 *  Description:  close a directory
 *  @param dp:	  the directory pointer
 * =====================================================================================
 */
PUBLIC	int
closedir (DIR *dp)
{
	int d;

	if (NULL == dp) {
		return -1;
	}
	d = dp->_fd;
	free((void *) dp);
	return close(d);
}		/* -----  end of function closedir  ----- */
