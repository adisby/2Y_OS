/*
 * =====================================================================================
 *
 *       Filename:  fs_stat.c
 *
 *    Description:  do something with dirent. stat chdir ,etc
 *
 *        Version:  1.0
 *        Created:  2012年02月26日 01时08分13秒
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
#include "keyboard.h"
#include "proto.h"
#include "elf.h"

/*-----------------------------------------------------------------------------
 *  change_porc_dir		PRIVATE
 *  do_stat				PUBLIC
 *  do_chdir			PUBLIC
 *-----------------------------------------------------------------------------*/

PRIVATE int change_proc_dir (struct inode ** work_dir, struct inode * root_dir, char * dir_name, int len  );

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  do_stat
 *  Description:  
 * =====================================================================================
 */
/*  	void
do_stat (  )
{
	return ;
}*/		/* -----  end of function do_stat  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  do_chdir
 *  Description:  do change dirent for the caller
 *  @return	:	  return 0 if do it successfully
 * =====================================================================================
 */
PUBLIC	int
do_chdir ()
{
	int src = fs_msg.source;
	if (TASK_MM == src) {
		//get the proc that the work directory and root directory of FS will change to
		struct proc * change_to_proc = &proc_table[fs_msg.PROC_NR];

		assert(NULL != change_to_proc->root);
		put_inode(p_proc_ready->root);
		change_to_proc->root->i_cnt++;
		p_proc_ready->root = change_to_proc->root;

		assert(NULL != change_to_proc->pwd);
		put_inode(p_proc_ready->pwd);
		change_to_proc->pwd->i_cnt++;
		p_proc_ready->pwd = change_to_proc->pwd;

		return 0;
	}

	char path[MAX_PATH];
    phys_copy((void*)va2la(TASK_FS, path),
          (void*)va2la(src, fs_msg.PATH),
          fs_msg.NAME_LEN);
	path[fs_msg.NAME_LEN] = '\0';

	struct proc * caller = &proc_table[src];
	change_proc_dir(&(caller->pwd), caller->root, path, fs_msg.NAME_LEN);
	return 0;
}		/* -----  end of function do_chdir  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  change_proc_dir
 *  Description:  if the caller is not TASK_MM, change the directory of this proc
 *  @param work_dir:	the directory which the current process is working in
 *  @param dir_name:	the directory which the work_dir will change to
 *  @param len:		the length of the dir_name
 *  @return:		return 0 if do it successfully
 * =====================================================================================
 */
PRIVATE	int
change_proc_dir (struct inode ** work_dir,
				struct inode * root_dir,
			   	char * dir_name,
			   	int len  )
{
//	printl("{FS} start to change proc dir\n");
	int d_ino = search_file(dir_name);
	if (INVALID_INODE == d_ino) {
		printl("no such file or directory\n");
		return 1;
	}
//	printl("{FS} get the d_ino\n");

	//the directory should share the dev with work_dir now
	struct inode * dir_inode  = get_inode((*work_dir)->i_dev, d_ino);
	if (NULL == dir_inode) {
		printl("can't get the inode struct of this directory\n");
		return 1;
	}
//	printl("{FS} get the dir_inode\n");

	//release the previous inode.
	//if we do not use env, so do not release root_inode
//	if (*work_dir != root_dir) {
		assert(*work_dir != dir_inode);
		put_inode(*work_dir);
//	}

	*work_dir = dir_inode;
//	printl("{FS} change directory success\n");

	return 0;
}		/* -----  end of function change_proc_dir  ----- */
