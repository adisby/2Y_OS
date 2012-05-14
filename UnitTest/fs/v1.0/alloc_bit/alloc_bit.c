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
#include	"alloc_bit.h"

char hd[SECTOR_SIZE * 10];

char fsbuf[SECTOR_SIZE];

struct super_block super_block[NR_SUPER_BLOCK] = {{273, 4096, 40257,  1, 
     10,  525,  512,  1, 
     64, 4, 12,  16, 
     0,  4,  800}};

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
	char a;
	while (scanf("%c", &a)) {
		int i,j;
		switch(a) {
			case 'a':
				printf("bit allocated: %d\n", alloc_bit(800));
				break;
			case 'b':
				printf("what bits do you want to set\nfrom [from] [size]\n");
				scanf("%d %d", &i, &j);
				memset(hd + i, (char)0xFF, j);
				break;
		}
	}
	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
