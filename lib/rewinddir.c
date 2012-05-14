/*
 * =====================================================================================
 *
 *       Filename:  rewinddir.c
 *
 *    Description:  rewind a directory
 *
 *        Version:  1.0
 *        Created:  2012年05月13日 21时50分40秒
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

void rewinddir(DIR * dp)
{
	(void) seekdir(dp, 0);
}
