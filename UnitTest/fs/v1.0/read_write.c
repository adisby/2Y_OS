/*************************************************************************//**
 *****************************************************************************
 * @file   read_write.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "dirent.h"
#include "unistd.h"
#include "stdlib.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE	int		alloc_bit  (int dev);
PRIVATE void	alloc_sect (int dev, zone_t * sector_ptr );
PRIVATE int		alloc_zone (int dev, int alloc_start, int alloc_end, zone_t * i_zone);
PRIVATE int		rdwt_zones (int pos, int len, int src, char * buf, struct inode * pin, int mode);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  rdwt_zones
 *  Description:  read of write the buf to the zones
 *  @param pos:	  the current position in the file
 *  @param buf_len:		the length of the buf
 *  @param src:			the source of the buf
 *  @param buf:			the buf should be read or written
 *  @param i_nr_sects:	the current amount of the sects of the file
 *  @param i_zone:		the i_zone of this inode
 *  @param mode:		READ or WRITE
 *  @return :			how many bytes writed or read
 * =====================================================================================
 */
PRIVATE int
rdwt_zones ( int pos, int len, int src, char * buf, struct inode * pin, int mode)
{
	//check if the mode is right
	assert(WRITE == mode || READ == mode);

	int i;
	int count = 0;
	int pos_end = pos + len;
	//how many sects does the buf need to take
	int nr_sect_start = pos >> SECTOR_SIZE_SHIFT;
	int nr_sect_end = pos_end >> SECTOR_SIZE_SHIFT;
	int off = pos_end % SECTOR_SIZE;
	if (off != 0) {
		nr_sect_end++;
	}

	//if the sects in the file is not enough
	if (nr_sect_end > pin->i_nr_sects) {
		//allocate the sectors that needed
		u32 zone_amount = alloc_zone(pin->i_dev, pin->i_nr_sects, nr_sect_end, pin->i_zone);
		assert((nr_sect_end - pin->i_nr_sects + 1) == zone_amount);
		pin->i_nr_sects += zone_amount;
	}

	if (nr_sect_start < NR_DIRECT_ZONE) {
		int direct_end = NR_DIRECT_ZONE - 1;
		//the sectors are only in the direct zone
		int flag_direct_zone = 0;
		if (nr_sect_end <= direct_end) {
			//the amount of sects is less than the direct zone
			flag_direct_zone = 1;
			direct_end = nr_sect_end;
		}
		for (i = nr_sect_start; i <= direct_end; i++, count++) {
			if (WRITE == mode) {
  				phys_copy((void*)va2la(TASK_FS, fsbuf),
						(void*)va2la(src, buf + count * SECTOR_SIZE),
						SECTOR_SIZE);
				WR_SECT(pin->i_dev, pin->i_zone[i]);
			} else if (READ == mode) {
				RD_SECT(pin->i_dev, pin->i_zone[i]);
				phys_copy((void*)va2la(src, buf + count * SECTOR_SIZE),
						(void*)va2la(TASK_FS, fsbuf),
						SECTOR_SIZE);
			}
		}
		if (flag_direct_zone) {
			return (nr_sect_end - nr_sect_start) 
				* SECTOR_SIZE
				+ off;
		}
	}

	//the first level indirect zone
	zone_t sectors[NR_ZONE_PER_SECT];

	if (nr_sect_start < NR_SECOND_LEVEL_ZONE) {
		int second_level_start = NR_DIRECT_ZONE;
		int second_level_end = NR_SECOND_LEVEL_ZONE - 1;
		//the sectors are only reach the second level
		int flag_second_level = 0;
		if (nr_sect_end <= second_level_end) {
			//the amount of sects is less than the second level zone
			flag_second_level = 1;
			second_level_end = nr_sect_end;
		}
		if (nr_sect_start > second_level_start) {
			second_level_start = nr_sect_start;
		}

		RD_SECT(pin->i_dev, pin->i_zone[SECOND_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_SECOND_LEVEL_ZONE * sizeof(int) );

		for (i = second_level_start; i <= second_level_end; i++, count++) {
            if (WRITE == mode) {
                phys_copy((void*)va2la(TASK_FS, fsbuf),
                        (void*)va2la(src, buf + count * SECTOR_SIZE),
                        SECTOR_SIZE);
                WR_SECT(pin->i_dev, sectors[i]);
            } else if(READ == mode) {
                RD_SECT(pin->i_dev, sectors[i]);
                phys_copy((void*)va2la(src, buf + count * SECTOR_SIZE),
                        (void*)va2la(TASK_FS, fsbuf),
                        SECTOR_SIZE);
            }

		}

		//if the zones only reach second level
		if (flag_second_level) {
			return (nr_sect_end - nr_sect_start)
				* SECTOR_SIZE
				+ off;
		}
	} 

	if (nr_sect_end < NR_THIRD_LEVEL_ZONE) {
		//less than the third level indirect zone
		int third_level_start = NR_SECOND_LEVEL_ZONE;
		int third_level_end = NR_THIRD_LEVEL_ZONE - 1;
		//the sectors are only reach the third level
		int flag_third_level = 0;
		if (nr_sect_end <= third_level_end) {
			//the amount of sects is less than the third level zone
			flag_third_level = 1;
			third_level_end = nr_sect_end;
		}
		if (nr_sect_start > third_level_start) {
			third_level_start = nr_sect_start;
		}

		RD_SECT(pin->i_dev, pin->i_zone[THIRD_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_ZONE_PER_SECT * sizeof(int) );

		int current_second_level_zone = NO_ZONE;
		zone_t current_zones[NR_ZONE_PER_SECT];
		for (i = third_level_start; i <= third_level_end; i++, count++) {
			int second_level_index	=
				(i - NR_SECOND_LEVEL_ZONE) / NR_SECT_PER_SECOND_LEVEL_ZONE;
			int third_level_index = 
				(i - NR_SECOND_LEVEL_ZONE) % NR_SECT_PER_SECOND_LEVEL_ZONE;

			if (NO_ZONE == current_second_level_zone
					|| current_second_level_zone != second_level_index){
				RD_SECT(pin->i_dev, sectors[second_level_index]);
				memcpy((void *)current_zones,
						(void *)fsbuf,
						NR_ZONE_PER_SECT * sizeof(int) );
			}
            if (WRITE == mode) {
                phys_copy((void*)va2la(TASK_FS, fsbuf),
                        (void*)va2la(src, buf + count * SECTOR_SIZE),
                        SECTOR_SIZE);
                WR_SECT(pin->i_dev, current_zones[third_level_index]);
            } else if(READ == mode) {
                RD_SECT(pin->i_dev, current_zones[third_level_index]);
                phys_copy((void*)va2la(src, buf + count * SECTOR_SIZE),
                        (void*)va2la(TASK_FS, fsbuf),
                        SECTOR_SIZE);
            }

		}

		//if the zones only reach third level
		if (flag_third_level) {
			return (nr_sect_end - nr_sect_start)
				* SECTOR_SIZE
				+ off;
		}
	}

	if (nr_sect_end >= NR_THIRD_LEVEL_ZONE) {
		//do not support zones larger than third level
		panic("buf is too large");
	}

	return -1;
}		/* -----  end of function write_to_zones  ----- */

/*****************************************************************************
 *                                do_rdwt
 *****************************************************************************/
/**
 * Read/Write file and return byte count read/written.
 *
 * Sector map is not needed to update, since the sectors for the file have been
 * allocated and the bits are set when the file was created.
 * 
 * @return How many bytes have been read/written.
 *****************************************************************************/
PUBLIC int do_rdwt()
{
	int fd = fs_msg.FD;	/**< file descriptor. */
	void * buf = fs_msg.BUF;/**< r/w buffer */
	int len = fs_msg.CNT;	/**< r/w bytes */

	int src = fs_msg.source;		/* caller proc nr. */

	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
		return 0;

	int pos = pcaller->filp[fd]->fd_pos;

	struct inode * pin = pcaller->filp[fd]->fd_inode;

	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

	int imode = pin->i_mode & I_TYPE_MASK;

	if (imode == I_CHAR_SPECIAL) {
		int t = fs_msg.type == READ ? DEV_READ : DEV_WRITE;
		fs_msg.type = t;

		int dev = pin->i_zone[DEV_ZONE];
		assert(MAJOR(dev) == 4);

		fs_msg.DEVICE	= MINOR(dev);
		fs_msg.BUF	= buf;
		fs_msg.CNT	= len;
		fs_msg.PROC_NR	= src;
		assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
		send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &fs_msg);
		assert(fs_msg.CNT == len);

		return fs_msg.CNT;
	}
	else {
		assert(pin->i_mode == I_REGULAR || pin->i_mode == I_DIRECTORY);
		assert((fs_msg.type == READ) || (fs_msg.type == WRITE));

//		int pos_end;
//		if (fs_msg.type == READ)
//			pos_end = min(pos + len, pin->i_size);
//		else		/* WRITE */
//			pos_end = min(pos + len, pin->i_nr_sects * SECTOR_SIZE);
//
//		int off = pos % SECTOR_SIZE;
//		int rw_sect_min=pin->i_start_sect+(pos>>SECTOR_SIZE_SHIFT);
//		int rw_sect_max=pin->i_start_sect+(pos_end>>SECTOR_SIZE_SHIFT);
//
//		int chunk = min(rw_sect_max - rw_sect_min + 1,
//				FSBUF_SIZE >> SECTOR_SIZE_SHIFT);
//
//		int bytes_rw = 0;
//		int bytes_left = len;
//		int i;
//		for (i = rw_sect_min; i <= rw_sect_max; i += chunk) {
//			/* read/write this amount of bytes every time */
//			int bytes = min(bytes_left, chunk * SECTOR_SIZE - off);
//			rw_sector(DEV_READ,
//				  pin->i_dev,
//				  i * SECTOR_SIZE,
//				  chunk * SECTOR_SIZE,
//				  TASK_FS,
//				  fsbuf);
//
//			if (fs_msg.type == READ) {
//				phys_copy((void*)va2la(src, buf + bytes_rw),
//					  (void*)va2la(TASK_FS, fsbuf + off),
//					  bytes);
//			}
//			else {	/* WRITE */
//				phys_copy((void*)va2la(TASK_FS, fsbuf + off),
//					  (void*)va2la(src, buf + bytes_rw),
//					  bytes);
//				rw_sector(DEV_WRITE,
//					  pin->i_dev,
//					  i * SECTOR_SIZE,
//					  chunk * SECTOR_SIZE,
//					  TASK_FS,
//					  fsbuf);
//			}
//			off = 0;
//			bytes_rw += bytes;
//			pcaller->filp[fd]->fd_pos += bytes;
//			bytes_left -= bytes;
//		}
//
		//for test
		int nr_sect = rdwt_zones (pos, len, src, buf, pin, fs_msg.type);

		if (pcaller->filp[fd]->fd_pos > pin->i_size) {
			/* update inode::size */
			pin->i_size = pcaller->filp[fd]->fd_pos;
			/* write the updated i-node back to disk */
			sync_inode(pin);
		}

//		return bytes_rw;
		return nr_sect;
	}
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  alloc_sect
 *  Description:  allocate a sector for a file
 *  @param sector_ptr : the slot to remember the sector
 * =====================================================================================
 */
PRIVATE	void
alloc_sect (int dev, zone_t * sector_ptr )
{
	//the second level zone is not available yet
	int allocated_sect = alloc_bit(dev);
	if (INVALID_SECTOR == allocated_sect) {
		panic("no enough sector");
	}
	*sector_ptr = allocated_sect;
}		/* -----  end of function alloc_sect  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  alloc_zone
 *  Description:  alloc zone
 *  @param dev :  In which device the sector-map is located.
 *  @param alloc_start: the start sect of the buf
 *  @param alloc_end:	the end sect of the buf
 *  @param i_zone: the	zone of the inode
 *	@return:			the amount of allocated zones
 * =====================================================================================
 */
PRIVATE	int
alloc_zone (int dev, int alloc_start, int alloc_end, zone_t * i_zone)
{
	int i;
	int flag_no_zone = 0;

	if (alloc_start < NR_DIRECT_ZONE) {
		int direct_end = NR_DIRECT_ZONE - 1;
		//the sectors are only in the direct zone
		int flag_direct_zone = 0;
		if (alloc_end <= direct_end) {
			//the amount of sects is less than the direct zone
			flag_direct_zone = 1;
			direct_end = alloc_end;
		}
		for (i = alloc_start; i <= direct_end; i++) {
			if (NO_ZONE == i_zone[i]) {
				alloc_sect(dev, &i_zone[i]);
			}
		}
		if (flag_direct_zone) {
			return alloc_end - alloc_start + 1;
		}
	}

	//the first level indirect zone
	zone_t sectors[NR_ZONE_PER_SECT];

	if (alloc_start < NR_SECOND_LEVEL_ZONE) {
		int second_level_start = NR_DIRECT_ZONE;
		int second_level_end = NR_SECOND_LEVEL_ZONE - 1;
		//the sectors are only reach the second level
		int flag_second_level = 0;
		if (alloc_end <= second_level_end) {
			//the amount of sects is less than the second level zone
			flag_second_level = 1;
			second_level_end = alloc_end;
		}
		if (alloc_start > second_level_start) {
			second_level_start = alloc_start;
		}

		//if the inode has second level zone
		flag_no_zone = 0;
		if (NO_ZONE == i_zone[SECOND_LEVEL_ZONE]) {
			//the second level zone is not available yet
			alloc_sect(dev, &i_zone[SECOND_LEVEL_ZONE]);
			flag_no_zone = 1;
		}

		RD_SECT(dev, i_zone[SECOND_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_SECOND_LEVEL_ZONE * sizeof(int) );

		if (flag_no_zone) {
			for (i = 0; i < NR_SECOND_LEVEL_ZONE; i++) {
				sectors[i] = NO_ZONE;
			}
		}

		for (i = second_level_start; i <= second_level_end; i++) {
			if (NO_ZONE == i_zone[i]) {
				alloc_sect(dev, &sectors[i]);
			}
		}

		//sync to disk
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_SECOND_LEVEL_ZONE * sizeof(int) );
		WR_SECT(dev, i_zone[SECOND_LEVEL_ZONE]);

		//if the zones only reach second level
		if (flag_second_level) {
			return alloc_end - alloc_start;
		}
	} 

	if (alloc_end < NR_THIRD_LEVEL_ZONE) {
		//less than the third level indirect zone
		int third_level_start = NR_SECOND_LEVEL_ZONE;
		int third_level_end = NR_THIRD_LEVEL_ZONE - 1;
		//the sectors are only reach the third level
		int flag_third_level = 0;
		if (alloc_end <= third_level_end) {
			//the amount of sects is less than the third level zone
			flag_third_level = 1;
			third_level_end = alloc_end;
		}
		if (alloc_start > third_level_start) {
			third_level_start = alloc_start;
		}

		//if the inode has third level zone
		flag_no_zone = 0;
		if (NO_ZONE == i_zone[THIRD_LEVEL_ZONE]) {
			//the third level zone is not available yet
			alloc_sect(dev, &i_zone[THIRD_LEVEL_ZONE]);
			flag_no_zone = 1;
		}

		RD_SECT(dev, i_zone[THIRD_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_ZONE_PER_SECT * sizeof(int) );

		if (flag_no_zone) {
			for (i = 0; i < NR_ZONE_PER_SECT; i++) {
				sectors[i] = NO_ZONE;
			}
		}

		int current_second_level_zone = NO_ZONE;
		zone_t current_zones[NR_ZONE_PER_SECT];
		for (i = third_level_start; i <= third_level_end; i++) {
			int second_level_index	=
				(i - NR_SECOND_LEVEL_ZONE) / NR_SECT_PER_SECOND_LEVEL_ZONE;
			int third_level_index = 
				(i - NR_SECOND_LEVEL_ZONE) % NR_SECT_PER_SECOND_LEVEL_ZONE;

			if (NO_ZONE == current_second_level_zone
					|| current_second_level_zone != second_level_index){

				if (NO_ZONE != current_second_level_zone) {
					//the previous zone is done for writing
					//sync to disk
					memcpy((void *)current_zones,
							(void *)fsbuf,
							NR_ZONE_PER_SECT * sizeof(int) );
					WR_SECT(dev, sectors[second_level_index]);
				}

				//the second level zone is a slot
				flag_no_zone = 0;
				if (NO_ZONE == sectors[second_level_index]) {
					flag_no_zone = 1;
					alloc_sect(dev, &sectors[second_level_index]);
				}
				RD_SECT(dev, sectors[second_level_index]);
				memcpy((void *)current_zones,
						(void *)fsbuf,
						NR_ZONE_PER_SECT * sizeof(int) );
				if (flag_no_zone) {
					for (i = 0; i < NR_ZONE_PER_SECT; i++) {
						current_zones[i] = NO_ZONE;
					}
				}
			}
			if (NO_ZONE == current_zones[third_level_index]) {
				alloc_sect(dev, &sectors[i]);
			}
		}

		//sync to disk
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_ZONE_PER_SECT * sizeof(int) );
		WR_SECT(dev, i_zone[THIRD_LEVEL_ZONE]);

		//if the zones only reach third level
		if (flag_third_level) {
			return alloc_end - alloc_start;
		}
		
	}

	if (alloc_end >= NR_THIRD_LEVEL_ZONE) {
		//do not support zones larger than third level
		panic("file is too large");
	}

	return -1;
}		/* -----  end of function alloc_zone  ----- */

/*****************************************************************************
 *                                alloc_bit
 *****************************************************************************/
/**
 * Allocate a bit in sector-map.
 * 
 * @param dev  In which device the sector-map is located.
 * @param nr_sects_to_alloc  How many sectors are allocated.
 * @param i_zone the zone of the inode
 * 
 * @return  The 1st sector nr allocated.
 *****************************************************************************/
PRIVATE int alloc_bit(int dev)
{
	int i; /* sector index */
	int j; /* byte index */
	int k; /* bit index */

	struct super_block * sb = get_super_block(dev);

	int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
	int flag_bit_alloc = 0;
	int bit_alloc = 0;
	for (i = 0; i < sb->nr_smap_sects; i++) { /* smap_blk0_nr + i :
						     current sect nr. */
		RD_SECT(dev, smap_blk0_nr + i);

		/* byte offset in current sect */
		for (j = 0; j < SECTOR_SIZE ; j++) {
			/* loop until a free bit is found */
			if (fsbuf[j] != 0xFF) {
				k = 0;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}

				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				flag_bit_alloc = 1;
				bit_alloc = i * SECTOR_SIZE + j * 8 + k;
				break;
			}
		}

		if (flag_bit_alloc) {
			/* free bit found, write the bits to smap */
			WR_SECT(dev, smap_blk0_nr + i);
			break;
		}
	}

	assert(flag_bit_alloc == 1);

	return bit_alloc;
}
