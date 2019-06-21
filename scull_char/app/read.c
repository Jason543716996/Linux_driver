#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

// ./a.out /dev/scullx 

int main(int argc, char *argv[])
{
	int fd;
	int ret;
#define SZ 512
	char buf[SZ];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/scullx\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDONLY | O_NDELAY);
	assert(fd >= 0);

	ret = read(fd, buf, SZ-1);
	assert(ret >= 0);
	
	if (ret > 0) {
		buf[ret] = '\0';
		printf("%s", buf);
	}

	return 0;
}
