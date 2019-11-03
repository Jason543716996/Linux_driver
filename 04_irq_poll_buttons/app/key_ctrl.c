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
/* seconddrvtest 
 */
int main(int argc, char **argv)
{
	int fd;
	unsigned char key_val;
	int ret = 0;
	
	struct pollfd fds[1];
	fd = open("/dev/keys", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	fds[0].fd = fd;
	fds[0].events = POLLIN;
	while (1)
	{
		ret = poll(fds,1,5000);
		if(ret == 0){
			printf("time out\n");
		}else{
			read(fd, &key_val, 1);
			printf("key_val =  %x\n ",key_val);
		}
	}

	return 0;
}


