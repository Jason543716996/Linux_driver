#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

// ./a.out /dev/scullx  xx.c/xx.txt

int main(int argc, char *argv[])
{
	int fddev, fdfile;
	int ret;
#define SZ 512
	char buf[SZ];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s /dev/scullx file\n", argv[0]);
		exit(1);
	}

	fddev = open(argv[1], O_WRONLY | O_NDELAY);
	assert(fddev >= 0);

	fdfile = open(argv[2], O_RDONLY | O_NDELAY);
	assert(fdfile >= 0);

	ret = read(fdfile, buf, SZ);
	assert(ret >= 0);
	
	if (ret > 0) {
		ret = write(fddev, buf, ret);
		assert(ret >= 0);
	}

	return 0;
}
