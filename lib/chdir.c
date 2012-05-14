/*
* =====================================================================================
*
* Filename: chdir.c
*
* Description: the file to change the direcotry
*
* Version: 1.0
* Created: 2012年03月07日 02时07分29秒
* Revision: none
* Compiler: gcc
*
* Author: adisby (), adisbyPD@gmail.com
* Company: 2Y_OS
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
 * === FUNCTION ======================================================================
 * Name: chdir
 * Description: change the directory to the path
 * @param path: the target directory
 * @return: return 0 if success, -1 failed
 * =====================================================================================
 */
PUBLIC	int
chdir (const char * path)
{
	MESSAGE msg;

	msg.type = CHDIR;

	msg.PATH = (void *)path;
	msg.NAME_LEN = strlen(path);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
} /* ----- end of function chdir ----- */
