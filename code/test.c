#include "utils.h"
#include "nvram.h"

int main()
{

}

int change_hostname(char *hostname, char *macaddr)
{
	if(hostname == NULL || macaddr == NULL)
		return -1; 

	//获取nvram里面的client_stainfo数据
	char client_info[MAX_MSG_SIZ];
	strcpy(client_info, nvram_bufget(RT2860_NVRAM, "client_info"));

	//判断是否为空
	if(strcmp(client_info, "") == 0)
	{
		//为空 -->添加数据
		nvram_bufset(RT2860_NVRAM, "client_info", hostname:mac);
	}
	else
	{
	//不为空 --> 寻找数据是否已记录，否-->添加  是-->修改hostname
		
	}

	char buff[2048]; //获取单个设备的信息到buff中

	char *tmp = macaddr;

	char *file_mac = NULL;

	char *myip = NULL;
	char temp[18]; //record the ip data
	char *msg_os = NULL;

	while(fgets(buff, 2048, stream) > 0)
	{   
		int length = strlen(buff);
		file_mac = web_get("mac", buff, 0);

		DBG_MSG("file_mac is %s", file_mac);

		//find the mac the same as stream mac
		if(strcmp(file_mac, tmp) == 0)
		{
			DBG_MSG(" %d %s", __LINE__, __FILE__);
			myip = web_get("ip", buff, 2);
			strcpy(temp, myip);

			msg_os = web_get("msg_os", buff, 0);

			//clear a line
			memset(buff, 0, length);
			//fputs(buff, stream);
			fwrite(buff, 1, length, stream);
			fseek(stream, -length, SEEK_CUR);

			//write a line
			snprintf(buff, length, "hostname=%s&mac=%s&ip=%s&msg_os=%s", hostname, macaddr, temp, msg_os);
			fseek(stream, -length, SEEK_CUR);
			fwrite(buff,1, length, stream);

			return 0;
		}

	}

}
