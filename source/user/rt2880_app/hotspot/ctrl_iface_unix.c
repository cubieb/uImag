/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ctrl_iface_unix.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "hotspot.h"
#include "ieee80211_defs.h"
#include <sys/socket.h>
#include <unistd.h>
#include "ctrl_iface_unix.h"

static void hotspot_ctrl_set_brace_multiple_param(struct hotspot_conf *conf,
                                                  const char *confname,
                                                  const char *param,
                                                  const char *value);

static int hotspot_ctrl_iface_event_register(struct hotspot_ctrl_iface *ctrl_iface,
											 struct sockaddr_un *from,
											 socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst;

	ctrl_dst = os_zalloc(sizeof(*ctrl_dst));

	if (!ctrl_dst) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	os_memcpy(&ctrl_dst->addr, from, sizeof(struct sockaddr_un));
	ctrl_dst->addrlen = fromlen;
	dl_list_add(&ctrl_iface->hs_ctrl_dst_list, &ctrl_dst->list);
		
	return 0;
}

static int hotspot_ctrl_iface_event_unregister(struct hotspot_ctrl_iface *ctrl_iface,
											   struct sockaddr_un *from,
											   socklen_t fromlen)
{
	struct hotspot_ctrl_dst *ctrl_dst, *ctrl_dst_tmp;

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
								struct hotspot_ctrl_dst, list) {
		if (fromlen == ctrl_dst->addrlen && os_memcpy(from->sun_path, ctrl_dst->addr.sun_path,
												fromlen - offsetof(struct sockaddr_un, sun_path))
												== 0) {
			dl_list_del(&ctrl_dst->list);
			os_free(ctrl_dst);
			return 0;
		}
	}

	return -1;
}

static int hotspot_ctrl_iface_cmd_version(struct hotspot *hs,
											char *reply, size_t *reply_len )
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	os_strncpy(reply, HOTSPOT_VERSION, 5);
	*reply_len = 5;
	return 0;
}

static int hotspot_ctrl_iface_cmd_on(struct hotspot *hs, const char *iface,
									 char *reply, size_t *reply_len)
{
	int ret = 0;
	
	ret = hotspot_onoff(hs, iface, 1, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	*reply_len = 0;
 
	return ret;
}

static int hotspot_ctrl_iface_cmd_off(struct hotspot *hs, const char *iface,
									  char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_onoff(hs, iface, 0, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	*reply_len = 0;

	return ret;
}

static int hotspot_ctrl_iface_cmd_get(struct hotspot *hs, const char *iface,
									  char *param, char *reply, size_t *reply_len)
{
	int ret = 0;
	struct hotspot_conf *conf = NULL;
	int is_found = 0;

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (is_found) {
		if (os_strcmp(param, "access_network_type") == 0) {
			sprintf(reply, "access_network_type=%d", conf->access_network_type);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "internet") == 0) {
			sprintf(reply, "internet=%d", conf->internet);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "timezone") == 0) {
			sprintf(reply, "timezone=%s", conf->time_zone);
			*reply_len = os_strlen(reply);
		} else if (os_strcmp(param, "all") == 0) {
			sprintf(reply, "access_network_type=%d\n"
						   "internet=%d\n",
						   conf->access_network_type,
						   conf->internet);
			*reply_len = os_strlen(reply);
		} else
			ret = -1;
	} else
		ret = -1; 

	return ret;
}

static void hotspot_ctrl_clear_general_multiple_param(const char *confname,
											 		  const char *param)
{
	FILE *file, *tmpfile;
	char buf[256];
	//char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf";
	char tmp_confname[256];
        char *tmp_ptr;

	u8 is_clear = 0;

	os_memset(buf, 0, 256);

	tmp_ptr = strrchr(confname, '/');
        os_strncpy(tmp_confname, confname, strlen(confname)- strlen(tmp_ptr));
        tmp_confname[strlen(confname)- strlen(tmp_ptr)] = '\0';

	strcat(tmp_confname, "/hotspot_ap_interface.conf");

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}

	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}
	
	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, param) != NULL) {
			if (!is_clear) {
				fprintf(tmpfile, "%s=n/a\n", param);
				is_clear = 1;
			}

			if (strcmp(param, "venue_name") == 0) 
			{
      	                	int tmplen, is_end = 0;
				do {
                                	for(tmplen=0;tmplen<strlen(buf);tmplen++) {
                                        	if (buf[tmplen] == '}') {
                                                	is_end = 1;
                                                        break;
                                             	}
					}
                                        if (is_end == 0) {
                                        	os_memset(buf, 0, 256);
                                                if (fgets(buf, sizeof(buf), file) == NULL)
                                                	break;
                                                fprintf(tmpfile, "");
                                        } 
                                } while (is_end == 0);
			}
		} else {
			fputs(buf, tmpfile);
		} 
	}
	
	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_general_param(const char *confname,
										   const char *param, char *value)
{
	FILE *file, *tmpfile;
	char buf[256];
	//char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 
	char tmp_confname[256]; 
	char *tmp_ptr;

	os_memset(buf, 0, 256);

	tmp_ptr = strrchr(confname, '/');
	os_strncpy(tmp_confname, confname, strlen(confname)- strlen(tmp_ptr));
	tmp_confname[strlen(confname)- strlen(tmp_ptr)] = '\0';

	strcat(tmp_confname, "/hotspot_ap_interface.conf");

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}

	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}
	
	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, param) != NULL) {
			fprintf(tmpfile, "%s=%s\n", param, value);
		} else {
			fputs(buf, tmpfile);
		} 
	}
	
	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_interface_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "interface", value);
}

static void hotspot_ctrl_set_osu_interface_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "osu_interface", value);
}

static void hotspot_ctrl_set_interworking_param(struct hotspot_conf *conf,
												const char *confname, 
					 							char *value)
{
	hotspot_ctrl_set_general_param(confname, "interworking", value);
}

static void hotspot_ctrl_set_access_net_type_param(struct hotspot_conf *conf,
												   const char *confname, 
					 							   char *value)
{
	hotspot_ctrl_set_general_param(confname, "access_network_type", value);
}

static void hotspot_ctrl_set_internet_param(struct hotspot_conf *conf,
											const char *confname, 
					 						char *value)
{
	hotspot_ctrl_set_general_param(confname, "internet", value);
}

static void hotspot_ctrl_set_venue_group_param(struct hotspot_conf *conf,
											   const char *confname, 
					 						   char *value)
{
	hotspot_ctrl_set_general_param(confname, "venue_group", value);
}

static void hotspot_ctrl_set_venue_type_param(struct hotspot_conf *conf,
											  const char *confname, 
					 						  char *value)
{
	hotspot_ctrl_set_general_param(confname, "venue_type", value);
}

void hotspot_ctrl_set_hessid_param(struct hotspot_conf *conf,
								   const char *confname,
								   char *value)
{
	hotspot_ctrl_set_general_param(confname, "hessid", value);
}

static void hotspot_ctrl_set_roaming_consortium_oi_param(struct hotspot_conf *conf,
														 const char *confname,
														 char *value)
{
	hotspot_ctrl_set_general_param(confname, "roaming_consortium_oi", value);
}

static void hotspot_ctrl_set_advertisement_proto_id_param(struct hotspot_conf *conf,
														  const char *confname,
														  char *value)
{
	hotspot_ctrl_set_general_param(confname, "advertisement_proto_id", value);
}

static void hotspot_ctrl_set_anqp_query_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_param(confname, "anqp_query", value);
}

static void hotspot_ctrl_set_mih_support_param(struct hotspot_conf *conf,
											   const char *confname,
											   char *value)
{
	hotspot_ctrl_set_general_param(confname, "mih_support", value);
}

static void hotspot_ctrl_set_dgaf_disabled_param(struct hotspot_conf *conf,
												 const char *confname,
												 char *value)
{
	hotspot_ctrl_set_general_param(confname, "dgaf_disabled", value);
}

static void hotspot_ctrl_set_timezone_param(struct hotspot_conf *conf,
											const char *confname,
										    char *value)
{
	hotspot_ctrl_set_general_param(confname, "timezone", value);
}

static void hotspot_ctrl_set_gas_cb_delay_param(struct hotspot_conf *conf,
												const char *confname,
												char *value)
{
	hotspot_ctrl_set_general_param(confname, "gas_cb_delay", value);
}

static void hotspot_ctrl_set_hs2_openmode_test_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "hs2_openmode_test", value);
}

static void hotspot_ctrl_set_hs2_legacy_osu_enable(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "legacy_osu", value);
}

static void hotspot_ctrl_set_qosmap_enable(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "qosmap", value);
}

static void hotspot_ctrl_set_qosmap_dscp_range(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "dscp_range", value);
}

static void hotspot_ctrl_set_qosmap_dscp_exception(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "dscp_exception", value);
}

static void hotspot_ctrl_set_qload_mode(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "qload_test", value);
}

static void hotspot_ctrl_set_qload_cu(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "qload_cu", value);
}

static void hotspot_ctrl_set_qload_sta_cnt(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "qload_sta_cnt", value);
}

void hotspot_ctrl_set_proxy_arp_param(struct hotspot_conf *conf,
									  const char *confname,
									  char *value)
{
	hotspot_ctrl_set_general_param(confname, "proxy_arp", value);
}

static void hotspot_ctrl_set_l2_filter_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "l2_filter", value);
}

static void hotspot_ctrl_set_p2p_cross_connect_permitted_param(struct hotspot_conf *conf,
															   const char *confname,
															   char *value)
{
	hotspot_ctrl_set_general_param(confname, "p2p_cross_connect_permitted", value);
}

static void hotspot_ctrl_set_mmpdu_size_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_param(confname, "mmpdu_size", value);
}

static void hotspot_ctrl_set_external_anqp_server_test_param(struct hotspot_conf *conf,
															 const char *confname,
															 char *value)
{
	hotspot_ctrl_set_general_param(confname, "external_anqp_server_test", value);
}

static void hotspot_ctrl_set_icmpv4_deny_param(struct hotspot_conf *conf,
											   const char *confname,
										 	   char *value)
{
	hotspot_ctrl_set_general_param(confname, "icmpv4_deny" ,value);
}

static void hotspot_ctrl_set_ipv4_type_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "ipv4_type", value);
}

static void hotspot_ctrl_set_ipv6_type_param(struct hotspot_conf *conf,
											 const char *confname,
											 char *value)
{
	hotspot_ctrl_set_general_param(confname, "ipv6_type", value);
}

static void hotspot_ctrl_set_ip_type_id_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_ipv4_type_param(conf, confname, "n/a");
	hotspot_ctrl_set_ipv6_type_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_ipv4_type_param(conf, confname, "3");
		hotspot_ctrl_set_ipv6_type_param(conf, confname, "0");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow IP Type ID\n");
	}
}

static void hotspot_ctrl_set_domain_name_param(struct hotspot_conf *conf,
											   const char *confname,
											 	char *value)
{
	hotspot_ctrl_set_general_param(confname, "domain_name", value);
}

static void hotspot_ctrl_set_anonymous_nai_param(struct hotspot_conf *conf,
											   const char *confname,
											 	char *value)
{
	hotspot_ctrl_set_general_param(confname, "anonymous_nai", value);
}

static void hotspot_ctrl_set_iconfile_path_param(struct hotspot_conf *conf,
											   const char *confname,
											 	char *value)
{
	hotspot_ctrl_set_general_param(confname, "icon_path", value);
}

static void hotspot_ctrl_set_icon_tag_param (struct hotspot_conf *conf, const char *conf_name, char *val)
{
    DBGPRINT(RT_DEBUG_TRACE, "(%s): set icon_tag = %d\n", conf_name, val);
	conf->icon_tag = atoi(val);	
//JERRY DBG
	printf("conf->icon_tag=%d\n", conf->icon_tag);
    hotspot_ctrl_set_general_param(conf_name, "icon_tag", val);
}

static void hotspot_set_multiple_param_nums(struct hotspot_conf *conf,
											const char *param,
											u8 param_nums)
{
	if (os_strcmp(param, "venue_name") == 0)
		conf->venue_name_nums = param_nums;
	else if (os_strcmp(param, "network_auth_type") == 0)
		conf->network_auth_type_nums = param_nums;
	else if (os_strcmp(param, "op_friendly_name") == 0)
		conf->op_friendly_name_nums = param_nums;
	else if (os_strcmp(param, "plmn") == 0)
		conf->plmn_nums = param_nums;
	else if (os_strcmp(param, "proto_port") == 0)
		conf->proto_port_nums = param_nums;
	else if (os_strcmp(param, "wan_metrics") == 0)
		conf->wan_metrics_nums = param_nums;
	else if (os_strcmp(param, "nai_realm_data") == 0)
		conf->nai_realm_data_nums = param_nums;
	else if (os_strcmp(param, "osu_providers_list") == 0)
		conf->osu_providers_list_nums = param_nums;		
	else
		DBGPRINT(RT_DEBUG_ERROR, "Unknow parameters(%s)\n", param);
}

static u8 hotspot_get_multiple_param_nums(struct hotspot_conf *conf,
											const char *param)
{
	u8 param_nums = 0;

	if (os_strcmp(param, "venue_name") == 0)
		param_nums = conf->venue_name_nums;
	else if (os_strcmp(param, "network_auth_type") == 0)
		param_nums = conf->network_auth_type_nums;
	else if (os_strcmp(param, "op_friendly_name") == 0)
		param_nums = conf->op_friendly_name_nums;
	else if (os_strcmp(param, "plmn") == 0)
		param_nums = conf->plmn_nums;
	else if (os_strcmp(param, "proto_port") == 0)
		param_nums = conf->proto_port_nums;
	else if (os_strcmp(param, "wan_metrics") == 0)
		param_nums = conf->wan_metrics_nums;
	else if (os_strcmp(param, "nai_realm_data") == 0)
		param_nums = conf->nai_realm_data_nums;
	else if (os_strcmp(param, "osu_providers_list") == 0)
		param_nums = conf->osu_providers_list_nums;		
	else
		DBGPRINT(RT_DEBUG_ERROR, "Unknow parameters(%s)\n", param);

	return param_nums;			
}
static void hotspot_ctrl_set_general_multiple_param(struct hotspot_conf *conf,
													const char *confname,
													const char *param,
													const char *value)
{
	FILE *file, *tmpfile;
	char buf[256];
	//char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 
	char tmp_confname[256]; 	
	char *tmp_ptr;	
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;

	os_memset(buf, 0, 256);
	
	tmp_ptr = strrchr(confname, '/');
	os_strncpy(tmp_confname, confname, strlen(confname)- strlen(tmp_ptr));
	tmp_confname[strlen(confname)- strlen(tmp_ptr)] = '\0';

	strcat(tmp_confname, "/hotspot_ap_interface.conf");
	
	if (os_strcmp(value, "n/a") == 0) {	
		hotspot_ctrl_clear_general_multiple_param(confname, param);
		hotspot_set_multiple_param_nums(conf, param, 0);
	} else {
		file = fopen(confname, "r");

		if (!file) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
			return;
		}

		tmpfile = fopen(tmp_confname, "w");

		if (!tmpfile) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
			fclose(file);
			return;
		}
	
		total_param_nums = hotspot_get_multiple_param_nums(conf, param);
		while (fgets(buf, sizeof(buf), file) != NULL) {
			if ((strstr(buf, param) != NULL)) {
				cur_param_nums++;
				if (total_param_nums == cur_param_nums) {
					fputs(buf, tmpfile);

					if (strcmp(param, "venue_name") == 0)
		                        {
                		                int tmplen, is_end = 0;
                                		do {
		                                        for(tmplen=0;tmplen<strlen(buf);tmplen++) {
                		                                if (buf[tmplen] == '}') {
		                                                	is_end = 1;
                		                                        break;
                                                		}
							}
                                        	
		                                        if (is_end == 0) {
                		                                os_memset(buf, 0, 256);
                                		                if (fgets(buf, sizeof(buf), file) == NULL)
                                                		        break;
                		                                fputs(buf, tmpfile);
							}
						} while (is_end == 0);
                        		}

					fprintf(tmpfile, "%s=%s\n", param, value);
					total_param_nums++;
					hotspot_set_multiple_param_nums(conf, param, total_param_nums);
				} else if (total_param_nums < cur_param_nums) {
					fprintf(tmpfile, "%s=%s\n", param, value);
					hotspot_set_multiple_param_nums(conf, param, 1);
				} else {
					fputs(buf, tmpfile);
				}
			} else {
				fputs(buf, tmpfile);
			} 
		}
	
		fclose(tmpfile);
		fclose(file);

		unlink(confname);
		rename(tmp_confname, confname);
	}
}

static void hotspot_ctrl_set_venue_name_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname, 
											"venue_name", value);
}

static void hotspot_ctrl_set_venue_name_id_param(struct hotspot_conf *conf,
												 const char *confname,
												 char *value)
{
	hotspot_ctrl_set_venue_name_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_venue_name_param(conf, confname, "eng%{Wi-Fi Alliance\n\
2989 Copper Road\n\
Santa Clara, CA 95051, USA}");
		hotspot_ctrl_set_venue_name_param(conf, confname, "chi%{Wi-Fi联盟实验室\n\
二九八九年库柏路\n\
圣克拉拉, 加利福尼亚95051, 美国}");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Venue Name ID\n");
	}
}

static void hotspot_ctrl_set_network_auth_type_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname, "network_auth_type", "n/a");
	hotspot_ctrl_set_general_multiple_param(conf, confname, 
											"network_auth_type", value);
}

static void hotspot_ctrl_set_net_auth_type_id_param(struct hotspot_conf *conf,
													const char *confname,
													char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname, "network_auth_type", "n/a");

	if (os_strcmp(value, "1") == 0) {
        hotspot_ctrl_set_general_multiple_param(conf, confname,
                                            "network_auth_type", "0,https://tandc-server.wi-fi.org");
    } else if (os_strcmp(value, "2") == 0) {
        hotspot_ctrl_set_general_multiple_param(conf, confname,
                                            "network_auth_type", "1");
    } else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Auth Type ID\n");		
	}
}

static void hotspot_ctrl_set_op_friendly_name_param(struct hotspot_conf *conf,
													const char *confname,
													char *value)
{
	hotspot_ctrl_set_general_multiple_param(conf, confname,
											"op_friendly_name", value);
}

static void hotspot_ctrl_set_op_friendly_name_id_param(struct hotspot_conf *conf,
													   const char *confname,
													   char *value)
{
	hotspot_ctrl_set_op_friendly_name_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_op_friendly_name_param(conf, confname, "eng,Wi-Fi Alliance");
		hotspot_ctrl_set_op_friendly_name_param(conf, confname, "chi,Wi-Fi联盟");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Operator Friendly Name ID\n");
	}
}
	
static void hotspot_ctrl_clear_brace_multiple_param(struct hotspot_conf *conf,
													const char *confname,
										   			const char *param)
{
	FILE *file, *tmpfile;
	char tmpbuf[256];
	char buf[256];
	//char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf";
	char tmp_confname[256];
	char *tmp_ptr;
	u8 is_brace_open = 0;
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;

	os_memset(tmpbuf, 0, 256);
	os_memset(buf, 0, 256);

	tmp_ptr = strrchr(confname, '/');
	os_strncpy(tmp_confname, confname, strlen(confname)- strlen(tmp_ptr));
	tmp_confname[strlen(confname)- strlen(tmp_ptr)] = '\0';

	strcat(tmp_confname, "/hotspot_ap_interface.conf");
	
	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		return;
	}
	
	tmpfile = fopen(tmp_confname, "w");

	if (!tmpfile) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
		fclose(file);
		return;
	}

	total_param_nums = hotspot_get_multiple_param_nums(conf, param);

	if (total_param_nums == 0)
		sprintf(tmpbuf, "%s=n/a\n", param);
	else
		sprintf(tmpbuf, "%s={\n", param);

	while (fgets(buf, sizeof(buf), file) != NULL) {
		if (strstr(buf, tmpbuf) != NULL) {
			if ((total_param_nums == 0) || (cur_param_nums == (total_param_nums - 1))) {
				fprintf(tmpfile, "%s=n/a\n", param);
			}

			if (total_param_nums != 0)
				is_brace_open = 1;
		} else if (strstr(buf, "}") != NULL && is_brace_open) {
			cur_param_nums++;
			is_brace_open = 0;
		} else {
			if (!is_brace_open)
				fputs(buf, tmpfile);
		} 
	}

	fclose(tmpfile);
	fclose(file);

	unlink(confname);
	rename(tmp_confname, confname);
}

static void hotspot_ctrl_set_brace_multiple_param(struct hotspot_conf *conf,
												  const char *confname,
												  const char *param,
												  const char *value)
{
	FILE *file, *tmpfile;
	char *token;
	char buf[512], tmp_value[512];
	//char tmp_confname[256] = "/etc_ro/hotspot_ap_interface.conf"; 
	char tmp_confname[512];
	char *tmp_ptr;
	u8 cur_param_nums = 0;
	u8 total_param_nums = 0;
	u8 is_param = 0;

	os_memset(buf, 0, 512);
	os_memset(tmp_value, 0, 512);
	
	tmp_ptr = strrchr(confname, '/');
	os_strncpy(tmp_confname, confname, strlen(confname)- strlen(tmp_ptr));
	tmp_confname[strlen(confname)- strlen(tmp_ptr)] = '\0';

	strcat(tmp_confname, "/hotspot_ap_interface.conf");
	
	if (os_strcmp(value, "n/a") == 0) {
		hotspot_ctrl_clear_brace_multiple_param(conf, confname, param);
		hotspot_set_multiple_param_nums(conf, param, 0);
	} else {
		file = fopen(confname, "r");

		if (!file) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
			return;
		}

		tmpfile = fopen(tmp_confname, "w");

		if (!tmpfile) {
			DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", tmp_confname);
			fclose(file);
			return;
		}
	
		total_param_nums = hotspot_get_multiple_param_nums(conf, param);
		os_memset(tmp_value, 0, 512);
		os_memcpy(tmp_value, value, os_strlen(value));
		while (fgets(buf, sizeof(buf), file) != NULL) {
			if ((strstr(buf, param) != NULL)) {
				is_param = 1;
				if (total_param_nums == 0) {
					fprintf(tmpfile, "%s={\n", param);
					token = strtok(tmp_value, ",");
					do {
						fprintf(tmpfile, "	%s=", token);
						token = strtok(NULL, ",");
						fprintf(tmpfile, "%s\n", token);
						token = strtok(NULL, ",");
					} while (token != NULL);
					fprintf(tmpfile, "}\n");
					hotspot_set_multiple_param_nums(conf, param, 1);
				} else {
					fputs(buf, tmpfile);
				}
			} else {
				if ((strstr(buf, "}") != NULL) && (is_param)) {
					cur_param_nums++;
					if (cur_param_nums == total_param_nums) {
						fputs(buf, tmpfile);
						fprintf(tmpfile, "%s={\n", param);
						token = strtok(tmp_value, ",");
						do {
							fprintf(tmpfile, "	%s=", token);
							token = strtok(NULL, ",");
							fprintf(tmpfile, "%s\n", token);
							token = strtok(NULL, ",");
						} while (token != NULL);
						fprintf(tmpfile, "}\n");
						total_param_nums++;
						hotspot_set_multiple_param_nums(conf, param, total_param_nums);
					} else
						fputs(buf, tmpfile);
					is_param = 0;
				} else
					fputs(buf, tmpfile);
			}
		}
	
		fclose(tmpfile);
		fclose(file);

		unlink(confname);
		rename(tmp_confname, confname);
	}
}
													

static void hotspot_ctrl_set_plmn_param(struct hotspot_conf *conf,
										const char *confname,
										char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "plmn", value);
}

static void hotspot_ctrl_set_operating_class_param(struct hotspot_conf *conf,
												   const char *confname,
												   char *value)
{
	hotspot_ctrl_set_general_param(confname, "operating_class", value);
}

static void hotspot_ctrl_set_operating_class_id_param(struct hotspot_conf *conf,
													  const char *confname,
													  char *value)
{
	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "81");
	} else if (os_strcmp(value, "2") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "115");
	} else if (os_strcmp(value, "3") == 0) {
		hotspot_ctrl_set_operating_class_param(conf, confname, "81,115");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow Operating Class ID\n");
	}
}

static void hotspot_ctrl_set_preferred_candi_list_included_param(struct hotspot_conf *conf,
																 const char *confname,
																 char *value)
{
	hotspot_ctrl_set_general_param(confname, "preferred_candi_list_included", value);
}

static void hotspot_ctrl_set_abridged_param(struct hotspot_conf *conf,
											const char *confname,
											char *value)
{
	hotspot_ctrl_set_general_param(confname, "abridged", value);
}

static void hotspot_ctrl_set_disassociation_imminent_param(struct hotspot_conf *conf,
														   const char *confname,
														   char *value)
{
	hotspot_ctrl_set_general_param(confname, "disassociation_imminent", value);
}

static void hotspot_ctrl_set_bss_termination_included_param(struct hotspot_conf *conf,
															const char *confname,
															char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_termination_included", value);
}

static void hotspot_ctrl_set_ess_disassociation_imminent_param(struct hotspot_conf *conf,
															   const char *confname,
															   char *value)
{
	hotspot_ctrl_set_general_param(confname, "ess_disassociation_imminent", value);
}

static void hotspot_ctrl_set_disassociation_timer_param(struct hotspot_conf *conf,
														const char *confname,
														char *value)
{
	hotspot_ctrl_set_general_param(confname, "disassociation_timer", value);
}

static void hotspot_ctrl_set_validity_interval_param(struct hotspot_conf *conf,
													 const char *confname,
													 char *value)
{
	hotspot_ctrl_set_general_param(confname, "validity_interval", value);
}

static void hotspot_ctrl_set_bss_termination_duration_param(struct hotspot_conf *conf,
															const char *confname,
															char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_termination_duration", value);
}

static void hotspot_ctrl_set_session_information_url_param(struct hotspot_conf *conf,
														   const char *confname,
														   char *value)
{
	hotspot_ctrl_set_general_param(confname, "session_information_url", value);
}

static void hotspot_ctrl_set_bss_transisition_candi_list_preferences_param(struct hotspot_conf *conf,
																		   const char *confname,
																		   char *value)
{
	hotspot_ctrl_set_general_param(confname, "bss_transisition_candi_list_preferences", value);
}

static void hotspot_ctrl_set_proto_port_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "proto_port", value);
}

static void hotspot_ctrl_set_con_cap_id_param(struct hotspot_conf *conf,
											  const char *confname,
											  char *value)
{
	hotspot_ctrl_set_proto_port_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,1,port,0,status,0");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,20,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,22,status,0");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,80,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,443,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,1723,status,0");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,5060,status,0");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,500,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,5060,status,0");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,4500,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,50,port,0,status,1");
    } else if (os_strcmp(value, "3") == 0) {
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,80,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,443,status,1");
    } else if (os_strcmp(value, "4") == 0) {
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,80,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,443,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,6,port,5060,status,1");
        hotspot_ctrl_set_proto_port_param(conf, confname, "ip_protocol,17,port,5060,status,1");
    } else if (os_strcmp(value, "5") == 0) {
	} else {
        DBGPRINT(RT_DEBUG_ERROR, "Unknow Connection Capability ID\n");
    }
}

static void hotspot_ctrl_set_wan_metrics_param(struct hotspot_conf *conf,
											   const char *confname,
											   char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "wan_metrics", value);
}

static void hotspot_ctrl_set_wan_metrics_id_param(struct hotspot_conf *conf,
												  const char *confname,
												  char *value)
{
	hotspot_ctrl_set_wan_metrics_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
        hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,2500,ul_speed,384,dl_load,0,up_load,0,lmd,10");
    } else if (os_strcmp(value, "2") == 0) {
        hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,1500,ul_speed,384,dl_load,20,up_load,20,lmd,10");
    } else if (os_strcmp(value, "3") == 0) {
        hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,2000,ul_speed,1000,dl_load,20,up_load,20,lmd,10");
    } else if (os_strcmp(value, "4") == 0) {
        hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,8000,ul_speed,1000,dl_load,20,up_load,20,lmd,10");
    } else if (os_strcmp(value, "5") == 0) {
        hotspot_ctrl_set_wan_metrics_param(conf, confname, "link_status,1,at_capacity,0,dl_speed,9000,ul_speed,5000,dl_load,20,up_load,20,lmd,10");
    }
    else {
        DBGPRINT(RT_DEBUG_ERROR, "Unknow WAN Metrics ID\n");
    }
}

static void hotspot_ctrl_set_nai_realm_data_param(struct hotspot_conf *conf,
												  const char *confname,
												  char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "nai_realm_data", value);
}

static void hotspot_ctrl_set_nai_realm_id_param(struct hotspot_conf *conf,
												const char *confname,
												char *value)
{
	hotspot_ctrl_set_nai_realm_data_param(conf, confname, "n/a");

	if (os_strcmp(value, "1") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,cisco.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,example.com,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "2") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
	} else if (os_strcmp(value, "3") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,cisco.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,example.com,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "4") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
	} else if (os_strcmp(value, "5") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,ruckuswireless.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
	} else if (os_strcmp(value, "6") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,mail.example.com,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7");
	} else if (os_strcmp(value, "7") == 0) {
		hotspot_ctrl_set_nai_realm_data_param(conf, confname, "nai_realm,wi-fi.org,eap_method,eap-ttls,auth_param,2:4,auth_param,5:7,eap_method,eap-tls,auth_param,5:6");
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Unknow NAI Realm List ID\n");
	}
}

static void hotspot_ctrl_set_osu_providers_list_param(struct hotspot_conf *conf,
												  const char *confname,
												  char *value)
{
	hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", value);
}

static void hotspot_ctrl_set_osu_providers_list_id_param(struct hotspot_conf *conf,
                                                  const char *confname,
                                                  char *value)
{
	char *token, *url = NULL, tmpbuf[2048];
	char *url2 = NULL;
	int osu_method, osu_method2, osu_method3, osu_method4;

	os_memset(tmpbuf, 0, 2048);

	hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "n/a");

	token = strtok(value, ",");
    if (token != NULL) {
		printf("Provider ID = %d\n", atoi(value));
		if (atoi(value) != 6)
		{
			url = strtok(NULL, ",");
            printf("Provider URL = %s\n", url);
            if (url != NULL)
            {
				osu_method = strtok(NULL, ",");
                if (osu_method != NULL)
           		{
                	printf("Provider METHOD = %d\n", atoi(osu_method));
				}
			}

		} else
		{
        	url = strtok(NULL, ",");
			printf("URL = %s\n", url);
			if (url != NULL)
			{
				url2 = strtok(NULL, ",");
	        	printf("URL2 = %s\n", url2);
				if (url2 != NULL)
	        	{
                    	osu_method = strtok(NULL, ",");
						if (osu_method != NULL)
	                    {
							printf("method = %d\n", atoi(osu_method));
	                        osu_method2 = strtok(NULL, ",");
							if (osu_method2 != NULL)
	                        {
								printf("method2 = %d\n", atoi(osu_method2));
                        	}
						}
				}
			}			
		}
    }

	if (os_strcmp(value, "1") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=1,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_red_zxx.png");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_red_eng.png");
		}
		else {
			printf("id=1,normal icon tag 1\n");
			mtk_copy_file ("/etc_ro/icon_red_zxx_default.png", "/etc_ro/icon_red_zxx.png");
			mtk_copy_file ("/etc_ro/icon_red_eng_default.png", "/etc_ro/icon_red_eng.png");
		} 

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 빨강 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_red_zxx.png,icon,160:76:eng:image/png:icon_red_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 빨강 테스트 전용,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_red_zxx.png,icon,160:76:eng:image/png:icon_red_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
        }
    } else if (os_strcmp(value, "2") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=2,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_zxx.png");
        }
		else {
            printf("id=2,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_orange_zxx_default.png", "/etc_ro/icon_orange_zxx.png");
        }

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,eng:Wireless Broadband Alliance,osu_friendly_name,kor:와이어리스 브로드밴드 얼라이언스,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:Wireless Broadband Alliance,osu_friendly_name,kor:와이어리스 브로드밴드 얼라이언스,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
        }
    } else if (os_strcmp(value, "8") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=8,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_red_zxx.png");
        }
		else {
            printf("id=8,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_red_zxx_default.png", "/etc_ro/icon_red_zxx.png");
        }

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 빨강 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_red_zxx.png,osu_nai,anonymous@hotspot.net,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 빨강 테스트 전용,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_red_zxx.png,osu_nai,anonymous@hotspot.net,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
        }
    } else if (os_strcmp(value, "9") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=9,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_zxx.png");
        }
		else {
            printf("id=9,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_orange_zxx_default.png", "/etc_ro/icon_orange_zxx.png");
        }

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,eng:SP Orange Test Only,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,test-anonymous@wi-fi.org,osu_service_desc,n/a", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Orange Test Only,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,test-anonymous@wi-fi.org,osu_service_desc,n/a");
        }
    }  
/*
 else if (os_strcmp(value, "11") == 0) {
        if (url != NULL) {
			sprintf(tmpbuf, "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 레드 시험 만,osu_server_uri,%s,osu_method,1,icon,160:76:eng:image/png:icon_red.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url);
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
		}
		else {
			hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Red Test Only,osu_friendly_name,kor:SP 레드 시험 만,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,160:76:eng:image/png:icon_red.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
		}
	}
*/ 
	else if (os_strcmp(value, "3") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=3,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_red_zxx.png");
        }
		else {
            printf("id=3,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_red_zxx_default.png", "/etc_ro/icon_red_zxx.png");
        }

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,spa:SP Red Test Only,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_red_zxx.png,osu_nai,n/a,osu_service_desc,spa:Free service for test purpose", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,spa:SP Red Test Only,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_red_zxx.png,osu_nai,n/a,osu_service_desc,spa:Free service for test purpose");
        }
    } else if (os_strcmp(value, "4") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=4,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_zxx.png");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_eng.png");
        }
		else {	
            printf("id=4,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_orange_zxx_default.png", "/etc_ro/icon_orange_zxx.png");
            mtk_copy_file ("/etc_ro/icon_orange_eng_default.png", "/etc_ro/icon_orange_eng.png");
        }

        if (url != NULL) {
			sprintf(tmpbuf, "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_orange_zxx.png,icon,160:76:eng:image/png:icon_orange_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
			hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 테스트 전용,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_orange_zxx.png,icon,160:76:eng:image/png:icon_orange_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
        }
	} else if (os_strcmp(value, "5") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=5,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_zxx.png");
        }
		else {
            printf("id=5,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_orange_zxx_default.png", "/etc_ro/icon_orange_zxx.png");
        }

        if (url != NULL) {
            sprintf(tmpbuf, "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
        }
        else {
            hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 테스트 전용,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,1,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스");
        }
    } else if (os_strcmp(value, "6") == 0) { 
		if (conf->icon_tag == 2) {
			printf("id=6,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_green_zxx.png");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_orange_zxx.png");
        }
		else {		
            printf("id=6,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_green_zxx_default.png", "/etc_ro/icon_green_zxx.png");
            mtk_copy_file ("/etc_ro/icon_orange_zxx_default.png", "/etc_ro/icon_orange_zxx.png");
        }

		sprintf(tmpbuf, "osu_friendly_name,eng:SP Green Test Only,kor,SP 초록 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_green_zxx.png,osu_nai,n/a,osu_service_desc,n/a", url, atoi(osu_method));
        hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);

        os_memset(tmpbuf, 0, 2048);
        sprintf(tmpbuf, "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_orange_zxx.png,osu_nai,n/a,osu_service_desc,n/a", url2, atoi(osu_method2));
        hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);		
	} else if (os_strcmp(value, "7") == 0) {
		if (conf->icon_tag == 2) {
			printf("id=7,fake icon tag 2\n");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_green_zxx.png");
            mtk_copy_file ("/etc_ro/wifi-abgn-logo_270x73.png", "/etc_ro/icon_green_eng.png");
        }
		else {
            printf("id=7,normal icon tag 1\n");
            mtk_copy_file ("/etc_ro/icon_green_zxx_default.png", "/etc_ro/icon_green_zxx.png");
            mtk_copy_file ("/etc_ro/icon_green_eng_default.png", "/etc_ro/icon_green_eng.png");
        }

		if (url != NULL) {
 			sprintf(tmpbuf, "osu_friendly_name,eng:SP Green Test Only,kor,SP 초록 테스트 전용,osu_server_uri,%s,osu_method,%d,icon,128:61:zxx:image/png:icon_green_zxx.png,icon,160:76:eng:image/png:icon_green_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서비스", url, atoi(osu_method));
        	hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
		}
		else {
			hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list","osu_friendly_name,eng:SP Green Test Only,kor,SP 녹색 테스트 만,osu_server_uri,https://osu-server.r2-testbed.wi-fi.org,osu_method,0,icon,128:61:zxx:image/png:icon_green_zxx.png,icon,160:76:eng:image/png:icon_green_eng.png,osu_nai,n/a,osu_service_desc,eng:Free service for test purpose,osu_service_desc,kor:테스트 목적으로 무료 서>비스");
		}
	} 
/*
	else if (os_strcmp(value, "10") == 0) {
        sprintf(tmpbuf, "osu_friendly_name,eng:SP Red Test Only,kor,SP 레드 시험 만,osu_server_uri,%s,osu_method,%d,icon,160:76:eng:image/png:icon_red.png,osu_nai,n/a,osu_service_desc,n/a", url, atoi(osu_method));
		hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);

		os_memset(tmpbuf, 0, 2048);
		sprintf(tmpbuf, "osu_friendly_name,eng:SP Blue Test Only,kor,SP 블루 만 테스트,osu_server_uri,%s,osu_method,%d,icon,160:76:eng:image/png:icon_blue.png,osu_nai,n/a,osu_service_desc,n/a", url2, atoi(osu_method2));
        hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);

		os_memset(tmpbuf, 0, 2048);
		sprintf(tmpbuf, "osu_friendly_name,eng:SP Green Test Only,kor,SP 녹색 테스트 만,osu_server_uri,%s,osu_method,%d,icon,160:76:eng:image/png:icon_green.png,osu_nai,n/a,osu_service_desc,n/a", url3, atoi(osu_method3));
        hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);

		os_memset(tmpbuf, 0, 2048);
		sprintf(tmpbuf, "osu_friendly_name,eng:SP Orange Test Only,kor,SP 오렌지 시험 만,osu_server_uri,%s,osu_method,%d,icon,160:76:eng:image/png:icon_orange.png,osu_nai,n/a,osu_service_desc,n/a", url4, atoi(osu_method4));
        hotspot_ctrl_set_brace_multiple_param(conf, confname, "osu_providers_list", tmpbuf);
	}
*/
	else {
        DBGPRINT(RT_DEBUG_ERROR, "Unknow OSU Provider List ID\n");
    }
}

/*
 * hotspot related parameters setting
 */
static struct hotspot_ctrl_set_param hs_ctrl_set_params[] = {
	{"interface", hotspot_ctrl_set_interface_param},
	{"interworking", hotspot_ctrl_set_interworking_param},
	{"access_network_type", hotspot_ctrl_set_access_net_type_param},
	{"internet", hotspot_ctrl_set_internet_param},
	{"venue_group", hotspot_ctrl_set_venue_group_param},
	{"venue_type", hotspot_ctrl_set_venue_type_param},
	{"anqp_query", hotspot_ctrl_set_anqp_query_param},
	{"mih_support", hotspot_ctrl_set_mih_support_param},
	{"venue_name", hotspot_ctrl_set_venue_name_param},
	{"venue_name_id", hotspot_ctrl_set_venue_name_id_param},
	{"hessid", hotspot_ctrl_set_hessid_param},
	{"roaming_consortium_oi", hotspot_ctrl_set_roaming_consortium_oi_param},
	{"advertisement_proto_id", hotspot_ctrl_set_advertisement_proto_id_param},
	{"domain_name", hotspot_ctrl_set_domain_name_param},
	{"network_auth_type", hotspot_ctrl_set_network_auth_type_param},
	{"net_auth_type_id", hotspot_ctrl_set_net_auth_type_id_param},
	{"ipv4_type", hotspot_ctrl_set_ipv4_type_param},
	{"ipv6_type", hotspot_ctrl_set_ipv6_type_param},
	{"ip_type_id", hotspot_ctrl_set_ip_type_id_param},
	{"nai_realm_data", hotspot_ctrl_set_nai_realm_data_param},
	{"nai_realm_id", hotspot_ctrl_set_nai_realm_id_param},
	{"op_friendly_name", hotspot_ctrl_set_op_friendly_name_param},
	{"op_friendly_name_id", hotspot_ctrl_set_op_friendly_name_id_param},
	{"proto_port", hotspot_ctrl_set_proto_port_param},
	{"con_cap_id", hotspot_ctrl_set_con_cap_id_param},
	{"wan_metrics", hotspot_ctrl_set_wan_metrics_param},
	{"wan_metrics_id", hotspot_ctrl_set_wan_metrics_id_param},
	{"plmn", hotspot_ctrl_set_plmn_param},
	{"operating_class", hotspot_ctrl_set_operating_class_param},
	{"operating_class_id", hotspot_ctrl_set_operating_class_id_param},
	{"preferred_candi_list_included", hotspot_ctrl_set_preferred_candi_list_included_param},
	{"abridged", hotspot_ctrl_set_abridged_param},
	{"disassociation_imminent", hotspot_ctrl_set_disassociation_imminent_param},
	{"bss_termination_included", hotspot_ctrl_set_bss_termination_included_param},
	{"ess_disassociation_imminent", hotspot_ctrl_set_ess_disassociation_imminent_param},
	{"disassociation_timer", hotspot_ctrl_set_disassociation_timer_param},
	{"validity_interval", hotspot_ctrl_set_validity_interval_param},
	{"bss_termination_duration", hotspot_ctrl_set_bss_termination_duration_param},
	{"session_information_url", hotspot_ctrl_set_session_information_url_param},
	{"bss_transisition_candi_list_preferences", hotspot_ctrl_set_bss_transisition_candi_list_preferences_param},
	{"timezone", hotspot_ctrl_set_timezone_param},
	{"dgaf_disabled", hotspot_ctrl_set_dgaf_disabled_param},
	{"proxy_arp", hotspot_ctrl_set_proxy_arp_param},
	{"l2_filter", hotspot_ctrl_set_l2_filter_param},
	{"icmpv4_deny", hotspot_ctrl_set_icmpv4_deny_param},
	{"p2p_cross_connect_permitted", hotspot_ctrl_set_p2p_cross_connect_permitted_param},
	{"mmpdu_size", hotspot_ctrl_set_mmpdu_size_param},
	{"external_anqp_server_test", hotspot_ctrl_set_external_anqp_server_test_param},
	{"gas_cb_delay", hotspot_ctrl_set_gas_cb_delay_param},
	{"hs2_openmode_test", hotspot_ctrl_set_hs2_openmode_test_param},
	{"anonymous_nai", hotspot_ctrl_set_anonymous_nai_param},
	{"osu_interface", hotspot_ctrl_set_osu_interface_param},
	{"osu_providers_list", hotspot_ctrl_set_osu_providers_list_param},
	{"osu_providers_id", hotspot_ctrl_set_osu_providers_list_id_param},
	{"legacy_osu", hotspot_ctrl_set_hs2_legacy_osu_enable},
	{"icon_path", hotspot_ctrl_set_iconfile_path_param},
	{"icon_tag", hotspot_ctrl_set_icon_tag_param},
	{"qosmap", hotspot_ctrl_set_qosmap_enable},
	{"dscp_range", hotspot_ctrl_set_qosmap_dscp_range},
	{"dscp_exception", hotspot_ctrl_set_qosmap_dscp_exception},
	{"qload_test", hotspot_ctrl_set_qload_mode},
	{"qload_cu", hotspot_ctrl_set_qload_cu},
	{"qload_sta_cnt", hotspot_ctrl_set_qload_sta_cnt},
}; 

static int hotspot_ctrl_iface_cmd_set(struct hotspot *hs, const char *iface,
									  char *param_value_pair, char *reply, size_t *reply_len)
{
	int ret = 0;
	struct hotspot_conf *conf = NULL;
	int is_found = 0;
	char *token;
	char confname[256];
	struct hotspot_ctrl_set_param *match = NULL;
	struct hotspot_ctrl_set_param *param = hs_ctrl_set_params;

	os_memset(confname, 0, 256);

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (is_found) {
		token = strtok(param_value_pair, " ");
		while (param->param) {
			if (os_strcmp(param->param, token) == 0) {
				match = param;
				break;
			}
			param++;
		}
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "Do not find corresponding hotspot configuration for this interface %s\n", iface);
		return -1;
	}
	
	if (match) {
		//sprintf(confname, "/etc_ro/hotspot_ap_%s.conf", iface);
		sprintf(confname, conf->confname);
		token = strtok(NULL, "");
		match->set_param(conf, confname, token);
	} else {
		DBGPRINT(RT_DEBUG_OFF, "Unkown parameters\n");
		return -1;
	}

	return ret;
}
static int hotspot_ctrl_iface_cmd_btmreq(struct hotspot *hs, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char *buf;
	size_t btm_req_len = 0;
	struct hotspot_conf *conf;
	int is_found = 0;
	char peer_addr[6], *token;
	char *peer_sta_addr = NULL,*ess_disassoc_imm = NULL, *disassoc_timer = NULL, *sess_url = NULL;
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_ERROR, "can not find configuration for interface(%s)\n", iface);
		return -1;
	}

	token = strtok(param_value_pair, " ");	
	while (token != NULL) {
		switch(i)
		{
			case 0:
				peer_sta_addr = token;
				break;
			case 1:
				ess_disassoc_imm = token;
				break;
			case 2:
				disassoc_timer = token;
				break;
			case 3:
				sess_url = token;
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown paramter:%s \n", token);
				break;
		}
		i++;
		
		token = strtok(NULL, " ");
	}
	
	// update btm paramters
	conf->ess_disassociation_imminent = atoi(ess_disassoc_imm);
	conf->disassociation_timer = atoi(disassoc_timer);
	if (conf->have_session_info_url)
		os_free(conf->session_info_url);
	conf->have_session_info_url = 1;
	conf->disassociation_imminent = 1;
	conf->session_info_url_len = os_strlen(sess_url);
	conf->session_info_url = os_zalloc(conf->session_info_url_len);
	os_memcpy(conf->session_info_url, sess_url, conf->session_info_url_len);	
	
	DBGPRINT(RT_DEBUG_OFF,"ESS_IMM[%d] Disassoc_Timer[%d] Session_URL : ", conf->ess_disassociation_imminent, conf->disassociation_timer);
	{
		for(i=0;i<conf->session_info_url_len;i++)
			printf("%c",conf->session_info_url[i]); 
		
		printf("\n");	
	}
	
	i = 0;
	token = strtok(peer_sta_addr, ":");
	while (token != NULL) {
		AtoH(token, &peer_addr[i], 1);
		DBGPRINT(RT_DEBUG_INFO, "peer_mac[%d] = 0x%02x\n", i, peer_addr[i]);
		i++;
		if (i >= 6)
			break;
		token = strtok(NULL, ":");
	}

	if (i != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "Wrong mac addr\n");
		return -1;
	}

	btm_req_len = hotspot_calc_btm_req_len(hs, conf);

	buf = os_zalloc(btm_req_len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "Not available memory\n");
		return -1;
	}

	hotspot_collect_btm_req(hs, conf, peer_addr, buf);

	hotspot_send_btm_req(hs, iface, peer_addr, buf, btm_req_len);

	os_free(buf);
	
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_drv_version(struct hotspot *hs, const char *iface,
										 	  char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_driver_version(hs, iface, reply, reply_len);

	return ret;	
}

static int hotspot_ctrl_iface_cmd_ipv4_proxy_arp_list(struct hotspot *hs, const char *iface,
										 		 char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_ipv4_proxy_arp_list(hs, iface, reply, reply_len);

	return ret;
}

static int hotspot_ctrl_iface_cmd_ipv6_proxy_arp_list(struct hotspot *hs, const char *iface,
										 		 char *reply, size_t *reply_len)
{
	int ret = 0;

	ret = hotspot_ipv6_proxy_arp_list(hs, iface, reply, reply_len);

	return ret;
}

static int hotspot_ctrl_iface_cmd_reload(struct hotspot *hs, const char *iface,
										 char *reply, size_t *reply_len)
{
	int ret = 0;
	
	if (hs->opmode == OPMODE_AP)
		ret = hotspot_ap_reload(hs, iface);
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_qosmap(struct hotspot *hs, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char *url = NULL, cnt = 0;
	size_t btm_req_len = 0;
	struct hotspot_conf *conf;
	int is_found = 0, delay = 0, type = 0;
	char code, peer_addr[6], *token, tmpbuf[256], tmpbuf2[256], exception[64], range[64];
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_ERROR, "can not find configuration for interface(%s)\n", iface);
		return -1;
	}

	os_strcpy(tmpbuf, param_value_pair);
	
	token = strtok(tmpbuf, ":");
	while (token != NULL) {
		AtoH(token, &peer_addr[i], 1);
		DBGPRINT(RT_DEBUG_INFO, "peer_mac[%d] = 0x%02x\n", i, peer_addr[i]);
		i++;
		if (i >= 6)
			break;
		token = strtok(NULL, ":");
	}

	if (i != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "Wrong mac addr\n");
		return -1;
	}
	
	//type
	printf("param_value_pair1=%s\n", param_value_pair);
	token = strtok(param_value_pair, " ");	
	if (token != NULL) {
		printf("token=%s\n", token);
		token = strtok(NULL, " ");
		type = atoi(token);
		printf("type=%d\n", type);
	}
	
	os_memset(tmpbuf2, 0, 256);	
	if (type == 0)
	{
		token = strtok(NULL, " ");
		os_memcpy(exception, token, os_strlen(token));
		token = strtok(NULL, " ");
		os_memcpy(range, token, os_strlen(token));
		
		token = strtok(exception, ":");	
		while (token != NULL) {
			tmpbuf2[cnt++] = atoi(token);
			token = strtok(NULL, ":");
		}
		
		token = strtok(range, ":");	
		while (token != NULL) {
			tmpbuf2[cnt++] = atoi(token);
			token = strtok(NULL, ":");
		}
		
		printf("iface=%s\n", iface);
		hotspot_send_qosmap_configure(hs, iface, peer_addr, tmpbuf2, cnt);
	}
	else 
	{
		token = strtok(NULL, " ");
		os_memcpy(range, token, os_strlen(token));
		
		token = strtok(range, ":");	
		while (token != NULL) {
			tmpbuf2[cnt++] = atoi(token);
			token = strtok(NULL, ":");
		}
		
		printf("iface=%s\n", iface);
		hotspot_send_qosmap_configure(hs, iface, peer_addr, tmpbuf2, cnt);
	}
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_wnmreq(struct hotspot *hs, const char *iface,
										 char *param_value_pair,
										 char *reply, size_t *reply_len)
{
	int ret = 0, i = 0;
	char *url = NULL;
	size_t btm_req_len = 0;
	struct hotspot_conf *conf;
	int is_found = 0, delay = 0, type = 0;
	char code, peer_addr[6], *token, tmpbuf[256], tmpbuf2[256];
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_ERROR, "can not find configuration for interface(%s)\n", iface);
		return -1;
	}

	os_strcpy(tmpbuf, param_value_pair);
	
	token = strtok(tmpbuf, ":");
	while (token != NULL) {
		AtoH(token, &peer_addr[i], 1);
		DBGPRINT(RT_DEBUG_INFO, "peer_mac[%d] = 0x%02x\n", i, peer_addr[i]);
		i++;
		if (i >= 6)
			break;
		token = strtok(NULL, ":");
	}

	if (i != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "Wrong mac addr\n");
		return -1;
	}
	
	//type
	printf("param_value_pair1=%s\n", param_value_pair);
	token = strtok(param_value_pair, " ");	
	if (token != NULL) {
		printf("token=%s\n", token);
		token = strtok(NULL, " ");
		type = atoi(token);
		printf("type=%d\n", type);
	}
	
	os_memset(tmpbuf2, 0, 256);	
	if (type == 0)
	{
		//token = strtok(param_value_pair, " ");	
		token = strtok(NULL, " ");
		while (token != NULL) {
			//printf("token=%s\n", token);
			//buf = token;
			os_memcpy(tmpbuf2, token, os_strlen(token));
			token = strtok(NULL, " ");
		}
		
		printf("iface=%s,%s\n", iface, tmpbuf2);
		hotspot_send_wnm_notify_req(hs, iface, peer_addr, tmpbuf2, os_strlen(tmpbuf2), type);
	}
	else
	{
		token = strtok(NULL, " ");
		code = atoi(token);
		token = strtok(NULL, " ");
		delay = atoi(token);
		token = strtok(NULL, " ");
		if (token != NULL)
		{
			url = token;
			tmpbuf2[0] = code;
			os_memcpy(&tmpbuf2[1], &delay, 2);
			os_memcpy(&tmpbuf2[3], url, os_strlen(url));
			
			printf("iface=%s\n", iface);
			hotspot_send_wnm_notify_req(hs, iface, peer_addr, tmpbuf2, 3+os_strlen(url), type);
		}
		else
		{
			tmpbuf2[0] = code;
			os_memcpy(&tmpbuf2[1], &delay, 2);
			
			printf("iface=%s\n", iface);
			hotspot_send_wnm_notify_req(hs, iface, peer_addr, tmpbuf2, 3, type);
		}
	}
	
	return ret;
}

static int hotspot_ctrl_iface_cmd_process(struct hotspot *hs, char *buf, 
											char *reply, size_t *reply_len)
{
	int ret = 0;
	char *token, *token1;
//JERRY
	char tmp[2048];
	char iface[256], cmd[2048];
	int linelen = 0;


	token = strtok(buf, "\n");

	os_memset(tmp, 0, 2048);
	os_memset(iface, 0, 256);
	os_memset(cmd, 0, 2048);

//	printf("==>token=%s\n", *token);
	while (token != NULL) {
		linelen = os_strlen(token);
		printf("len=%d\n", linelen);
		os_strncpy(tmp, token, 2048);
		tmp[2047] = '\0';

		token1 = strtok(tmp, "=");

		if (os_strcmp(token1, "interface") == 0) {
			token1 = strtok(NULL, "");
			os_strncpy(iface, token1, 256);
			iface[255] = '\0';
		} else if (os_strcmp(token1, "cmd") == 0) {
			printf("!!!token1=%s\n", token1);
			token1 = strtok(NULL, "");
//			os_strncpy(cmd, token1, 256);
//			cmd[255] = '\0';
			printf("!!token2=%s\n", token1);
			os_strncpy(cmd, token1, 2048);
            cmd[2047] = '\0';
		}

		token = strtok(token + linelen + 1, "\n");
	}

	os_sleep(0, 5000);

	DBGPRINT(RT_DEBUG_ERROR, "interface = %s, cmd = %s\n", iface, cmd);

	if (os_strcmp(cmd, "hs_version") == 0)
		ret = hotspot_ctrl_iface_cmd_version(hs, reply, reply_len);
	else if (os_strcmp(cmd, "on") == 0)
		ret = hotspot_ctrl_iface_cmd_on(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "off") == 0)
		ret = hotspot_ctrl_iface_cmd_off(hs, iface, reply, reply_len);
	else if (os_strncmp(cmd, "get", 3) == 0)
		ret = hotspot_ctrl_iface_cmd_get(hs, iface, cmd + 4, reply, reply_len);
	else if (os_strncmp(cmd, "set", 3) == 0)
		ret = hotspot_ctrl_iface_cmd_set(hs, iface ,cmd + 4, reply, reply_len);
	else if (os_strncmp(cmd, "btmreq", 6) == 0)
		ret = hotspot_ctrl_iface_cmd_btmreq(hs, iface, cmd + 7, reply, reply_len);
	else if (os_strncmp(cmd, "drv_version", 11) == 0)
		ret = hotspot_ctrl_iface_cmd_drv_version(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "ipv4_proxy_arp_list") == 0)
		ret = hotspot_ctrl_iface_cmd_ipv4_proxy_arp_list(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "ipv6_proxy_arp_list") == 0)
		ret = hotspot_ctrl_iface_cmd_ipv6_proxy_arp_list(hs, iface, reply, reply_len);
	else if (os_strcmp(cmd, "reload") == 0)
		ret = hotspot_ctrl_iface_cmd_reload(hs, iface, reply, reply_len);
	else if (os_strncmp(cmd, "wnmreq", 6) == 0)
		ret = hotspot_ctrl_iface_cmd_wnmreq(hs, iface, cmd + 7, reply, reply_len);	
	else if (os_strncmp(cmd, "qosmap", 6) == 0)
		ret = hotspot_ctrl_iface_cmd_qosmap(hs, iface, cmd + 7, reply, reply_len);			
	else {
		DBGPRINT(RT_DEBUG_ERROR, "no such command\n");
		ret = -1;
	}

	if (ret == 0 && reply_len > 0)
		ret = 1;

	return ret;
}

static void hotspot_ctrl_iface_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct hotspot *hs = eloop_ctx;
	struct hotspot_ctrl_iface *ctrl_iface = sock_ctx;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	int receive_len;
	char buf[4096]; //JERRY 256];
	size_t replylen = 2047;
	char reply[2048];
	int ret;

	//os_memset(buf, 0, 256);
//JERRY
	os_memset(buf, 0, 4096);
	os_memset(reply, 0, 2048);

	receive_len = recvfrom(sock, buf, sizeof(buf) - 1, 0, 
							(struct sockaddr *)&from, &fromlen);

	if (receive_len < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "receive from control interface fail\n");
		return;
	}

	buf[receive_len] = '\0';

	if (os_strcmp(buf, "EVENT_REGISTER") == 0) {
		if (hotspot_ctrl_iface_event_register(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);

	} else if(os_strcmp(buf , "EVENT_UNREGISTER") == 0) {
		if (hotspot_ctrl_iface_event_unregister(ctrl_iface, &from, fromlen))
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
	} else {
			ret = hotspot_ctrl_iface_cmd_process(hs, buf, reply, &replylen);
		if (ret == -1)
			sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *)&from, fromlen);
		else if (ret == 0)
			sendto(sock, "OK\n", 3, 0, (struct sockaddr *)&from, fromlen);
		else
			sendto(sock, reply, replylen, 0, (struct sockaddr *)&from, fromlen);
	}

	return;
}

struct hotspot_ctrl_iface *hotspot_ctrl_iface_init(struct hotspot *hs)
{
	struct hotspot_ctrl_iface *ctrl_iface;
	struct sockaddr_un addr;

	ctrl_iface = os_zalloc(sizeof(*ctrl_iface));

	if (!ctrl_iface) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		goto error0;
	}

	dl_list_init(&ctrl_iface->hs_ctrl_dst_list);
	ctrl_iface->sock = -1;

	ctrl_iface->sock = socket(PF_UNIX, SOCK_DGRAM, 0);

	if (ctrl_iface->sock < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "create socket for ctrl interface fail\n");
		goto error1;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	//os_snprintf(addr.sun_path,sizeof(addr.sun_path),"/tmp/hotspot%s",hs->iface);	
	os_strncpy(addr.sun_path, "/tmp/hotspot", sizeof(addr.sun_path));

	if (bind(ctrl_iface->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "bind addr to ctrl interface fail\n");
		goto error2;
	}

	eloop_register_read_sock(ctrl_iface->sock, hotspot_ctrl_iface_receive, hs, 
										ctrl_iface);

	return ctrl_iface;

error2:
	close(ctrl_iface->sock);
error1:
	os_free(ctrl_iface);
error0:
	return NULL;
}

void hotspot_ctrl_iface_deinit(struct hotspot *hs)
{
	struct hotspot_ctrl_iface *ctrl_iface = hs->hs_ctrl_iface;
	struct hotspot_ctrl_dst	*ctrl_dst, *ctrl_dst_tmp;
	char socket_path[64]={0};

	eloop_unregister_read_sock(ctrl_iface->sock);
		
	close(ctrl_iface->sock);
	ctrl_iface->sock = -1;

	//os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot%s",hs->iface);
	os_snprintf(socket_path,sizeof(socket_path),"/tmp/hotspot");
	unlink(socket_path);

	dl_list_for_each_safe(ctrl_dst, ctrl_dst_tmp, &ctrl_iface->hs_ctrl_dst_list,
									struct hotspot_ctrl_dst, list) {
		os_free(ctrl_dst);
	}

	os_free(ctrl_iface);
}
