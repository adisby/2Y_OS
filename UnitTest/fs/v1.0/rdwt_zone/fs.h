/*************************************************************************//**
 *****************************************************************************
 * @file   include/sys/fs.h
 * @brief  Header file for File System.
 * @author Forrest Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

#ifndef	_ORANGES_FS_H_
#define	_ORANGES_FS_H_

/**
 * @struct dev_drv_map fs.h "include/sys/fs.h"
 * @brief  The Device_nr.\ - Driver_nr.\ MAP.
 */
struct dev_drv_map {
	int driver_nr; /**< The proc nr.\ of the device driver. */
};

/**
 * @def   MAGIC_V1
 * @brief Magic number of FS v1.0
 */
#define	MAGIC_V1	0x111

/**
 * @struct super_block fs.h "include/fs.h"
 * @brief  The 2nd sector of the FS
 *
 * Remember to change SUPER_BLOCK_SIZE if the members are changed.
 */
struct super_block {
	u32	magic;		  /**< Magic number */
	u32	nr_inodes;	  /**< How many inodes */
	u32	nr_sects;	  /**< How many sectors */
	u32	nr_imap_sects;	  /**< How many inode-map sectors */
	u32	nr_smap_sects;	  /**< How many sector-map sectors */
	u32	n_1st_sect;	  /**< Number of the 1st data sector */
	u32	nr_inode_sects;   /**< How many inode sectors */
	u32	root_inode;       /**< Inode nr of root directory */
	u32	inode_size;       /**< INODE_SIZE */
	u32	inode_isize_off;  /**< Offset of `struct inode::i_size' */
	u32	inode_zone_off;  /**< Offset of `struct inode::i_start_sect' */
	u32	dir_ent_size;     /**< DIRENT_SIZE */
	u32	dir_ent_inode_off;/**< Offset of `struct dirent::inode_nr' */
	u32	dir_ent_fname_off;/**< Offset of `struct dirent::name' */

	/*
	 * the following item(s) are only present in memory
	 */
	int	sb_dev; 	/**< the super block's home device */
};

/**
 * @def   SUPER_BLOCK_SIZE
 * @brief The size of super block \b in \b the \b device.
 *
 * Note that this is the size of the struct in the device, \b NOT in memory.
 * The size in memory is larger because of some more members.
 */
#define	SUPER_BLOCK_SIZE	56

/**
 * @struct inode
 * @brief  i-node
 *
 * The \c start_sect and\c nr_sects locate the file in the device,
 * and the size show how many bytes is used.
 * If <tt> size < (nr_sects * SECTOR_SIZE) </tt>, the rest bytes
 * are wasted and reserved for later writing.
 *
 * \b NOTE: Remember to change INODE_SIZE if the members are changed
 */
struct inode {
	u32	i_mode;		/**< Accsess mode */
	u32	i_size;		/**< File size */
//	u32	i_start_sect;	/**< The first sector of the data */
	u32	i_nr_sects;	/**< How many sectors the file occupies */
	zone_t i_zone[10];	/**< How many zones the file occupies */
	u8	_unused[12];	/**< Stuff for alignment */

	/* the following items are only present in memory */
	int	i_dev;
	int	i_cnt;		/**< How many procs share this inode  */
	int	i_num;		/**< inode nr.  */
};

/**
 * @def   INODE_SIZE
 * @brief The size of i-node stored \b in \b the \b device.
 *
 * Note that this is the size of the struct in the device, \b NOT in memory.
 * The size in memory is larger because of some more members.
 */
#define	INODE_SIZE	(sizeof(struct inode) - 12)

/**
 * @def   NR_BYTE_PER_SECTOR_IN_ZONE
 * @brief 
 * @see	  
 */
#define	NR_BYTE_PER_SECTOR_IN_ZONE	(sizeof(zone_t))

/**
 * @def   NR_ZONE_PER_SECT
 * @brief the amount of zones in a sector
 * @see	  
 */
#define	NR_ZONE_PER_SECT	((SECTOR_SIZE) / (NR_BYTE_PER_SECTOR_IN_ZONE))

/**
 * @def   NR_SECT_PER_SECONDLEVEL_ZONE
 * @brief the amount of zones in one second level zone
 * @see	  
 */
#define	NR_SECT_PER_SECOND_LEVEL_ZONE	NR_ZONE_PER_SECT

/**
 * @def   NO_ZONE
 * @brief if there is no zone in the slot
 * @see	  
 */
#define NO_ZONE				0		

/**
 * @def   DEV_ZONE
 * @brief the zone of the dev
 * @see	  
 */
#define	DEV_ZONE			0

/**
 * @def   NR_DIRECT_ZONE		
 * @brief the max amount of the direct zones
 * @see	  
 */
#define NR_DIRECT_ZONE		8

/**
 * @def   SECOND_LEVEL_ZONE
 * @brief the index of second level zone
 * @see	  
 */
#define	SECOND_LEVEL_ZONE	8

/**
 * @def	  SECOND_LEVEL_ZONE	 
 * @brief the second level indirect zone
 * @see	  
 */
#define	NR_SECOND_LEVEL_ZONE	(NR_DIRECT_ZONE + NR_SECT_PER_SECOND_LEVEL_ZONE)

/**
 * @def   THIRD_LEVEL_ZONE
 * @brief the third level indirect zone
 * @see	  
 */
#define	THIRD_LEVEL_ZONE	9


/**
 * @def   NR_ZONE_PER_INODE
 * @brief the zones in a inode
 * @see	  
 */
#define	NR_ZONE_PER_INODE	(THIRD_LEVEL_ZONE + 1)

/**
 * @def   NR_THIRD_LEVEL_ZONE	
 * @brief the third level indirect zone
 * @see	  
 */
#define NR_THIRD_LEVEL_ZONE		(NR_SECOND_LEVEL_ZONE + NR_SECT_PER_SECOND_LEVEL_ZONE * NR_SECT_PER_SECOND_LEVEL_ZONE)

/**
 * @struct file_desc
 * @brief  File Descriptor
 */
struct file_desc {
	int		fd_mode;	/**< R or W */
	int		fd_pos;		/**< Current position for R/W. */
	int		fd_cnt;		/**< How many procs share this desc */
	struct inode*	fd_inode;	/**< Ptr to the i-node */
};

#define phys_copy memcpy

/**
 * Since all invocations of `rw_sector()' in FS look similar (most of the
 * params are the same), we use this macro to make code more readable.
 */
#endif /* _ORANGES_FS_H_ */
