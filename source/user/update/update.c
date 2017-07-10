#include <stdio.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <stdlib.h>  
#include <netdb.h>  
#include <string.h>  
#include <fcntl.h>  
#include "cJSON.h"

/************************************
****    Macro
*************************************/
#define BUFF_LEN							2048

#define VERSION_MAX_LEN                                16

/* JSONÊý¾ÝTYPE */
#define NAME_JSON_ERROR					"errno"
#define NAME_JSON_UPDATE 					"update"
#define NAME_JSON_UPDATE_URL				"update_url"
#define NAME_JSON_UPDATE_LOG 				"update_log"


/************************************
****    Function
*************************************/
char* get_version(char *pVer)
{

	char long_buf[VERSION_MAX_LEN];
	FILE *fp;

	memset(long_buf, 0, VERSION_MAX_LEN);

	if(!(fp = popen("cat /etc_ro/conf/verson", "r")))
	{		
		return NULL;
	}

	if(0 == fread(long_buf, 1, VERSION_MAX_LEN, fp))
	{
		printf("get_version: fread read none.\n");
		pclose(fp);
             return NULL;
	}
	pclose(fp);

	/* no need char 'v' */
	strcpy(pVer, long_buf + 1);
	return pVer;

}

/* build the HTTP GET packet */
char *build_get_packet(char* pIPAddr , char* pPort , char* pPrjName)
{
	char *pGetPacket = NULL;
	char pVer[VERSION_MAX_LEN];
	
	char *pHttpGetPacket = "GET http://%s:%s/api/%s/%s HTTP/1.1\r\n";
	char *pHost = "Host: %s:%s\r\n";

	memset(pVer, 0, VERSION_MAX_LEN);
	get_version(pVer);

	pGetPacket = (char *)malloc(strlen(pHttpGetPacket) + strlen(pIPAddr) + strlen(pPort) + strlen(pPrjName) + strlen(pVer)
		                                   + strlen(pHost) + strlen(pIPAddr) + strlen(pPort));
	sprintf(pGetPacket, pHttpGetPacket, pIPAddr, pPort, pPrjName, pVer);
	sprintf(pGetPacket + strlen(pGetPacket), pHost, pIPAddr, pPort);
	sprintf(pGetPacket + strlen(pGetPacket), "\r\n\r\n");

	return pGetPacket;  
}

/************************************************
***  example: update 183.230.102.49 1003 wifi-repeator 
***	argv[1]: server address
***  argv[2]: port
***  argv[3]: project name
***********************************************/
int main(int argc,char **argv)
{
	int sockFd;
	struct sockaddr_in *pServer = NULL;
	int count = 0;
	int countTotal = 0;
	char *pGetPacket = NULL;
	char buff[BUFF_LEN];
	char *pData = NULL;
	cJSON *pJSONData = NULL;
	fd_set   t_set1;
	struct timeval  tv;
	int i,h;
	struct hostent *hent;
	char *p;

	int JSONError = 0;
	int JSONUpdate = 0;
	char *pJSONUpdateUrl = NULL;

	int port = atoi(argv[2]);

	/* create client socket */
	if((sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Create TCP socket failed.\n");
		exit(1);
	}

	if((pServer = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in*))) == NULL)
	{
		perror("Malloc sockaddr_in failed.\n");
		exit(1);
	}

	memset(pServer, 0, sizeof(*pServer));  
	pServer->sin_family = AF_INET;
	pServer->sin_port = htons(port); 
	hent = gethostbyname(argv[1]);
	p = inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]);

	if( inet_pton(AF_INET, p, &pServer->sin_addr) <= 0)
	{  
		printf("inet_pton error for %s\n",argv[1]);  
		exit(0);  
	}  

	/* connect server */
	if(connect(sockFd, (struct sockaddr *)pServer, sizeof(struct sockaddr)) < 0)
	{
		perror("Could not connect server.\n");
		exit(1);
	}
	
	/* form http GET packet(for check whether need to upgrade) */
	pGetPacket = build_get_packet(argv[1], argv[2], argv[3]);
	printf("%s\n",pGetPacket);

	/* send request packet */
	while(countTotal < strlen(pGetPacket))
	{
		count=send(sockFd, pGetPacket + countTotal, strlen(pGetPacket) - countTotal, 0);  

		if(count==-1)
		{  
            		perror("Send failed.\n");  
            		exit(1);
		}
		countTotal += count;  
	}
	
	/* receive HTTP response packet */
	memset(buff, 0 , BUFF_LEN);

	FD_ZERO(&t_set1);
	FD_SET(sockFd, &t_set1);
	count = 0;
 
	while(1)
	{
		sleep(2);
		tv.tv_sec= 0;
		tv.tv_usec= 0;
		h= 0;
		h= select(sockFd +1, &t_set1, NULL, NULL, &tv);

		if (h > 0)
		{
			memset(buff, 0, BUFF_LEN);

			if ((count = read(sockFd, buff, BUFF_LEN -1))==0)
			{
				free(pGetPacket);
				free(pServer);	
				close(sockFd);
				printf("server close when read.\n");
				return -1;
			}
			break;
	        }		
		else if (h < 0) 
		{
			free(pGetPacket);
			free(pServer);	
			close(sockFd);
			printf("select abort .\n");
			return -1;
		};
	}

	printf("receive data: \n %s \r\n ", buff);

	/* parse HTTP response packet */
	/* find the response-body */
	if((pData = strstr(buff, "\r\n\r\n")) != NULL)
	{
		/* response-body use JSON format */
		if((pJSONData = cJSON_Parse(pData + 4)) != NULL)
		{
			/* read the value of "errno" */
			if(cJSON_GetObjectItem(pJSONData,  NAME_JSON_ERROR) != NULL)
			{
				JSONError= cJSON_GetObjectItem(pJSONData,  NAME_JSON_ERROR)->valueint;
				
				printf("JSONError = %d\n", JSONError);
			}

			if(0 == JSONError)
			{			
				/* read the value of "update" */
				if(cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE) != NULL)
				{
					JSONUpdate = cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE)->valueint;
					printf("JSONUpdate = %d\n", JSONUpdate);
				}

				/* need to upgrade */
				if(1 == JSONUpdate)
				{
					/* read the value of "update_url" */
					if(cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE_URL) != NULL)
					{
						pJSONUpdateUrl = cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE_URL)->valuestring;

						printf("pJSONUpdateUrl = %s\n", pJSONUpdateUrl);
					}

					/* download image file and rename to uImage */
					memset(buff, 0 , BUFF_LEN);
					sprintf(buff, "wget %s -O uImage", pJSONUpdateUrl);
					system(buff);

					/* write the file to flash(mtd4 is partition of kernel) */				
					system("mtd_write write uImage  /dev/mtd4");
				}
			}
		}		
	}	

	free(pGetPacket);
	free(pServer);	
	close(sockFd);

	return 1;
	
}


