/*************************************************************************
  @Author: Jason
  @Created Time : Mon 21 Oct 2019 11:54:23 PM CST
  @File Name: key_ctrl.c
  @Description:
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* seconddrvtest 
 */
int main(int argc, char **argv)
{
	int fd;
	unsigned char key_val;
	int cnt = 0;

	fd = open("/dev/keys", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	while (1)
	{
		read(fd, &key_val, 1);
		printf("key_val =  %x\n ",key_val);
	}

	return 0;
}


