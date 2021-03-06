/*
 * =====================================================================================
 *
 *       Filename:  stdlib.h
 *
 *    Description:  the standard head file stdlib
 *
 *        Version:  1.0
 *        Created:  2012年02月26日 22时58分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */

#ifndef	_2Y_OS_STDLIB_H_
#define _2Y_OS_STDLIB_H_

#ifndef	NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef	unsigned int size_t;	/* type returned by sizeof */
#endif

extern void * malloc(size_t _size);
extern void	  free(void * addr);

#endif
