#ifndef DM_DHCP_H_
#define DM_DHCP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/route.h>
#include <sys/time.h>
//#include <net/if.h>
#include <time.h>

#define DHCPFILE	"/etc/repeater_udhcpd.conf"

char *get_ipaddr(char *ifname);
char *get_netmask(char *ifname);

int getuptime(int nvram);
void clear_time(int nvram);

int get_start_end_ip(char *ip , char *netmask, char *start, char *end);
int config_duchpd(char *ifname);

void get_gata_way(char *gateway);
void get_dns(int index, char *mydns);

#endif 
