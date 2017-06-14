#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <linux/autoconf.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "i2c_drv.h"
//#include "spi_drv.h"
//#include "spi_lcd_drv.h"
#include "ralink_gpio.h"

#define GPIO_DEV	"/dev/gpio"
#define FB_DEV    "/dev/fb0"
enum {
	gpio_in,
	gpio_out,
};

unsigned long CH;
int fd_i2c, fd_spi;
int count=0;
int firstcount=0;
void open_picture(char *picture_path)
{
	int size, fb;
	char *fb_mem = 0;
	//struct fb_fillrect rect;
	FILE *ScreenBitmap = fopen(picture_path, "r");
	fb = open(FB_DEV, O_RDWR);
	if (fb < 0) printf("open fb device failed\n");

	fb_mem = mmap (NULL, 320*240*3, PROT_READ|PROT_WRITE, MAP_SHARED, fb, 0); 	
		  
	// clear frame buffer
  memset(fb_mem, 0, 320*240*3);
  if ((int)fb_mem == -1) {
  	printf("Error: failed to map framebuffer device to memory.\n");
    return 1;
  }
  //user can cpoy image to framebuffer
	 	fread(fb_mem, 320*240*3, 1, ScreenBitmap);
		if(ioctl(fb, FBIOIMAGEBLIT, NULL)){ //kick off transmit
			printf("ioctl failed\n");
		}

	 	munmap(fb_mem, 240*320*3); //
    close(fb);//
}
void signal_handler(int signum)
{
	int x_array, y_array,i;
	unsigned long temp;
	fd_i2c = open_fd();
 
	ioctl(fd_i2c, Touch_Router_Read_Channel, NULL);	//ACK
	ioctl(fd_i2c, Touch_Router_Get, &CH);
 	x_array =CH&0x0000000f;
	y_array =(CH&0x000f0000)>>16;
		//printf("CH=%x\n",CH);
		//printf("x_array=%d, y_array=%d \n",x_array,y_array);
	if (signum == SIGUSR2){
		count++;
		count=count%2;
		
		x_array =CH&0x0000000f;
		y_array =(CH&0x000f0000)>>16;

		if(count==1){
			if (x_array <0x08 && y_array <0x05 ){
			 open_picture("/etc_ro/A.bmp");
			}
			if (x_array > 0x08 && y_array <0x05 ){
			 open_picture("/etc_ro/G.bmp");
			}
			if (x_array <0x08 && y_array >0x05 && y_array <0x0A){
			 open_picture("/etc_ro/k.bmp");
			}
			if (x_array > 0x08 && y_array >0x05 && y_array <0x0A ){
			 open_picture("/etc_ro/123.bmp");
			}		 
			if (x_array <0x08 && y_array >0x0A ){
			 open_picture("/etc_ro/j.bmp");
			}		 
			if (x_array >0x08 && y_array >0x0A ){
				for(i=0;i<20;i++){
				//while(1){
					open_picture("/etc_ro/A.bmp");		 
			 		open_picture("/etc_ro/G.bmp");
			 	  open_picture("/etc_ro/k.bmp");
			 		open_picture("/etc_ro/123.bmp");
			 		open_picture("/etc_ro/j.bmp");
			 	}
			}
			
		}else if(count==0){
			open_picture("/etc_ro/home.bmp");
		}
	}

 close(fd_i2c);
}
int gpio_reg_info(int gpio_num)
{
	int fd;
	ralink_gpio_reg_info info;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	info.pid = getpid();

	info.irq = gpio_num;

	if (ioctl(fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}


int gpio_dis_irq(void)
{
	int fd;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_enb_irq(void)
{
	int fd;
  
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_ENABLE_INTP) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_set_dir(int r, int dir)
{
	int fd, req;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
#if defined (CONFIG_RALINK_MT7620)
		req = RALINK_GPIO7140_SET_DIR_IN;
		if (ioctl(fd, req, 0x1000) < 0) {
			perror("ioctl");
			close(fd);
		return -1;
	}
#endif
#if defined (CONFIG_RALINK_MT7621)	
	if (dir == gpio_in) {
			req = RALINK_GPIO_SET_DIR_IN;
	}
	else {
			req = RALINK_GPIO_SET_DIR_OUT;
	}
	if (ioctl(fd, req, 0x2000) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
#endif

	close(fd);
	return 0;
}

static void gpio_config(void)
{
	
	int gpio_num=10;
	//enable gpio enterrupt
	gpio_enb_irq();
	//register my information
	gpio_reg_info(gpio_num);
	//issue a handler to handle SIGUSR1
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
while(1){
	//wait for signal
	//printf("waite pause\n");
	pause();
}

	//disable gpio interrupt
	gpio_dis_irq();
}

int open_fd(void)
{
	char nm[16];
	int fd;

	snprintf(nm, 16, "/dev/%s", I2C_DEV_NAME);
	if ((fd = open(nm, O_RDONLY)) < 0) {
		perror(nm);
		exit(fd);
	}
	return fd;
}

int main(int argc, char *argv[])
{
	unsigned long addr, off, val, len, temp;
	int fd;
	struct i2c_write_data wdata;



	switch (argv[1][0]) {
		case 'a':
		case 'T':

	 		fd_i2c = open_fd();
	
			wdata.address = 0x1F;//Touch : Soft Reset
	  	wdata.value = 0xDE;
	  	wdata.size = 0;
	 		ioctl(fd_i2c, Touch_Router_Write, &wdata);
	 		
	 		
		  wdata.address = 0xE0; 	//Touch : Pen Trigger mode
			wdata.value = 0x0;
	  	wdata.size = 0;
    	fd = open_fd();
	 		ioctl(fd_i2c, Touch_Router_Write_CMD, &wdata);
	 	
			open_picture("/etc_ro/home.bmp");
			gpio_config();
			break;
		case 'Y':  //debug use: read touch module register
      fd = open_fd();
      ioctl(fd, Touch_Router_Read, 0x40);
      ioctl(fd, Touch_Router_Get, &temp);
      printf("0x40=%x\n", temp);
      ioctl(fd, Touch_Router_Read, 0x41);
      ioctl(fd, Touch_Router_Get, &temp);
      printf("0x41=%x\n", temp);
      ioctl(fd, Touch_Router_Read, 0x42);
      ioctl(fd, Touch_Router_Get, &temp);
      printf("0x42=%x\n", temp);
      ioctl(fd, Touch_Router_Read, 0x43);
      ioctl(fd, Touch_Router_Get, &temp);
      printf("0x43=%x\n", temp);
      ioctl(fd, Touch_Router_Read, 0x44);
      ioctl(fd, Touch_Router_Get, &temp);
      printf("0x44=%x\n", temp);
			ioctl(fd, Touch_Router_Read, 0x45);
			ioctl(fd, Touch_Router_Get, &temp);
			printf("0x45=%x\n", temp);
			break;
		default:
			printf("\n");
	}
	 
	close(fd);
	return 0;
}
