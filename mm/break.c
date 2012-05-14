/*
 * =====================================================================================
 *
 *       Filename:  break.c
 *
 *    Description:  perform the brk system call
 *
 *        Version:  1.0
 *        Created:  2012年05月12日 09时21分20秒
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
#include "keyboard.h"
#include "proto.h"
#include "elf.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  do_brk
 *  Description:  perform the brk(addr) system call,
 *				  change the data segment of the proc
 *  @return:	  return 0 if success
 * =====================================================================================
 */
PUBLIC	int
do_brk ( )
{
	/* get the caller */
	int src = mm_msg.source;
	struct proc * p = &(proc_table[src]);

	/* increase the brk */
	u32 brk_addr = (u32)(mm_msg.BRK_ADDR);
	if (brk_addr >= p->end_text &&
			brk_addr < p->start_stack - MM_STACK_GAP) {
		p->brk = brk_addr;
	}

	/* return to the caller */
	mm_msg.BRK_ADDR = p->brk;
	return 0;
}		/* -----  end of function do_brk  ----- */
