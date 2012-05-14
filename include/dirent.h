/*
 * =====================================================================================
 *
 *       Filename:  dirent.h
 *
 *    Description:  the POSIX head of dirent.h
 *
 *        Version:  1.0
 *        Created:  2012年01月04日 00时36分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */

#ifndef _2Y_OS_DIRENT_H
#define _2Y_OS_DIRENT_H   1

#include "sys/const.h"

/* POSIX inode serial type */
typedef	unsigned long int ino_t;
typedef unsigned long off_t;

/**
 * @def   MAX_FILENAME_LEN
 * @brief Max len of a filename
 * @see   dir_entry
 */
#define	MAX_FILENAME_LEN	12

/**
 * @struct dirent
 * @brief  Directory Entry
 *		   The dir entry of POSIX
 */
struct dirent {
	ino_t	d_ino;
	char	d_name[MAX_FILENAME_LEN];
};	/* ----------  end of struct dirent  ---------- */

/*
 * @def   DIRENT_SIZE
 * @brief The size of directory entry in the device.
 *
 * It is as same as the size in memory.
 */
#define	DIRENT_SIZE	sizeof(struct dirent)


/**
 * @def   DIR_PER_SECT
 * @brief the amout of struct dirent in a sector
 * @see	  
 */
#define	DIR_PER_SECT (SECTOR_SIZE / DIRENT_SIZE)

/*
 * @type  DIR
 * @brief the directory stream
 *	
 * Its actual structure is opaque to users.
 */
typedef struct
{
	int _fd;							/* file descriptor of the open directory  */
	int _count;							/* how many files in the directory  */
	int _pos;							/* the position in the open directory */
	struct dirent * _ptr;				/* the next slot  */
	struct dirent	_buf[DIR_PER_SECT]; /* the buf for the dirent in a sector */
} DIR;

/* the function to open a DIR */
extern DIR				*opendir(const char * __d_name);

/* the function to read a DIR and return the dir entry */
extern struct dirent	*readdir(DIR * __dirp);

/* close a DIR */
extern int				closedir(DIR * __dirp);

/* rewind a DIR*/
extern void				rewinddir(DIR * __dirp);

/* rewind a DIR*/
extern int				seekdir(DIR * __dirp, off_t __pos);

#endif /*  dirent.h  */
