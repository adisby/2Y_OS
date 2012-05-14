/*************************************************************************//**
 *****************************************************************************
 * @file   stat.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   Wed May 21 21:17:21 2008
 *****************************************************************************
 *****************************************************************************/

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


/*****************************************************************************
 *                                stat
 *************************************************************************//**
 * 
 * 
 * @param path 
 * @param buf 
 * 
 * @return  On success, zero is returned. On error, -1 is returned.
 *****************************************************************************/
PUBLIC int stat(const char *path, struct stat *buf)
{
	MESSAGE msg;

	msg.type	= STAT;

	msg.PATHNAME	= (void*)path;
	msg.BUF		= (void*)buf;
	msg.NAME_LEN	= strlen(path);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mkdir
 *  Description:  attempts to create a directory named pathname
 *  @param pathname: the path name of the new directory
 *  @param mode:  the mode of the new directory
 *  @return:	  return 0 if success
 * =====================================================================================
 */
PUBLIC	int
mkdir (const char *pathname, int mode)
{
	MESSAGE msg;

	msg.type		= MKDIR;
	printl("{lib} MKDIR : %d\n", MKDIR);
	msg.PATHNAME	= (void *)pathname;
	msg.NAME_LEN	= strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}		/* -----  end of function mkdir  ----- */
