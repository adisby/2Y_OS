/*
* =====================================================================================
*
* Filename: memory.c
*
* Description: the file to change the direcotry
*
* Version: 1.0
* Created: 2012年05月05日 02时07分29秒
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
#include "fs.h"
#include "mm.h"
#include "dirent.h"
#include "unistd.h"
#include "stdlib.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "string.h"
#include "global.h"
#include "proto.h"

/* 本文件内函数声明 */
PRIVATE u32 get_free_page();
PRIVATE void un_wp_page (u32 * pte);
PRIVATE	void write_verify(u32 address);
PRIVATE	void free_page (u32 addr);

/*
 * === FUNCTION ======================================================================
 * Name: invalidate
 * Description: refresh the TLB
 * =====================================================================================
 */
#define invalidate() \
        __asm__ __volatile__ ("movl %%eax, %%cr3"::"a"(PDT_ADDR))

/*
 * === FUNCTION ======================================================================
 * Name: get_free_page
 * Description: get a free page from page table
 * @return: the address of the page
 * =====================================================================================
 */
PRIVATE u32 get_free_page() {
        int i;
        for (i = 0; i < nr_page && MM_PAGE_FREE != page_table[i]; i++);
        if (nr_page == i) {
            sys_printx(0, 0, "\002no enough page\n", p_proc_ready);
        }
        //the page_table is engaged and it can be shared
        page_table[i]++;
        //decrease the memory size
        mm_free_mem -= PAGE_SIZE;
        u32 page = MM_KERNEL_END + 0x1000 * i;
		//initial the page
		memset((void *)page, 0, PAGE_SIZE);
		return page;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  free_page
 *  Description:  free a page of a proc at physical address address
 *  @param	addr: the page table entry of a page
 * =====================================================================================
 */
PRIVATE		void
free_page (u32 addr)
{
		if (addr < MM_KERNEL_END) {
				//can not free kernel address
				return;
		}
		if (addr >= memory_size) {
				//the addr is lager than physical memory
				sys_printx(0, 0, "\002try to free nonexistent page\n", p_proc_ready);
		}
		int page_ind = GET_IND_IN_PT(addr);
		if (page_table[page_ind] > 0) {
				//normal
				page_table[page_ind]--;
				if (!page_table[page_ind]) {
					mm_free_mem += PAGE_SIZE;
				}
		} else {
				//the page is free
				page_table[page_ind] = 0;
				sys_printx(0, 0, "\002try to free free page\n", p_proc_ready);
		}
}		/* -----  end of function free_page  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  free_page_tables
 *  Description:  free the page tables of a process (ring 1)
 *  @param start: the start of the linear address
 *  @param size:  the size of the proc
 * =====================================================================================
 */
PUBLIC	void
free_page_tables (u32 start, u32 size)
{
		u32* pd = (u32 *)PDT_ADDR;
		u32* pt;
		int pde_align;
		int i;

		if (start & MM_4M_MASK) {
				panic("free_page_tables called with wrong alignment\n");
		}
		if (!start) {
				panic("try to free kernel space");
		}

		size = (size + MM_4M_MASK) >> 22;
		pde_align = (start >> 22);

		while (size-->0) {
				if (!(pd[pde_align] & MM_PG_P)) {
						//the page table is not exist
						continue;
				}
				pt = (u32 *)(pd[pde_align] & MM_PT_MASK);
				for (i = 0; i < NR_ENTRY_PER_PAGE; i++) {
						if (pt[i] & MM_PG_P) {
								free_page(pt[i] & MM_PT_MASK);
						}
						pt[i] = 0;
				}
				free_page((u32)pt);
				pd[pde_align] = 0;
				pde_align++;
		}
}		/* -----  end of function free_page_tables  ----- */

/*
 * === FUNCTION ======================================================================
 * Name: do_no_page
 * Description: do the page fault that the page is missing
 * @param error_code :
 * @param address : the address that cause the error
 * =====================================================================================
 */
PUBLIC void do_no_page(u32 error_code, u32 address) {
        if (address < MM_KERNEL_END) {
            sys_printx(0, 0, "\002do_no_page segment fault\n", p_proc_ready);
        }

		u32	*pd = (u32 *)PDT_ADDR;
		u32	*pt;
		address &= MM_PT_MASK;

		int pt_align = address >> 22;
		pt = (u32 *)pd[pt_align];
		if (! ((u32)pt & MM_PG_P)) {
			//page table is not available
			pt = (u32 *)get_free_page();
			pd[pt_align] = (u32)pt
				| MM_PG_P
				| MM_PG_USU
				| MM_PG_RWW;
		} else {
			pt = (u32 *) ((u32)pt & MM_PT_MASK);
		}
		//get a new page
        pt[(address >> 12) & MM_PTE_MASK] = get_free_page()
            | MM_PG_P
            | MM_PG_USU
            | MM_PG_RWW;

		//refresh the TLB
        invalidate();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  un_wp_page
 *  Description:  call off one shared page
 *  @param	pte:  the page table entry that need to call off shared page
 * =====================================================================================
 */
PRIVATE		void
un_wp_page (u32 * pte)
{
		u32 orig_page;
		orig_page = *pte & MM_PT_MASK;
        int index = GET_IND_IN_PT(orig_page);
        if (1 == page_table[index]) {
            *pte |= MM_PG_RWW;
            invalidate();
        } else {
            *pte = get_free_page()
                | MM_PG_P
                | MM_PG_USU
                | MM_PG_RWW;

			//sync the page table
			page_table[index]--;
            
            invalidate();
            phys_copy((void *)(*pte & MM_PT_MASK),
                (void *)(orig_page & MM_PT_MASK),
                PAGE_SIZE);
        }
		return ;
}		/* -----  end of function un_wp_page  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  write_verify
 *  Description:  if the page is not writable, un wp the page
 *  @param address : the address of the page that need to be verify  
 * =====================================================================================
 */
PRIVATE void
write_verify (u32 address)
{       
	   	if (address < MM_KERNEL_END) {
				return;
//            sys_printx(0, 0, "\002write_verify segment fault\n", p_proc_ready);
		}

		u32	*pd = (u32 *)PDT_ADDR;
		u32	*pt;
		address &= MM_PT_MASK;

		int pt_align = address >> 22;
		pt = (u32 *)pd[pt_align];
		if (! ((u32)pt & MM_PG_P)) {
			//the page table is unavailable, there is no need to verify
			return;
		} else {
			pt = (u32 *) ((u32)pt & MM_PT_MASK);
		}

		int pte_ind = ((address >> 12) & MM_PTE_MASK);
		if (MM_PG_P == (pt[pte_ind]	& (MM_PG_P | MM_PG_RWW))) {
				un_wp_page(&(pt[pte_ind]));
		}
}		/* -----  end of function write_verify  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  verify_area
 *  Description:  verify a area that need to write
 *  @param address: the address that start to verify, linear address
 *  @param size:	the size of the area
 * =====================================================================================
 */
PUBLIC	void
verify_area (u32 address, int size)
{
		//change address to the left start of the page
		address &= MM_PT_MASK;
		//increase the size for insurance
		size += (address & MM_PTE_MASK);
		while (size > 0) {
				size -= PAGE_SIZE;
				write_verify(address);
				address += PAGE_SIZE;
		}
}		/* -----  end of function verify_area  ----- */

/*
 * === FUNCTION ======================================================================
 * Name: do_wp_page
 * Description: do the page fault that the page is only readable
 * @param error_code :
 * @param address : the address that cause the error
 * =====================================================================================
 */
PUBLIC void do_wp_page(u32 error_code, u32 address) {
        if (address < MM_KERNEL_END) {
            sys_printx(0, 0, "\002do_wp_page segment fault\n", p_proc_ready);
		}

		u32	*pd = (u32 *)PDT_ADDR;
		u32	*pt;
		address &= MM_PT_MASK;

		int pt_align = address >> 22;
		pt = (u32 *)pd[pt_align];
		if (! ((u32)pt & MM_PG_P)) {
			//page table is not available
			pt = (u32 *)get_free_page();
			pd[pt_align] = (u32)pt
				| MM_PG_P
				| MM_PG_USU
				| MM_PG_RWW;
		} else {
			pt = (u32 *) ((u32)pt & MM_PT_MASK);
		}

		int pte_ind = ((address >> 12) & MM_PTE_MASK);
		un_wp_page(&(pt[pte_ind]));
}

/*****************************************************************************
 *                               copy_on_write 
 *****************************************************************************/
/**
 * Perform the copy_on_write() syscall. from the left to right
 * @param l		: the left start
 * @param r		: the right start
 * @param base  : the memory base of right one
 * @param size  : the memory size of right one
 *****************************************************************************/
PUBLIC void copy_on_write(u32 l, u32 r, u32 size)
{
	//check the l and the r are at the start of 4M
	if ((l & MM_4M_MASK) || (r & MM_4M_MASK)) {
		panic("copy_on_write called when not start at (n * 4M)\n");
	}

	u32 * pdt = (u32 *)PDT_ADDR;

	u32	l_start_pde = l >> 22;
	u32 r_start_pde = r >> 22;

	u32 l_start_pte = (l >> 12) & MM_PTE_MASK;
	u32 r_start_pte = (r >> 12) & MM_PTE_MASK;

	int i = 0;
	int nr_copy_page = size >> 12;
	//add a page if the addr is not end in the end of page 
//	if (size & MM_PAGE_MASK) {
//		nr_copy_page++;
//	}
	//add a page if the start of the page is not a star of page
//	if (l & MM_PAGE_MASK) {
//		nr_copy_page++;
//	}

	u32 curr_l_pde = l_start_pde;
	u32 curr_r_pde = r_start_pde;
	u32 curr_l_pte = l_start_pte;
	u32 curr_r_pte = r_start_pte;
	while (i++ < nr_copy_page) {
		//the current page table of right one does not exist
		if (!(pdt[curr_l_pde] & MM_PG_P)) {
			pdt[curr_l_pde] = get_free_page()
				| MM_PG_P
				| MM_PG_USU
				| MM_PG_RWW;
		}
		u32 * l_pt = (u32 *)(pdt[curr_l_pde] & MM_PT_MASK);
		//the current page table of right one does not exist
		if (!(pdt[curr_r_pde] & MM_PG_P)) {
			pdt[curr_r_pde] = get_free_page()
				| MM_PG_P
				| MM_PG_USU
				| MM_PG_RWW;
		}
		u32 * r_pt = (u32 *)(pdt[curr_r_pde] & MM_PT_MASK);
		if (!(l_pt[curr_l_pte] & MM_PG_P)) {
			//the pte of the left one is not present, just jump to next
			continue;
		}
		if ((r_pt[curr_r_pte] & MM_PG_P)) {
			//the pte of the right one is already exist
			panic("copy_on_write when the des is already exist\n");
		}
		l_pt[curr_l_pte] &= (~2);
		r_pt[curr_r_pte] = l_pt[curr_l_pte];

		//sync the free page table
		int index  = GET_IND_IN_PT(l_pt[curr_l_pte]);
		page_table[index]++;

		//inc
		curr_l_pte++;
		curr_r_pte++;
		//the current pte index has reached the maximem
		if (!(curr_l_pte ^ MM_PTE_MASK)) {
			curr_l_pte = 0;
			curr_l_pde++;
		}
		if (!(curr_r_pte ^ MM_PTE_MASK)) {
			curr_r_pte = 0;
			curr_r_pde++;
		}
	}
}
