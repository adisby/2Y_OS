#include "type.h"
#include "dirent.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int test1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int test2[10] = {0};

int main(int argc, char * argv[])
{
	char * pathname;
	printf("ls.c argc %d\n", argc);
	if (argc < 2) {
		pathname = ".";
	} else {
		pathname = argv[1];
	}
	DIR * dp;
	struct dirent * dirp;
	dp = opendir(pathname);

	do {
		dirp = readdir(dp);
		printf("%s\n", dirp->d_name);
	} while (NULL != dirp); 

	closedir(dp);
	return 0;
}
