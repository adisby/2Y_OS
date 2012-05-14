/*************************************************************************//**
 *****************************************************************************
 * @file   mm/main.c
 * @brief  Orange'S Memory Management.
 * @author Forrest Y. Yu
 * @date   Tue May  6 00:33:39 2008
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "config.h"
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

PUBLIC void do_fork_test();

PRIVATE void init_mm();

/*****************************************************************************
 *                                task_mm
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK MM.
 * 
 *****************************************************************************/
PUBLIC void task_mm()
{
	init_mm();

	while (1) {
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;

		int msgtype = mm_msg.type;

		switch (msgtype) {
		case FORK:
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			reply = 0;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();
			break;
		case WAIT:
			do_wait();
			reply = 0;
			break;
		case BRK:
			mm_msg.RETVAL = do_brk();
			break;
		default:
			dump_msg("MM::unknown msg", &mm_msg);
			assert(0);
			break;
		}

		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}

/*****************************************************************************
 *                               init_mm_page_table 
 *****************************************************************************/
/**
 * Do some initialization work.
 * 
 *****************************************************************************/
PRIVATE void init_mm_page_table()
{
	int i;
    for (i = 0; i < nr_page; i++) {
        page_table[i] = MM_PAGE_FREE;
    }
	for (; i < MM_NR_PAGE; i++) {
		page_table[i] = MM_PAGE_UNAVAILABLE;
	}
}



/*****************************************************************************
 *                                init_mm
 *****************************************************************************/
/**
 * Do some initialization work.
 * 
 *****************************************************************************/
PRIVATE void init_mm()
{
	struct boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;
	mm_free_mem = memory_size - MM_KERNEL_END;
	nr_page	= (bp.mem_size - MM_KERNEL_END) / PAGE_SIZE;

	init_mm_page_table();

	/* print memory size */
	printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
}

/*****************************************************************************
 *                                alloc_mem
 *****************************************************************************/
/**
 * Allocate a memory block for a proc.
 * 
 * @param pid  Which proc the memory is for.
 * @param memsize  How many bytes is needed.
 * 
 * @return  The base of the memory just allocated.
 *****************************************************************************/
PUBLIC int alloc_mem(int pid, int memsize)
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > VM_SIZE) {
		panic("unsupported memory request: %d. "
		      "(should be less than %d)",
		      memsize,
		      VM_SIZE);
	}

	int base = (pid - (NR_TASKS + NR_NATIVE_PROCS) + 1) * VM_SIZE;

	return base;
}

/*****************************************************************************
 *                                free_mem
 *****************************************************************************/
/**
 * Free a memory block. free the page tables 
 * 
 * @param pid  Whose memory is to be freed.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PUBLIC int free_mem(int pid)
{
	/* duplicate the process: T, D & S */
	struct descriptor * ppd;

	/* Text segment */
	ppd = &proc_table[pid].ldts[INDEX_LDT_C];
	/* base of T-seg, in bytes */
	int caller_T_base  = reassembly(ppd->base_high, 24,
					ppd->base_mid,  16,
					ppd->base_low);
	/* limit of T-seg, in 1 or 4096 bytes,
	   depending on the G bit of descriptor */
	int caller_T_limit = reassembly(0, 0,
					(ppd->limit_high_attr2 & 0xF), 16,
					ppd->limit_low);
	/* size of T-seg, in bytes */
	int caller_T_size  = ((caller_T_limit + 1) *
			      ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
			       4096 : 1));

	/* Data & Stack segments */
	ppd = &proc_table[pid].ldts[INDEX_LDT_RW];
	/* base of D&S-seg, in bytes */
	int caller_D_S_base  = reassembly(ppd->base_high, 24,
					  ppd->base_mid,  16,
					  ppd->base_low);
	/* limit of D&S-seg, in 1 or 4096 bytes,
	   depending on the G bit of descriptor */
	int caller_D_S_limit = reassembly((ppd->limit_high_attr2 & 0xF), 16,
					  0, 0,
					  ppd->limit_low);
	/* size of D&S-seg, in bytes */
	int caller_D_S_size  = ((caller_T_limit + 1) *
				((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
				 4096 : 1));

	/* we don't separate T, D & S segments, so we have: */
	assert((caller_T_base  == caller_D_S_base ) &&
	       (caller_T_limit <= caller_D_S_limit) &&
	       (caller_T_size  <= caller_D_S_size ));

	/* clean up the page table */
	free_page_tables(caller_D_S_base, caller_D_S_size);

	return 0;
}
