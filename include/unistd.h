/*
 * =====================================================================================
 *
 *       Filename:  unistd.h
 *
 *    Description:  the unix special head file
 *
 *        Version:  1.0
 *        Created:  2012年02月26日 00时51分21秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */

#ifndef _2Y_OS_UNISTD_H_
#define _2Y_OS_UNISTD_H_

extern int	chdir(const char *path);

/* lib/brk.c */
extern int	brk(void * addr);
extern void sbrk(int incr);

#endif
