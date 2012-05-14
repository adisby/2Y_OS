/*
 * =====================================================================================
 *
 *       Filename:  rm.c
 *
 *    Description:  remove the directory
 *
 *        Version:  1.0
 *        Created:  2012年03月12日 23时58分11秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  adisby (), adisbyPD@gmail.com
 *        Company:  2Y_OS
 *
 * =====================================================================================
 */


#include	"stdio.h"

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
	if (argc < 2) {
		printf("usage : %s path\n", argv[0]);
		return 1;
	}

	unlink(argv[1]);
	return 0;
}				/* ----------  end of function main  ---------- */
