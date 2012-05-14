/*************************************************************************//**
 *****************************************************************************
 * @file   fs/ls.c
 * The file contains:
 *   - do_ls()
 * @author 2Y_OS
 * @date   2012
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

/*****************************************************************************
 *                                do_ls
 *****************************************************************************/
/**
 * display all the files under the current directory.
 * just for now
 * 
 *****************************************************************************/
PUBLIC void do_ls()
{
	char *path = "/";
	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode * dir_inode;
	if (strip_path(filename, path, &dir_inode)) {
		return;
	}

	int dir_blk0_nr = dir_inode->i_start_sect;
        int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        int nr_dir_entries =
          dir_inode->i_size / DIRENT_SIZE; 

        int m = 0;
        struct dirent * pde;
	int i, j;
        for (i = 0; i < nr_dir_blks; i++) {
                RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
                pde = (struct dirent *)fsbuf;
                for (j = 0; j < SECTOR_SIZE / DIRENT_SIZE; j++,pde++) {
                        printl("%s\n", pde->d_name);
                        if (++m > nr_dir_entries)
                                break;
                }
                if (m > nr_dir_entries)
                        break;
        }
}
