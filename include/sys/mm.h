/*
 * =====================================================================================
 *
 *       Filename:  mm.h
 *
 *    Description:  the head file of memory management
 *
 *        Version:  1.0
 *        Created:  2012年04月25日 23时54分21秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */

#ifndef	_2Y_OS_MM_H_
#define	_2Y_OS_MM_H_


/**
 * @def   PDT_ADDR
 * @brief where put the PDT
 * @see	  
 */
#define	PDT_ADDR 0x100000

/**
 * @def   PAGE_SIZE
 * @brief the size of a PAGE
 * @see	  
 */
#define	PAGE_SIZE	0x1000


/**
 * @def   ENTRY_SIZE
 * @brief the size of a pde or pte
 * @see	  
 */
#define	ENTRY_SIZE	0x4


/**
 * @def   
 * @brief 
 * @see	  
 */
#define	NR_ENTRY_PER_PAGE	(PAGE_SIZE / ENTRY_SIZE)


/**
 * @def   VM_SIZE
 * @brief the vitual memory size of a process
 * @see	  
 */
#define	VM_SIZE	0x4000000

/**
 * @def   MM_MEM_SIZE
 * @brief the size of the machine
 * @see	  
 */
#define	MM_MEM_SIZE (0x4000000)
							
/**
 * @def   MM_KERNEL_END
 * @brief the end of the kernel space
 * @see	  
 */
#define	MM_KERNEL_END   (0x400000)

/**
 * @def   MM_NR_KRNL_PAGE
 * @brief the amount of page in kernel
 * @see	  
 */
#define	MM_NR_KRNL_PAGE ( MM_KERNEL_END / PAGE_SIZE )

/**
 * @def   MM_NR_PAGE
 * @brief the amount of page in the machine
 * @see	  
 */
#define	MM_NR_PAGE ( (MM_MEM_SIZE - MM_KERNEL_END) / PAGE_SIZE )

/**
 * @def   MM_PAGE_FREE
 * @brief the flag that page is in use
 * @see	  
 */
#define	MM_PAGE_FREE    (0)

/**
 * @def   MM_PAGE_UNAVAILATBLE
 * @brief the flag that page is in not available
 * @see	  
 */
#define	MM_PAGE_UNAVAILABLE	(-1)

/**
 * @def   MM_PAGE_MASK
 * @brief the mask of page that shades the pde and pte
 * @see	  
 */
#define	MM_PAGE_MASK (0xfff)

/**
 * @def   MM_4M_MASK
 * @brief the mask of (n * 4m) nmemory start
 * @see	  
 */
#define	MM_4M_MASK (0x3fffff)


/**
 * @def   MM_PTE_MASK
 * @brief the mask of pte that shades the pde
 * @see	  
 */
#define	MM_PTE_MASK (0x3ff)

/**
 * @def   MM_PT_MASK
 * @brief the mask of pt that shades the sign bits
 * @see	  
 */
#define	MM_PT_MASK (0xfffff000)

/**
 * @def   MM_PG_P 
 * @brief the sign bit that show the existence of this page
 * @see	  
 */
#define	MM_PG_P (0x1)

/**
 * @def   MM_PG_RWR 
 * @brief the sign bit that decide the read/exec privilege
 * @see	  
 */
#define	MM_PG_RWR (0x0)

/**
 * @def   MM_PG_RWW 
 * @brief the sign bit that decide the read/write/exec privilege
 * @see	  
 */
#define	MM_PG_RWW (0x2)

/**
 * @def   MM_PG_USS
 * @brief the sign bit that decide super user privilege
 * @see	  
 */
#define	MM_PG_USS (0x0)

/**
 * @def   MM_PG_USU
 * @brief the sign bit that decide user privilege
 * @see	  
 */
#define	MM_PG_USU (0x4)

/**
 * @def   GET_IND_IN_PT
 * @brief get the index in the page table
 * @see	  
 */
#define GET_IND_IN_PT(addr) (((addr & MM_PT_MASK) - MM_KERNEL_END)>> 12 )

/**
 * @def   MM_STACK_SIZE
 * @brief the maximum size of stack
 * @see	  
 */
#define MM_STACK_SIZE (0x800000)

/**
 * @def   MM_STACK_GAP
 * @brief the gap between the end of data and the start of the stack
 * @see	  
 */
#define MM_STACK_GAP (0x10000)

#endif
