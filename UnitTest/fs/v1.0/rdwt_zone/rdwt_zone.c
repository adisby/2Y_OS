/*
 * =====================================================================================
 *
 *       Filename:  alloc_bit.c
 *
 *    Description:  unit test alloc bit in fs
 *
 *        Version:  1.0
 *        Created:  2012年03月25日 23时19分17秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *			state:	verified
 * =====================================================================================
 */

#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"const.h"
#include	"type.h"
#include	"fs.h"
#include	"rdwt_zone.h"

char hd[SECTOR_SIZE * 1000];

char fsbuf[SECTOR_SIZE];

char file[SECTOR_SIZE * 521];

struct inode pin = {0, 0, 0, {0}, {0}, 800, 0, 0};

struct super_block super_block[NR_SUPER_BLOCK] = {{273, 4096, 40257,  1, 
     10,  525,  512,  1, 
     64, 4, 12,  16, 
     0,  4,  800}};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  va2la
 *  Description:  
 * =====================================================================================
 */
	void *
va2la (int i, void * buf)
{
	return buf;
}		/* -----  end of function va2la  ----- */

PUBLIC struct super_block * get_super_block(int dev)
{
	struct super_block * sb = super_block;
	for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
		if (sb->sb_dev == dev)
			return sb;

//	panic("super block of devie %d not found.\n", dev);

	return 0;
}

PUBLIC void RD_SECT(int dev, int sect_nr)
{
	memcpy(fsbuf,
			hd + sect_nr * SECTOR_SIZE,
			SECTOR_SIZE);
}

PUBLIC void WR_SECT(int dev, int sect_nr)
{
	memcpy(hd + sect_nr * SECTOR_SIZE,
			fsbuf,
			SECTOR_SIZE);
}

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
			if (fsbuf[j] != (char)0xFF) {
				k = 0;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}

//				assert(((fsbuf[j] >> k) & 1) == 0);
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

//	assert(flag_bit_alloc == 1);

	return bit_alloc;
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
//		panic("no enough sector");
	}
	*sector_ptr = allocated_sect;
}		/* -----  end of function alloc_sect  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  alloc_zone
 *  Description:  alloc zone
 *  @param dev :  In which device the sector-map is located.
 *  @param alloc_start: the start sect of the buf
 *  @param alloc_nr:	the amount of the sect of the buf
 *  @param i_zone: the	zone of the inode
 *	@return:			the amount of allocated zones
 * =====================================================================================
 */
PRIVATE	int
alloc_zone (int dev, int alloc_start, int alloc_nr, zone_t * i_zone)
{
	int i;
	int flag_no_zone = 0;
	int nr_sect = alloc_start + alloc_nr;

	if (alloc_start < NR_DIRECT_ZONE) {
		//the sectors are only in the direct zone
		int flag_direct_zone = 1;

		//asumpt the amount of the sect is less than direct zone
		int direct_alloc_nr = alloc_nr;
		if (nr_sect > NR_DIRECT_ZONE) {
			//the amount of sects is less than the direct zone
			flag_direct_zone = 0;
			direct_alloc_nr = NR_DIRECT_ZONE - alloc_start;
			alloc_nr -= direct_alloc_nr;
		}
		for (i = 0; i < direct_alloc_nr; i++) {
			if (NO_ZONE == i_zone[alloc_start + i]) {
				alloc_sect(dev, &i_zone[alloc_start + i]);
			}
		}
		if (flag_direct_zone) {
			return nr_sect - alloc_start;
		}
	}

	//the first level indirect zone
	zone_t sectors[NR_ZONE_PER_SECT];

	if (alloc_start < NR_SECOND_LEVEL_ZONE) {
		int second_level_start = NR_DIRECT_ZONE;
		if (alloc_start > second_level_start) {
			second_level_start = alloc_start;
		}

		//the sectors are only reach the second level
		int flag_second_level = 1;
		int second_alloc_nr = alloc_nr;
		if (nr_sect > NR_SECOND_LEVEL_ZONE) {
			//the amount of sects is less than the second level zone
			flag_second_level = 0;
			second_alloc_nr = NR_SECOND_LEVEL_ZONE - second_level_start;
			alloc_nr -= second_alloc_nr;
		}

		//get the begin and end in second level zone
		second_level_start -= NR_DIRECT_ZONE;

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
				NR_SECT_PER_SECOND_LEVEL_ZONE * sizeof(int) );

		if (flag_no_zone) {
			for (i = 0; i < NR_SECT_PER_SECOND_LEVEL_ZONE; i++) {
				sectors[i] = NO_ZONE;
			}
		}

		for (i = 0; i < second_alloc_nr; i++) {
			if (NO_ZONE == sectors[second_level_start + i]) {
				alloc_sect(dev, &sectors[second_level_start + i]);
			}
		}

		//sync to disk
		memcpy((void *)fsbuf,
				(void *)sectors,
				NR_SECT_PER_SECOND_LEVEL_ZONE * sizeof(int) );
		WR_SECT(dev, i_zone[SECOND_LEVEL_ZONE]);

		//if the zones only reach second level
		if (flag_second_level) {
			return nr_sect - alloc_start;
		}
	} 

	if (alloc_start < NR_THIRD_LEVEL_ZONE) {
		//less than the third level indirect zone
		int third_level_start = NR_SECOND_LEVEL_ZONE;
		if (alloc_start > third_level_start) {
			third_level_start = alloc_start;
		}
		//the sectors are only reach the third level
		int flag_third_level = 1;
		int third_alloc_nr = alloc_nr;
		if (nr_sect > NR_THIRD_LEVEL_ZONE) {
			//the amount of sects is less than the third level zone
			flag_third_level = 0;
			third_alloc_nr = NR_THIRD_LEVEL_ZONE - third_level_start;
			alloc_nr -= third_alloc_nr;
		}

		//get the start and end in third level zone
		third_level_start -= NR_SECOND_LEVEL_ZONE;

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
		for (i = 0; i < third_alloc_nr; i++) {
			int second_level_index	=
				(third_level_start + i) /
				NR_SECT_PER_SECOND_LEVEL_ZONE;
			int third_level_index = 
				(third_level_start + i) % 
				NR_SECT_PER_SECOND_LEVEL_ZONE;

			if (NO_ZONE == current_second_level_zone
					|| current_second_level_zone != second_level_index){

				if (NO_ZONE != current_second_level_zone) {
					//the previous zone is done for writing
					//sync to disk
					memcpy((void *)fsbuf,
							(void *)current_zones,
							NR_ZONE_PER_SECT * sizeof(int) );
					WR_SECT(dev, current_second_level_zone);
				}

				//the second level zone is a slot
				flag_no_zone = 0;
				if (NO_ZONE == sectors[second_level_index]) {
					flag_no_zone = 1;
					alloc_sect(dev, &sectors[second_level_index]);
				}
				current_second_level_zone = sectors[second_level_index];

				RD_SECT(dev, sectors[second_level_index]);
				memcpy((void *)current_zones,
						(void *)fsbuf,
						NR_ZONE_PER_SECT * sizeof(int) );

				//if the second level zone is a slot, get to init
				if (flag_no_zone) {
					for (i = 0; i < NR_ZONE_PER_SECT; i++) {
						current_zones[i] = NO_ZONE;
					}
				}
			}
			if (NO_ZONE == current_zones[third_level_index]) {
				alloc_sect(dev, &current_zones[third_level_index]);
			}
		}

		//sync the rest second level zone to disk
		if (NO_ZONE != current_second_level_zone) {
			//the previous zone is done for writing
			//sync to disk
			memcpy((void *)fsbuf,
					(void *)current_zones,
					NR_ZONE_PER_SECT * sizeof(int) );
			WR_SECT(dev, current_second_level_zone);
		}

		//sync to disk
		memcpy((void *)fsbuf,
				(void *)sectors,
				NR_ZONE_PER_SECT * sizeof(int) );
		WR_SECT(dev, i_zone[THIRD_LEVEL_ZONE]);

		//if the zones only reach third level
		if (flag_third_level) {
			return nr_sect - alloc_start;
		}
		
	}

	if (nr_sect > NR_THIRD_LEVEL_ZONE) {
		//do not support zones larger than third level
//		panic("file is too large");
	}

	return -1;
}		/* -----  end of function alloc_zone  ----- */

PRIVATE int
rdwt_zones ( int pos, int len, int src, char * buf, struct inode * pin, int mode)
{
	//check if the mode is right
//	assert(WRITE == mode || READ == mode);

	int i;
	int count = 0;
	int pos_end = pos + len;
	//how many sects does the buf need to take
	int nr_sect_start = pos >> SECTOR_SIZE_SHIFT;
	int nr_rw_sect = len >> SECTOR_SIZE_SHIFT;
	int off = pos_end % SECTOR_SIZE;

	//if the buf is not n * SECTOR_SIZE, just give one more sector
	if (off > 0) {
		nr_rw_sect ++;
	}

	int nr_sect = nr_sect_start + nr_rw_sect;

	//if the sects in the file is not enough
	if (nr_sect > pin->i_nr_sects) {
		//allocate the sectors that needed
		u32 zone_amount = alloc_zone(pin->i_dev, pin->i_nr_sects, nr_sect - pin->i_nr_sects, pin->i_zone);
//		assert((nr_sect_end - pin->i_nr_sects) == zone_amount);
		pin->i_nr_sects += zone_amount;
	}

	if (nr_sect_start < NR_DIRECT_ZONE) {
		//the sectors are only in the direct zone
		int flag_direct_zone = 1;

		//asumpt the amount of the sect is less than direct zone
		int direct_nr_rw_sect = nr_rw_sect;
		if (nr_sect > NR_DIRECT_ZONE) {
			//the amount of sects is more than the direct zone
			flag_direct_zone = 0;
			direct_nr_rw_sect = NR_DIRECT_ZONE - nr_sect_start;
			nr_rw_sect -= direct_nr_rw_sect;
		}
		for (i = 0; i < direct_nr_rw_sect; i++) {
			if (WRITE == mode) {
  				phys_copy((void*)va2la(TASK_FS, fsbuf),
						(void*)va2la(src, buf + i * SECTOR_SIZE),
						SECTOR_SIZE);
				WR_SECT(pin->i_dev, pin->i_zone[nr_sect_start + i]);
			} else if (READ == mode) {
				RD_SECT(pin->i_dev, pin->i_zone[nr_sect_start + i]);
				phys_copy((void*)va2la(src, buf + i * SECTOR_SIZE),
						(void*)va2la(TASK_FS, fsbuf),
						SECTOR_SIZE);
			}
		}
		if (flag_direct_zone) {
			return off ? ((nr_rw_sect - 1) * SECTOR_SIZE + off)
						:(nr_rw_sect * SECTOR_SIZE);
		}
	}

	//the first level indirect zone
	zone_t sectors[NR_ZONE_PER_SECT];

	if (nr_sect_start < NR_SECOND_LEVEL_ZONE) {
		int second_level_start = NR_DIRECT_ZONE;
		if (nr_sect_start > second_level_start) {
			second_level_start = nr_sect_start;
		}
		//the sectors are only reach the second level
		int flag_second_level = 1;
		int second_nr_rw_sect = nr_rw_sect;
		if (nr_sect > NR_SECOND_LEVEL_ZONE) {
			//the amount of sects is more than the second level zone
			flag_second_level = 0;
			second_nr_rw_sect = NR_SECOND_LEVEL_ZONE - second_level_start;
			nr_rw_sect -= second_nr_rw_sect;
		}

		//get the begin and end in second level zone
		second_level_start -= NR_DIRECT_ZONE;

		RD_SECT(pin->i_dev, pin->i_zone[SECOND_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_ZONE_PER_SECT * sizeof(int) );

		for (i = 0; i < second_nr_rw_sect; i++) {
            if (WRITE == mode) {
                phys_copy((void*)va2la(TASK_FS, fsbuf),
                        (void*)va2la(src, buf + i * SECTOR_SIZE),
                        SECTOR_SIZE);
                WR_SECT(pin->i_dev, sectors[second_level_start + i]);
            } else if(READ == mode) {
                RD_SECT(pin->i_dev, sectors[second_level_start + i]);
                phys_copy((void*)va2la(src, buf + i * SECTOR_SIZE),
                        (void*)va2la(TASK_FS, fsbuf),
                        SECTOR_SIZE);
            }

		}

		//if the zones only reach second level
		if (flag_second_level) {
			return off ? ((nr_rw_sect - 1) * SECTOR_SIZE + off)
						:(nr_rw_sect * SECTOR_SIZE);
		}
	} 

	if (nr_sect_start < NR_THIRD_LEVEL_ZONE) {
		//less than the third level indirect zone
		int third_level_start = NR_SECOND_LEVEL_ZONE;
		if (nr_sect_start > third_level_start) {
			third_level_start = nr_sect_start;
		}
		//the sectors are only reach the third level
		int flag_third_level = 1;
		int third_nr_rw_sect = nr_rw_sect;
		if (nr_sect > NR_THIRD_LEVEL_ZONE) {
			//the amount of sects is more than the third level zone
			flag_third_level = 0;
			third_nr_rw_sect = NR_THIRD_LEVEL_ZONE - third_level_start;
			nr_rw_sect -= third_nr_rw_sect;
		}

		//get the start and end in third level zone
		third_level_start -= NR_SECOND_LEVEL_ZONE;

		RD_SECT(pin->i_dev, pin->i_zone[THIRD_LEVEL_ZONE]);
		memcpy((void *)sectors,
				(void *)fsbuf,
				NR_ZONE_PER_SECT * sizeof(int) );

		int current_second_level_zone = NO_ZONE;
		zone_t current_zones[NR_ZONE_PER_SECT];
		for (i = 0; i < third_nr_rw_sect; i++) {
			int second_level_index	=
				(third_level_start + i) /
				NR_SECT_PER_SECOND_LEVEL_ZONE;
			int third_level_index = 
				(third_level_start + i) %
				NR_SECT_PER_SECOND_LEVEL_ZONE;

			if (NO_ZONE == current_second_level_zone
					|| current_second_level_zone != second_level_index){
				RD_SECT(pin->i_dev, sectors[second_level_index]);
				memcpy((void *)current_zones,
						(void *)fsbuf,
						NR_ZONE_PER_SECT * sizeof(int) );
			}

			//record the current second level zone for compare in next time
			//to comfirm if that the current second level zone is finished
			current_second_level_zone = sectors[second_level_index];

            if (WRITE == mode) {
                phys_copy((void*)va2la(TASK_FS, fsbuf),
                        (void*)va2la(src, buf + i * SECTOR_SIZE),
                        SECTOR_SIZE);
                WR_SECT(pin->i_dev, current_zones[third_level_index]);
            } else if(READ == mode) {
                RD_SECT(pin->i_dev, current_zones[third_level_index]);
                phys_copy((void*)va2la(src, buf + i * SECTOR_SIZE),
                        (void*)va2la(TASK_FS, fsbuf),
                        SECTOR_SIZE);
            }

		}

		//if the zones only reach third level
		if (flag_third_level) {
			return off ? ((nr_rw_sect - 1) * SECTOR_SIZE + off)
						:(nr_rw_sect * SECTOR_SIZE);
		}
	}

	if (nr_sect > NR_THIRD_LEVEL_ZONE) {
		//do not support zones larger than third level
//		panic("buf is too large");
	}

	return -1;
}		/* -----  end of function rdwt_zones  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  the main function
 *	@param argc:  the amount of arguments
 *	@param argv:  the arguments
 *	@return	   :  the exit code
 * =====================================================================================
 */
	int
main ( int argc, char *argv[] )
{
	memset(hd, 0, sizeof(hd));
	memset(file, 'a', sizeof(file));
	char a;
	printf("print a(alloc_bit), b(set bit), c(alloc zone) or d(rdwt zone )\n");
	while (scanf("%c", &a)) {
		int i,j;
		zone_t i_zone[NR_ZONE_PER_INODE] = {NO_ZONE};
		switch(a) {
			case 'a':
				printf("bit allocated: %d\n", alloc_bit(800));
				break;
			case 'b':
				printf("what bits do you want to set\nfrom [from] [size]\n");
				scanf("%d %d", &i, &j);
				memset(hd + i, (char)0xFF, j);
				break;
			case 'c':
				printf("alloc zone\n [alloc_start] [alloc_end]\n");
				scanf("%d %d", &i, &j);
				alloc_zone(800, i, j, i_zone);
				break;
			case 'd':
				printf("rdwt zone\n [pos] [len] \n");
				scanf("%d %d", &i, &j);
				rdwt_zones(i, j, 0, file, &pin, WRITE);
				break;
		}		
		printf("print a(alloc_bit), b(set bit), or c(alloc zone)\n");
	}
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
