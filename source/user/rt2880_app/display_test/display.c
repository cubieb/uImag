#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
static unsigned int use_double_addr = 0;
#define RGB565_TO_ARGB8888(x)   \
    ((((x) &   0x1F) << 3) |    \
     (((x) &  0x7E0) << 5) |    \
     (((x) & 0xF800) << 8) |    \
     (0xFF << 24)) // opaque
     
unsigned char *fbbuf = NULL;
	// use for nmap framebuffer 
static unsigned int *fb_addr = NULL;


// use double fb address
static unsigned int *front_fb_addr = NULL;
static unsigned int *back_fb_addr = NULL;
void senddata(int shift){
		int i =0;
   //***anim_fb_addr_switch**/         
  	  if(use_double_addr == 0) {
        use_double_addr++;
        fb_addr = front_fb_addr;
    } else {
        use_double_addr = 0;
        fb_addr = back_fb_addr;
    }          
            
            
//fill framebuffer
//printf("*****fill framebuffer2*******\n");
	//fread(fb_mem, 720*1280*3, 1, RGB565_TO_ARGB8888(ScreenBitmap));

	for (i=0;  i<720*1280 ;i++){
		*(fb_addr+i) = RGB565_TO_ARGB8888(0x000 + shift);
		//*(fb_addr+i) = shift;
 }

	//if ((fbbuf = mmap(0, vfbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0)) == (void *)-1) {return -3;}


	
}
int main(void)
{
	struct fb_var_screeninfo vinfo;
	static struct fb_fix_screeninfo finfo;
	FILE *ScreenBitmap = fopen("/etc_ro/hd720_battery.bmp", "r");
	char *fb_mem = 0;
	int fbfd = -1;
	int fbsize = 0;
	int vfbsize = 0;

	int j =0;

	if((fbfd = open("/dev/fb0", O_RDWR)) < 0) {
		printf("open fail\n") ;
		return -1; 
		}
	ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
  ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);	
		
	//fbsize = vinfo.xres * vinfo.yres *(vinfo.bits_per_pixel / 8);	
	fbsize  = finfo.line_length * vinfo.yres;
	vfbsize = vinfo.xres_virtual * vinfo.yres_virtual *(vinfo.bits_per_pixel / 8);
  front_fb_addr =(unsigned int*)mmap(0, fbsize*2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  back_fb_addr = (unsigned int*)((unsigned int)front_fb_addr + fbsize);
  fb_addr = front_fb_addr;	
	printf("[Harry Display %s %d]vinfo.xres  = %d, vinfo.yres = %d, vinfo.xres_virtual =%d, fb_size =%d, fb_addr = %d,back_fb_addr=%d\n"
            ,__FUNCTION__, __LINE__, vinfo.xres,vinfo.yres, vinfo.xres_virtual, fbsize,( int)fb_addr, (int)back_fb_addr);
            
while(1){
	j= j+1;
	senddata(j);
		//printf("*****trigger HW update*******\n");
//**********trigger HW update
	  if (fb_addr == back_fb_addr) {
        vinfo.yoffset = vinfo.yres;
    } else {
        vinfo.yoffset = 0;
    }
    
    vinfo.activate |= (FB_ACTIVATE_FORCE | FB_ACTIVATE_NOW);
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
    {
        printf("*******ioctl FBIOPUT_VSCREENINFO flip failed************\n");
    }	
}
    

	//if (munmap(fbbuf, vfbsize)) {return -6;}
	if (close(fbfd) == -1) {return -7;}
	return 0;
}