/*************************************************************************//**
 *****************************************************************************
 * @file   mm/exec.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   Tue May  6 14:14:02 2008
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
#include "keyboard.h"
#include "proto.h"
#include "elf.h"

PRIVATE void tell_fs(int what, int p1, int p2, int p3);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  tell_fs
 *  Description:  tell the fs to change the work directory or other
 *  @param what:  the system call
 *  @param p1:	  the first info
 *  @param p2:	  the second info
 *  @param p3:	  the third info
 * =====================================================================================
 */
PRIVATE	void
tell_fs (int what, int p1, int p2, int p3)
{

	/*-----------------------------------------------------------------------------
	 *  This routine is used by TASK_MM to inform FS of certain events
	 *		tell_fs(CHDIR, proc);
	 *-----------------------------------------------------------------------------*/
	MESSAGE m;
	m.type= what;
	m.PROC_NR = p1;
	m.source = TASK_MM;
	send_recv(BOTH, TASK_FS, &m);
	assert(SYSCALL_RET == m.type);
}		/* -----  end of function tell_fs  ----- */

/*****************************************************************************
 *                                do_exec
 *****************************************************************************/
/**
 * Perform the exec() system call.
 * 
 * @return  Zero if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int do_exec()
{
	/* get parameters from the message */
	int name_len = mm_msg.NAME_LEN;	/* length of filename */
	int src = mm_msg.source;	/* caller proc nr. */
	assert(name_len < MAX_PATH);

	char pathname[MAX_PATH];
	phys_copy((void*)va2la(TASK_MM, pathname),
		  (void*)va2la(src, mm_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;	/* terminate the string */

	/* change the directory of task fs to the caller's work directory  */
	tell_fs(CHDIR, src, 0, 0);

	/* get the file size */
	struct stat s;
	int ret = stat(pathname, &s);
	if (ret != 0) {
		printl("{MM} MM::do_exec()::stat() returns error. %s", pathname);
		return -1;
	}

	/* read the file */
	int fd = open(pathname, O_RDWR);
	if (fd == -1)
		return -1;
	assert(s.st_size < MMBUF_SIZE);
	read(fd, mmbuf, s.st_size);
	close(fd);

	/* setup the arg stack */
	int orig_stack_len = mm_msg.BUF_LEN;
	char stackcopy[PROC_ORIGIN_STACK];
	phys_copy((void*)va2la(TASK_MM, stackcopy),
		  (void*)va2la(src, mm_msg.BUF),
		  orig_stack_len);

	/* clean up the image of the current process */
	free_mem(src);

	/* the information about elf32 */
	u32 start_text = 0, end_text = 0, start_data = 0, end_data = 0;

	/* overwrite the current proc image with the new one */
	Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)(mmbuf);
	int i;
	for (i = 0; i < elf_hdr->e_phnum; i++) {
		Elf32_Phdr* prog_hdr = (Elf32_Phdr*)(mmbuf + elf_hdr->e_phoff +
			 			(i * elf_hdr->e_phentsize));
		if (prog_hdr->p_type == PT_LOAD) {
			assert(prog_hdr->p_vaddr + prog_hdr->p_memsz <
				PROC_IMAGE_SIZE_DEFAULT);
			phys_copy((void*)va2la(src, (void*)prog_hdr->p_vaddr),
				  (void*)va2la(TASK_MM,
						 mmbuf + prog_hdr->p_offset),
				  prog_hdr->p_filesz);
			if ((PF_R | PF_W) == prog_hdr->p_flags) {
					//the data segment
//					printl("elf32 .data start %d, size %d\n", prog_hdr->p_vaddr
//									, prog_hdr->p_memsz);
					start_data = prog_hdr->p_vaddr;
					end_data = start_data + prog_hdr->p_memsz;
			} else if ((PF_R | PF_X) == prog_hdr->p_flags) {
					//the text segment
//					printl("elf32 .text start %d, size %d\n", prog_hdr->p_vaddr
//									, prog_hdr->p_memsz);
					start_text = prog_hdr->p_vaddr;
					end_text = start_text + prog_hdr->p_memsz;
			}
		}
	}

	/* setup the arg stack */
//	int orig_stack_len = mm_msg.BUF_LEN;
//	char stackcopy[PROC_ORIGIN_STACK];
//	phys_copy((void*)va2la(TASK_MM, stackcopy),
//		  (void*)va2la(src, mm_msg.BUF),
//		  orig_stack_len);

	initial_brk(src, start_text, end_text, start_data, end_data);

	int * orig_stack = (int*)(VM_SIZE - PROC_ORIGIN_STACK);

	int delta = (int)orig_stack - (int)mm_msg.BUF;

	int argc = 0;
	if (orig_stack_len) {	/* has args */
		char **q = (char**)stackcopy;
		for (; *q != 0; q++,argc++)
			*q += delta;
	}

	phys_copy((void*)va2la(src, orig_stack),
		  (void*)va2la(TASK_MM, stackcopy),
		  orig_stack_len);

	proc_table[src].regs.ecx = argc; /* argc */
	proc_table[src].regs.eax = (u32)orig_stack; /* argv */

	/* setup eip & esp */
	proc_table[src].regs.eip = elf_hdr->e_entry; /* @see _start.asm */
	proc_table[src].regs.esp = VM_SIZE - PROC_ORIGIN_STACK;

	strcpy(proc_table[src].name, pathname);

	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initial_brk
 *  Description:  initial the information of elf32 for the brk
 *  @param pid:
 *  @param start_text:
 *  @param end_text:
 *  @param start_data:
 *  @param end_data:
 * =====================================================================================
 */
PUBLIC	void
initial_brk (	int pid,
				u32 start_text,
			   	u32 end_text,
			   	u32 start_data,
			   	u32 end_data)
{

	//initial the brk
	if ((!start_data) || (!end_data)) {
			//the size of data segment is zero
			start_data = end_data = (end_text + PAGE_SIZE); //the data and text has a gap of one page size
	}
	u32 brk = end_data;
	u32 start_stack = VM_SIZE - PROC_ORIGIN_STACK - MM_STACK_SIZE;

	struct proc * p = &(proc_table[pid]);
	p->start_text	= start_text;
	p->end_text		= end_text;
	p->start_data	= start_data;
	p->end_data		= end_data;
	p->start_stack	= start_stack;
	p->brk			= brk;
}		/* -----  end of function initial_brk  ----- */
