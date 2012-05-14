/*
 * =====================================================================================
 *
 *       Filename:  malloc.c
 *
 *    Description:  the malloc c source file
 *
 *        Version:  1.0
 *        Created:  2012年02月27日 00时43分11秒
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

#define	MA_NIL	((MEM_AREA *) 0)
#define MA_USED	1
#define MA_FREE	0
typedef	struct	mem_area {
	void * start;
	u32    size;
	int	   status;
	struct mem_area * next;
}MEM_AREA;

extern	void* _brksize;
static	void* proc_use;
static	MEM_AREA *	head;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  malloc
 *  Description:  the most simple malloc function
 *  @param _size: the size of the allocated memory space
 *  @return:	  the allocated memory address
 * =====================================================================================
 */
PUBLIC	void *
malloc (size_t _size)
{
	printf("%s %d malloc start\n", __FILE__, __LINE__, _brksize);

	if (0 == proc_use || 0 == _brksize) {
		sbrk(0);
		proc_use = _brksize;
		printf("%s %d _brksize %d\n", __FILE__, __LINE__, _brksize);
	}

	//try to find a free slot that is enough for the new malloc
	MEM_AREA * ma = head;
	while (MA_NIL != ma) {
		if (MA_FREE == ma->status &&
				ma->size >= _size) {
			ma->status = MA_USED;
			printf("%s %d the free slot: %d\n", __FILE__, __LINE__, _brksize, ma->start);
			return ma->start;
		}
		ma = ma->next;
	}
	
	printf("%s %s can't find free slot\n", __FILE__, __LINE__, _brksize);

	//find a the end of the list
	ma = head;
	do {
		if (MA_NIL == ma ||
				MA_NIL == ma->next) {
			break;
		}
		ma = ma->next;
	} while (MA_NIL != ma->next);

	//increase the data segment
	u32 new_proc_use = (u32)proc_use + sizeof(MEM_AREA) + _size;
	if (new_proc_use > (u32)_brksize) {
		sbrk(_size);
	}

	MEM_AREA * t;
	if (MA_NIL == head) {
		t = head = (MEM_AREA *)proc_use;
	} else if (MA_NIL == ma->next){
		t = ma->next = (MEM_AREA *)proc_use;
	}
	t->start = (void *)((u32)proc_use + sizeof(MEM_AREA));
	t->size	 = _size;
	t->status= MA_USED;

	printf("%s %d malloc start %d\n", __FILE__, __LINE__, (u32)(t->start));
	return (void *)(t->start);
}		/* -----  end of function malloc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  free
 *  Description:  free the memory allocated
 *  @param addr:  the start address that the memory allocated
 * =====================================================================================
 */
	void
free (void * addr)
{
	MEM_AREA * ma = head;
	if (MA_NIL == ma) {
		printf("try to free free slot\n");
		return;
	}
	while (MA_NIL != ma) {
		if (addr == ma->start) {
			ma->status = MA_FREE;
			return;
		}
	}
	printf("no such allocated address\n");
}		/* -----  end of function free  ----- */
