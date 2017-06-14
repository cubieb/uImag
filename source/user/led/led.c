#include <stdio.h>
#include <sys/ioctl.h>

int main()
{

	char long_buf[1024];
	FILE *fp;
	char *str;
	
	system("reg set 0xb0000064");
	system("reg w 0 0x1");

	while(1)
	{
		system("iwpriv apcli0 show connStatus");	
		memset(long_buf, 0, 1024);


		if(!(fp = popen("nvram_get 2860 LinkTest", "r")))
			return 1;

		if(0 == fread(long_buf, 1, 1024, fp))
		{
			printf("fread read none.\n");
			pclose(fp);
                        return 1;
		}
		pclose(fp);

		printf("long_buf = %s\n", long_buf);

		//没有查找到字符串
		if(atoi(long_buf) == 1)
		{
			system("gpio l 44 0 4000 0 0 0");
			system("gpio l 11 4000 0 0 0 0");
			printf("Connect \n");		
		}
		else
		{
			system("gpio l 11 0 4000 0 0 0");
			system("gpio l 44 4000 0 0 0 0");
			printf("Disconnect \n");
		}

		sleep(3);
	}	

	return 0;
}
