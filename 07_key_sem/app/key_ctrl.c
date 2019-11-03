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
#include <poll.h>
#include <signal.h>
int fd;


int main(int argc, char **argv)
{
	unsigned char key_val;
	int ret;
	int Oflags;
	
	
	
	fd = open("/dev/keys", O_RDWR|O_NONBLOCK);
	if (fd < 0)
	{
		printf("can't open!\n");
		return -1;
	}

	while (1)
	{
		ret = read(fd,&key_val,1);
		printf("key_val :0x%x,ret = %d\n",key_val,ret);
		sleep(1);
	}

	return 0;
}


