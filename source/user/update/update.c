#include <stdio.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <stdlib.h>  
#include <netdb.h>  
#include <string.h>  
#include <fcntl.h>  
#include <sys/mman.h>
#include <sys/stat.h>

#include "cJSON.h"

/************************************
****    Macro
*************************************/
#define BUFF_LEN							2048

#define VERSION_MAX_LEN                                16

/* JSON DATA TYPE */
#define NAME_JSON_ERROR					"errno"
#define NAME_JSON_UPDATE 					"update"
#define NAME_JSON_UPDATE_URL				"update_url"
#define NAME_JSON_UPDATE_LOG 				"update_log"


#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN	(32-4)		/* Image Name Length		*/

/*
 * all data in network byte order (aka natural aka bigendian)
 */

typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
	uint32_t	ih_ksz;		/* Kernel Part Size		*/
} image_header_t;

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

/*  */
int check(char *imagefile)
{
	struct stat sbuf;

	int  data_len;
	char *data;
	unsigned char *ptr;
	unsigned long checksum;

	image_header_t header;
	image_header_t *hdr = &header;

	int ifd;

	ifd = open(imagefile, O_RDONLY);
	if(!ifd){
		printf ("Can't open %s \n", imagefile);
		close(ifd);
		return 0;
	}

	if (fstat(ifd, &sbuf) < 0) {
		close(ifd);
		printf ("Can't stat %s\n", imagefile);
		return 0;
	}

	ptr = (unsigned char *) mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if ((caddr_t)ptr == (caddr_t)-1) {
		close(ifd);
		printf ("Can't mmap %s\n", imagefile);
		return 0;
	}

	/*
	 *  handle Header CRC32
	 */
	memcpy (hdr, ptr, sizeof(image_header_t));

#if 0
	printf("hdr->ih_magic = %d\n" ,  hdr->ih_magic);
	printf("hdr->ih_hcrc = %d\n" ,  hdr->ih_hcrc);
	printf("hdr->ih_time = %d\n" ,  hdr->ih_time);
	printf("hdr->ih_size = %d\n" ,  hdr->ih_size );
	printf("hdr->ih_load = %d\n" ,  hdr->ih_load);
	printf("hdr->ih_ep = %d\n" ,  hdr->ih_ep);
	printf("hdr->ih_dcrc = %d\n" ,  hdr->ih_dcrc );
	printf("hdr->ih_os = %d\n" ,  hdr->ih_os );
	printf("hdr->ih_arch = %d\n" ,  hdr->ih_arch );
	printf("hdr->ih_type = %d\n" ,  hdr->ih_type );
	printf("hdr->ih_comp = %d\n" ,  hdr->ih_comp );
	printf("hdr->ih_name = %s\n" ,  hdr->ih_name );
#endif

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		munmap(ptr, sbuf.st_size);
		close(ifd);
		printf ("Bad Magic Number: \"%s\" is no valid image\n", imagefile);
		return 0;
	}

	data = (char *)hdr;

	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

	if (crc32 (0, data, sizeof(image_header_t)) != checksum) {
		munmap(ptr, sbuf.st_size);
		close(ifd);
		printf ("*** Warning: \"%s\" has bad header checksum!\n", imagefile);
		return 0;
	}

	/*
	 *  handle Data CRC32
	 */
	data = (char *)(ptr + sizeof(image_header_t));
	data_len  = sbuf.st_size - sizeof(image_header_t) ;

	if (crc32 (0, data, data_len) != ntohl(hdr->ih_dcrc)) {
		munmap(ptr, sbuf.st_size);
		close(ifd);
		printf ("*** Warning: \"%s\" has corrupted data!\n", imagefile);
		return 0;
	}

	munmap(ptr, sbuf.st_size);
	close(ifd);

	return 1;
}

/************************************************
***  example: update 183.230.102.49 1003 wifi-repeator 0
***	argv[1]: server address
***  argv[2]: port
***  argv[3]: project name
***  argv[4]: 0 :query update status
		      1 :query status and update
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
	int h;
	struct hostent *hent;
	char *p;

	int JSONError = 0;
	int JSONUpdate = 0;
	char *pJSONUpdateUrl = NULL;

	/*  */
	if(argc != 5)
	{
		printf("command is wrong.\n");
		return -1;
	}

	system("nvram_set CMCC_HaveNewVersion 0");
	system("nvram_set CMCC_UpdateSuccess 0");

	int port = atoi(argv[2]);
	int update = atoi(argv[4]);

	/* create client socket */
	if((sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		printf("Create TCP socket failed.\n");
		return -1;
	}

	if((pServer = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in*))) == NULL)
	{
		printf("Malloc sockaddr_in failed.\n");
		close(sockFd);
		return -1;
	}

	memset(pServer, 0, sizeof(*pServer));  
	pServer->sin_family = AF_INET;
	pServer->sin_port = htons(port); 
	hent = gethostbyname(argv[1]);
	p = inet_ntoa(*(struct in_addr *)hent->h_addr_list[0]);

	if( inet_pton(AF_INET, p, &pServer->sin_addr) <= 0)
	{  
		printf("inet_pton error for %s\n",argv[1]);
		free(pServer);	
		close(sockFd);
		return -1;  
	}  

	/* connect server */
	if(connect(sockFd, (struct sockaddr *)pServer, sizeof(struct sockaddr)) < 0)
	{
		printf("Could not connect server.\n");
		free(pServer);	
		close(sockFd);
		return -1;
	}
	
	/* form http GET packet(for check whether need to upgrade) */
	pGetPacket = build_get_packet(argv[1], argv[2], argv[3]);
	printf("%s\n",pGetPacket);

	/* send request packet */
	while(countTotal < strlen(pGetPacket))
	{
		count=send(sockFd, pGetPacket + countTotal, strlen(pGetPacket) - countTotal, 0);  

		if(count == -1)
		{  
			printf("Send failed.\n");
			goto error_occur;
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
				printf("server close when read.\n");
				goto error_occur;
			}
			break;
	        }		
		else if (h < 0) 
		{
			printf("select abort .\n");
			goto error_occur;
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
			}

			if(0 == JSONError)
			{			
				/* read the value of "update" */
				if(cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE) != NULL)
				{
					JSONUpdate = cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE)->valueint;
				}

				/* need to upgrade */
				if(1 == JSONUpdate)
				{
					/*write NVRAM*/
					system("nvram_set CMCC_HaveNewVersion 1");

					if(1 == update)
					{					
						/* read the value of "update_url" */
						if(cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE_URL) != NULL)
						{
							if((pJSONUpdateUrl = cJSON_GetObjectItem(pJSONData,  NAME_JSON_UPDATE_URL)->valuestring) != NULL)
							{
								printf("pJSONUpdateUrl = %s\n", pJSONUpdateUrl);

								/* download image file and rename to uImage */
								memset(buff, 0 , BUFF_LEN);
								sprintf(buff, "wget %s -O uImage", pJSONUpdateUrl);
								system(buff);

								/* check uImage head CRC and data CRC */
								if(1 == check("uImage"))
								{
									/* write the file to flash(mtd4 is partition of kernel) and reboot */				
									system("mtd_write write uImage  /dev/mtd4");

									system("nvram_set CMCC_UpdateSuccess 1");
								}

								free(pGetPacket);
								free(pServer);
								close(sockFd);

								return 1;							
							}
						}
					}
					else
					{
						/* no need to upgrade */
						free(pGetPacket);
						free(pServer);	
						close(sockFd);

						return 0;
					}
				}
				else
				{
					/* no need to upgrade */
					free(pGetPacket);
					free(pServer);	
					close(sockFd);

					return 0;
				}
			}
			else
			{
				goto error_occur;
			}
		}		
	}
	
error_occur:
	free(pGetPacket);
	free(pServer);	
	close(sockFd);

	return -1;	
}


