/*
 * =====================================================================================
 *
 *       Filename:  brk.c
 *
 *    Description:  call the kernel for brk
 *
 *        Version:  1.0
 *        Created:  2012年05月12日 09时35分52秒
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

void* _brksize;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  brk
 *  Description:  perform the brk system call
 *  @param addr:  the address that the segment size want to change
 *  @return:	  return 0 if success
 * =====================================================================================
 */
PUBLIC	int
brk (void* addr)
{
	if (_brksize == addr && 0 != _brksize) {
		//no need to change 
		printf("%s %d no need to change the brk\n", __FILE__, __LINE__);
		return 0;
	}

	MESSAGE	msg;
	msg.type		= BRK;
	msg.BRK_ADDR	= addr;

	send_recv(BOTH, TASK_MM, &msg);
	printf("%s %d msg.BRK_ADDR %d\n", __FILE__, __LINE__, msg.BRK_ADDR);
	_brksize = msg.BRK_ADDR;
	printf("%s %d _brksize %d\n", __FILE__, __LINE__, _brksize);
	 
	return msg.RETVAL;
}		/* -----  end of function brk  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sbrk
 *  Description:  change the data segment by increment
 *  @param incr:  the change between the old brk and the new brk
 * =====================================================================================
 */
PUBLIC	void	
sbrk (int incr)
{
	char * newsize, *oldsize;
	oldsize = _brksize;
	newsize = _brksize + incr;
	if ((incr > 0 && newsize < oldsize) ||
			(incr < 0 && newsize > oldsize)) {
		return;
	}
	brk(newsize);
}		/* -----  end of function sbrk  ----- */
