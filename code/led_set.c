#include "utils.h"
#include "nvram.h"

/*
   led灯设置（总是最后一次设置有效）
   led灯常开模式（默认常开）：点亮灯，修改nvram里面的settime为空
   led灯常关模式：关灯，修改nvram里面的settime为空
   led灯某个时间段关灯：获取web客户端的时间，写到nvram里面的settime变量, eg: settime=22:00-8:00
   22:00为开始时间，8:00为截止时间。
*/

int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;
	length = get_message_for_web(input);
	if(length <=0)
	{
		DBG_MSG("get the message from web is empty");
		return -1;
	}

	char *temp;
	temp = web_get("led_on", input, 0);
	if(strcmp("on", temp) == 0)
	{
		//led on
		nvram_bufset(RT2860_NVRAM, "led_off", 0);

		//开始时间和截止时间都为空
		//nvram_bufset(RT2860_NVRAM, "led_begin", "");
		//nvram_bufset(RT2860_NVRAM, "led_end", "");
		nvram_commit(RT2860_NVRAM);
	}

	temp = web_get("led_off", input, 0);
	if(strcmp("off", temp) == 0)
	{
		//led off
		nvram_bufset(RT2860_NVRAM, "led_off", 1);

		//nvram_bufset(RT2860_NVRAM, "led_begin", "");
		//nvram_bufset(RT2860_NVRAM, "led_end", "");
		nvram_commit(RT2860_NVRAM);
	}

	temp = web_get("led_set", input, 0);
	if(strcmp("set", temp) == 0)
	{
		char settime[12];  //保存时间值，格式：xx:xx-xx:xx
		//led set
		char begintime[6];
		char endtime[6];
		strcpy(begintime, web_get("BegTime", input, 2));
		get_nth_value(0, begintime, ":", begintime, sizeof(begintime));
		strcpy(endtime, web_get("EndTime", input, 2));
		get_nth_value(0, endtime, ":", endtime, sizeof(endtime));

		DBG_MSG("start time is %s, enditime is %s", begintime, enditime);

		nvram_bufset(RT2860_NVRAM, "led_off", 0);
		nvram_bufset(RT2860_NVRAM, "led_begin", begintime);
		nvram_bufset(RT2860_NVRAM, "led_end", enditime);

		nvram_commit(RT2860_NVRAM);
	}

	return 0;
}
