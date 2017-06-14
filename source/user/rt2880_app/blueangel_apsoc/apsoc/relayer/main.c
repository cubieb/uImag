#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "bt_relayer.h"

static int start_fd = -1;

static int read_data_from_pc(int fd, unsigned char *buffer, unsigned long len)
{
    int bytesRead = 0;
    int bytesToRead = len;
    
    int ret = 0;
    struct timeval tv;
    fd_set readfd;
    
    tv.tv_sec = 5;  //SECOND
    tv.tv_usec = 0;   //USECOND
    FD_ZERO(&readfd);
    
printf("fd=%d\n",fd);
    if (fd < 0) 
        return -1;
    
    // Hope to receive len bytes
    while (bytesToRead > 0){
    
        FD_SET(fd, &readfd);
        ret = select(fd + 1, &readfd, NULL, NULL, &tv);
        
        if (ret > 0){
            bytesRead = read(fd, buffer, bytesToRead);
            if (bytesRead < 0){
printf("bytesRead<0\n",fd);
                if(errno == EINTR || errno == EAGAIN)
                    continue;
                else
                    return -1;
            }
            else{
printf("fd=%d\n",fd);
                bytesToRead -= bytesRead;
                buffer += bytesRead;
            }
        }
        else if (ret == 0){
            return -1; // read com port timeout 5000ms
        }
        else if ((ret == -1) && (errno == EINTR)){
            continue;
        }
        else{
            return -1;
        }
    }

    return 0;
}

static int waiting_start(char *str)
{
	unsigned char ucTxBuf[32];
	int count;

	DBG("Waiting start signal[%s]\n", str);

	while(1){
		memset(ucTxBuf, 0, sizeof(ucTxBuf));

		// Receive HCI packet from PC
		count = 0;
		while(read_data_from_pc(start_fd, ucTxBuf, strlen(str)) < 0) {
			count++;
			if(count > 3)
				break;
		}

		//printf("count=%d\n", count);
		if((count <= 3) && (!strcmp((char *)ucTxBuf,str))){
			DBG("Receive start signal from PC\n");
			break;
		}
		sleep(1);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	char tmp;
	char buf[7];

	if((argc  == 2) && (!strcmp(argv[1], "wait"))){
		start_fd = init_serial(0 , 115200);
		if (start_fd < 0){
			ERR("Initialize serial port to PC failed\n");
			return -1;
		}

		waiting_start("#START%");
	}

	


	if(RELAYER_start(0, 115200))
		DBG("RELAYER start successfully\n");
	else
		ERR("RELAYER start failed\n");


	/* Loop in waiting for user type "exit" */
	i = 0;
	do {
		if (i >= 5)
			i = 0; /*rollback*/

		tmp = getchar();
		buf[i] = tmp;

		if (tmp != '\r' && tmp != '\n') {
			i ++;
		}
		else {
			buf[i] = '\0';
			if (0 != strcmp(buf, "exit"))
				i = 0; /*discard this string*/
			else
				break;
		}
	} while(1);

	RELAYER_exit();

	return 0;
}
