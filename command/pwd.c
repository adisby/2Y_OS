#include "type.h"
#include "stdio.h"
#include "dirent.h"
#include "stdlib.h"

#define PATH_MAX 255

int main(int argc, char * argv[])
{
	char buf[PATH_MAX];
	char *cwd = getcwd(buf, PATH_MAX);
	if (NULL == cwd) {
		printf("get current directory failed\n");
		return 1;
	}
	printf("%s\n", cwd);
	return 0;
}
