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
	hotspot.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include "ieee80211_defs.h"
#include "hotspot.h"

int RTDebugLevel = RT_DEBUG_WARN;

//extern struct hotspot_drv_ops hotspot_drv_ranl_ops;

extern struct hotspot_drv_ops hotspot_drv_wext_ops;

char * hotspot_config_get_line(char *s, int size, FILE *stream, int *line,
									 char **_pos)
{
	char *pos, *end, *sstart;
    
    while (fgets(s, size, stream)) {
        (*line)++; 
        s[size - 1] = '\0';
        pos = s;
                       
        /* Skip white space from the beginning of line. */
        while (*pos == ' ' || *pos == '\t' || *pos == '\r')
            pos++;
            
        /* Skip comment lines and empty lines */
        if (*pos == '#' || *pos == '\n' || *pos == '\0')
            continue;
                
        /* 
         * Remove # comments unless they are within a double quoted
         * string.
		 */
        sstart = os_strchr(pos, '"');
        if (sstart)
            sstart = os_strrchr(sstart + 1, '"');
        if (!sstart)
            sstart = pos;
        end = os_strchr(sstart, '#');
        if (end)
            *end-- = '\0';
        else
            end = pos + os_strlen(pos) - 1;

        /* Remove trailing white space. */
        while (end > pos &&
               (*end == '\n' || *end == ' ' || *end == '\t' ||
            *end == '\r'))
            *end-- = '\0';

        if (*pos == '\0')
            continue;

		if (_pos)
            *_pos = pos;
        return pos;
    }

    if (_pos)
        *_pos = NULL;
    return NULL;
}

#if 0
static int hotspot_writeback_sta_config(struct hotspot *hs, const char *confname)
{
	int ret = 0;

	return ret;
}
#endif

static int hotspot_init_sta_config(struct hotspot *hs, const char *confname)
{
	int ret = 0;
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256], tmp1buf[256], tmp2buf[256];
	int line = 0, i;
	struct hotspot_conf *conf;
	int query_id;

	DBGPRINT(RT_DEBUG_TRACE, "%s(%s)\n", __FUNCTION__, confname);

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	os_memset(tmp1buf, 0, 256);
	os_memset(tmp2buf, 0, 256);

	conf = os_zalloc(sizeof(struct hotspot_conf));
	
	if (!conf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available\n"));
		return -1;
	}

	/* Following are 802.11u element list */
	dl_list_init(&conf->anqp_capability_list);
	dl_list_init(&conf->venue_name_list);
	dl_list_init(&conf->emergency_call_number_list);
	dl_list_init(&conf->network_auth_type_list);
	dl_list_init(&conf->oi_duple_list);
	dl_list_init(&conf->nai_realm_list);
	dl_list_init(&conf->plmn_list);
	dl_list_init(&conf->domain_name_list);
	
	/* Following are HS2.0 elemets list */
	dl_list_init(&conf->hs_capability_list);
	dl_list_init(&conf->operator_friendly_duple_list);
	dl_list_init(&conf->connection_capability_list);
	dl_list_init(&conf->nai_home_realm_name_query_list);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", confname);
		goto error;
	}

	while (hotspot_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "interface = %s\n", conf->iface);
			} else if (os_strcmp(token, "hs_peer_mac") == 0) {
				i = 0;
				token = strtok(NULL, ":");
				while (token != NULL) {
					AtoH(token, &conf->hs_peer_mac[i], 1);
					DBGPRINT(RT_DEBUG_TRACE, "hs_peer_mac[%d] = 0x%02x\n", i, conf->hs_peer_mac[i]);
					i++;
					token = strtok(NULL, ":");
				}
			} else if (os_strcmp(token, "ANQPQueryID") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					query_id = atoi(token);
					switch (query_id) {
					case ANQP_CAPABILITY:
						conf->query_anqp_capability_list = 1;
						break;
					case VENUE_NAME_INFO:
						conf->query_venue_name = 1;
						break;
					case EMERGENCY_CALL_NUMBER_INFO:
						conf->query_emergency_call_number = 1;
						break;
					case NETWORK_AUTH_TYPE_INFO:
						conf->query_network_auth_type = 1;
						break;
					case ROAMING_CONSORTIUM_LIST:
						conf->query_roaming_consortium_list = 1;
						break;
					case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
						conf->query_ip_address_type = 1;
						break;
					case NAI_REALM_LIST:
						conf->query_nai_realm_list = 1;
						break;
					case ThirdGPP_CELLULAR_NETWORK_INFO:
						conf->query_3gpp_network_info = 1;
						break;
					case AP_GEOSPATIAL_LOCATION:
						conf->query_ap_geospatial_location = 1;
						break;
					case AP_CIVIC_LOCATION:
						conf->query_ap_civic_location = 1;
						break;
					case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
						conf->query_ap_location_public_uri = 1;
						break;
					case DOMAIN_NAME_LIST:
						conf->query_domain_name_list = 1;
						break;
					case EMERGENCY_ALERT_IDENTIFIER_URI:
						conf->query_emergency_alert_uri = 1;
						break;
					case EMERGENCY_NAI:
						conf->query_emergency_nai = 1;
						break;
					default:
						DBGPRINT(RT_DEBUG_ERROR, "Unknown QueryID\n");
						break;
					}
					token = strtok(NULL, ";");
				}
			} else if (os_strcmp(token, "ANQPQueryType") == 0) {
				token = strtok(NULL, ";");
				conf->anqp_req_type = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "ANQPQueryType = %d\n", conf->anqp_req_type);
				token = strtok(NULL, "");
			} else if (os_strcmp(token, "HSANQPQueryID") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					query_id = atoi(token);
					switch (query_id) {
					case HS_CAPABILITY:
						conf->query_hs_capability_list = 1;
						break;
					case OPERATOR_FRIENDLY_NAME:
						conf->query_operator_friendly_name = 1;
						break;
					case WAN_METRICS:
						conf->query_wan_metrics = 1;
						break;
					case CONNECTION_CAPABILITY:
						conf->query_connection_capability_list = 1;
						break;
					case NAI_HOME_REALM_QUERY:
						conf->query_nai_home_realm = 1;
						break;
					default:
						DBGPRINT(RT_DEBUG_ERROR, "Unknown HS2.0 QueryID\n");
						break;
						
					}
					token = strtok(NULL, ";");
				}
			}
		}
	}
	
	dl_list_add_tail(&hs->hs_conf_list, &conf->list);

	fclose(file);

	return ret;
	
error:
	os_free(conf);
	return -1;
}

static int hotspot_drv_ops_pre_check(struct hotspot *hs, const char *iface)
{
	int is_found = 0;
	struct hotspot_conf *conf;
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	if (conf->hotspot_onoff)
		return 0;
	else {
		DBGPRINT(RT_DEBUG_ERROR, "hs daemon disable\n");
		return -1;
	}
}

/* hotspot parameter setting */
inline static int hotspot_param_setting(struct hotspot *hs, 
										struct hotspot_conf *conf, 
										u32 param, u32 value)
{
	int ret;

	ret = hs->drv_ops->drv_param_setting(hs->drv_data, conf->iface, 
								    param, value);
	return ret;
}

static int hotspot_init_param_setting(struct hotspot *hs, struct hotspot_conf *conf)
{
	int ret;

	/* DGAF */
	ret = hotspot_param_setting(hs, conf, PARAM_DGAF_DISABLED, conf->DGAF_disabled & 0x01);

	if (ret)
		return -1;

	/* Proxy ARP */
	/* From Spec, if DGAF Disabel bit set to 1, Proxy ARP service shall be enabled */
	if (conf->DGAF_disabled) {
		char value[3];
		os_strcpy(value, "1");
		conf->proxy_arp = 1;
		hotspot_ctrl_set_proxy_arp_param(conf, conf->confname, value);
	}

	ret = hotspot_param_setting(hs, conf, PARAM_PROXY_ARP, conf->proxy_arp);

	if (ret)
		return -1;

	/* l2_filter */
	ret = hotspot_param_setting(hs, conf, PARAM_L2_FILTER, conf->l2_filter);

	if (ret)
		return -1;

	/* MMPDU Size */
	ret = hotspot_param_setting(hs, conf, PARAM_MMPDU_SIZE, conf->mmpdu_size);

	if (ret)
		return -1;

	/* icmpv4_deny */
	ret = hotspot_param_setting(hs, conf, PARAM_ICMPV4_DENY, conf->icmpv4_deny);

	if (ret)
		return -1;

	/* external_anqp_server_test */
	ret = hotspot_param_setting(hs, conf, PARAM_EXTERNAL_ANQP_SERVER_TEST, 
											conf->external_anqp_server_test);

	if (ret)
		return -1;

	/* GAS come back delay */
	ret = hotspot_param_setting(hs, conf, PARAM_GAS_COME_BACK_DELAY, 
											conf->gas_cb_delay);

	if (ret)
		return -1;

	/* set WNM BSS transition management */
	
		ret = hotspot_param_setting(hs, conf, PARAM_WNM_BSS_TRANSITION_MANAGEMENT, 1);

	if (ret)
		return -1;

	/* set WNM Notification */

	if (hs->version >= 2)
	{
		ret = hotspot_param_setting(hs, conf, PARAM_WNM_NOTIFICATION, 1);

		if (ret)
			return -1;

		if (!conf->legacy_osu_exist) {
			char asan_enable = 1;

			//20160531 wants hs daemon to start with even only 1 SSID, ignore if osu setting fail

			//set osu interface asan enable
			ret = hotspot_set_osu_asan(hs, conf->osu_iface, &asan_enable, 1);
		
			//get legacy osu SSID
			ret += hotspot_get_legacy_osu_ssid(hs, conf);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, "%s: single SSID case , please enable MBSSID if you need OSU function\n", __FUNCTION__);					
			}
			else
			{
				/*set OSEN DGAF=1, PROXY_ARP=1 */
			    //ret = hotspot_param_setting(hs, conf, PARAM_DGAF_DISABLED, 1);
				ret = hs->drv_ops->drv_param_setting(hs->drv_data, conf->osu_iface,
	                                    PARAM_DGAF_DISABLED, 1);
				if (ret)
					return -1;	
				ret = hs->drv_ops->drv_param_setting(hs->drv_data, conf->osu_iface,
	                                    PARAM_PROXY_ARP, 1);

			    if (ret)
	        		return -1;
        	}
#if 0		
			//set MBSSID element to driver
			ret = hotspot_set_mbssid_ie(hs, conf);

			if (ret)
				return -1;
			
			//get non-transmitted profile length
			ret = hotspot_get_nontx_profile_len(hs, conf);

			if (ret) 
				return -1;

			//set hs indication ie to osu interface
			ret = hotspot_set_hs_indication_ie(hs, conf, 1);

			if (ret)
				return -1;
			
			printf("length=%d\n", conf->nontransmitted_len);
#endif		
		}
		else if (conf->legacy_osu_exist == 1){
			char asan_enable = 0;
			
			//20160531 wants hs daemon to start with even only 1 SSID, ignore if osu setting fail
			
			//set osu interface for asan disable
			ret = hotspot_set_osu_asan(hs, conf->osu_iface, &asan_enable, 1);			
			
			//get legacy osu SSID
			ret += hotspot_get_legacy_osu_ssid(hs, conf);

			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, "%s: single SSID case , please enable MBSSID if you need OSU function\n", __FUNCTION__);					
			}
			else
			{
				/*set OSEN DGAF=0 */
	            //ret = hotspot_param_setting(hs, conf, PARAM_DGAF_DISABLED, 1);
	            ret = hs->drv_ops->drv_param_setting(hs->drv_data, conf->osu_iface,
	                                    PARAM_DGAF_DISABLED, 0);

	            if (ret)
	                return -1;
	            ret = hs->drv_ops->drv_param_setting(hs->drv_data, conf->osu_iface,
	                                    PARAM_PROXY_ARP, 0);

	            if (ret)
	                return -1;
            }
		}
		else if (conf->legacy_osu_exist == 2) {
			//test mode, don't check osu interface
			//get legacy osu SSID
			char tmpSSID[] = "OSU";
			conf->legacy_osu_ssidlen = os_strlen(tmpSSID);
			conf->legacy_osu_ssid = os_zalloc(conf->legacy_osu_ssidlen);
			os_memcpy(conf->legacy_osu_ssid, tmpSSID, conf->legacy_osu_ssidlen);	
		}				
	
		/* Qos map */
		ret = hotspot_param_setting(hs, conf, PARAM_QOSMAP, conf->qosmap_enable);

		if (ret)
			return -1;
	
		/* BSS Load test */
		if (conf->qload_mode)
		{
			ret = hotspot_set_bss_load(hs, conf);

			if (ret)
				return -1;
		}
	}
			
	return ret;
}

static int hotspot_get_bssid(struct hotspot *hs, struct hotspot_conf *conf)
{
	int ret;
	size_t hessid_len = 6;

	ret = hs->drv_ops->drv_get_bssid(hs->drv_data, conf->iface, conf->hessid, &hessid_len);

	if (hessid_len != 6) {
		DBGPRINT(RT_DEBUG_ERROR, "hessid_len is not six\n");
		return -1;
	}

	return ret;
}

static int hotspot_init_ap_config(struct hotspot *hs, const char *confname)
{
	int ret = 0;
	FILE *file;
	char buf[256], *pos, *token, *token1;
	char tmpbuf[256], tmp1buf[256], tmp2buf[256];
	int line = 0, i = 0;
	struct hotspot_conf *conf;
	int varlen;
	struct anqp_capability *capability_info;
	struct nai_realm_data *realm_data = NULL, *realm_data_new = NULL;
	struct eap_method *eapmethod = NULL;	
	struct auth_param *authparam, *authparam_new;
	struct anqp_hs_capability *hs_capability_subtype;
	struct plmn *plmn_unit = NULL;
	struct proto_port_tuple *proto_port_unit = NULL; 
	u8 IsNAIRealmData = 0, IsPLMN = 0, IsProtoPort = 0, IsWanMetrics = 0;

	struct osu_providers *providers_list = NULL;
	u8 IsProviderList = 0;
	u8 qos_cnt = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s(%s)\n", __FUNCTION__, confname);

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);
	os_memset(tmp1buf, 0, 256);
	os_memset(tmp2buf, 0, 256);

	conf = os_zalloc(sizeof(struct hotspot_conf));

	if (!conf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available\n"));
		return -1;
	}

	os_strcpy(conf->confname, confname);

	dl_list_init(&conf->anqp_capability_list);
	dl_list_init(&conf->venue_name_list);
	dl_list_init(&conf->emergency_call_number_list);
	dl_list_init(&conf->network_auth_type_list);
	dl_list_init(&conf->oi_duple_list);
	dl_list_init(&conf->nai_realm_list);
	dl_list_init(&conf->plmn_list);
	dl_list_init(&conf->domain_name_list);

	/* Following are HS2.0 elemets list */
	dl_list_init(&conf->hs_capability_list);
	dl_list_init(&conf->operator_friendly_duple_list);
	dl_list_init(&conf->connection_capability_list);
	dl_list_init(&conf->nai_home_realm_name_query_list);
	dl_list_init(&conf->operating_class_list);
	dl_list_init(&conf->bss_transition_candi_list);
	dl_list_init(&conf->osu_providers_list);
	dl_list_init(&conf->icon_file_list);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, ("open configuration fail\n"));
		goto error;
	}

	while (hotspot_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		varlen = 0;
		token = strtok(pos, "=");
		if (token != NULL) {
			if (os_strcmp(token, "interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "Interface = %s\n", conf->iface);
			} else if (os_strcmp(token, "interworking") == 0) {
				token = strtok(NULL, "");
				conf->interworking = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "interworking = %d\n", conf->interworking);
			}else if (os_strcmp(token, "access_network_type") == 0) {
				token = strtok(NULL, "");
				conf->access_network_type = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "access_network_type = %d\n", conf->access_network_type);
			} else if (os_strcmp(token, "internet") == 0) {
				token = strtok(NULL, "");
				conf->internet = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "internet = %d\n", conf->internet);
			} else if (os_strcmp(token, "venue_group") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->venue_group = atoi(token);
					conf->is_venue_group = 1;
					DBGPRINT(RT_DEBUG_TRACE, "venue_group = %d\n", conf->venue_group);
				}
			} else if (os_strcmp(token, "venue_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->venue_type = atoi(token);
					conf->is_venue_type = 1;
					DBGPRINT(RT_DEBUG_TRACE, "venue_type = %d\n", conf->venue_type);
				}
			} else if (os_strcmp(token, "anqp_query") == 0) {
				token = strtok(NULL, "");
				conf->anqp_query = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "anqp_query = %d\n", conf->anqp_query);
			} else if (os_strcmp(token, "mih_support") == 0) {
				token = strtok(NULL, "");
				conf->mih_support = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "mih_support = %d\n", conf->mih_support);
			} else if (os_strcmp(token, "hessid") == 0) {
				token = strtok(NULL, ",");
				if (os_strcmp(token, "bssid") == 0) {
					hotspot_get_bssid(hs, conf);
					conf->is_hessid = 1;
				} else if (os_strcmp(token, "n/a") != 0) {
					os_strcpy(tmp1buf, token);
					token1 = strtok(tmp1buf, ":");
					i = 0;
					while (token1 != NULL) {
						//i = 0;
						AtoH(token1, &conf->hessid[i], 1);
						DBGPRINT(RT_DEBUG_TRACE, "hessid[%d] = 0x%02x\n", i, conf->hessid[i]);
						i++;
						token1 = strtok(NULL, ":");
					}
					if (i == 6)
						conf->is_hessid = 1;
				}
			} else if (os_strcmp(token, "roaming_consortium_oi") == 0) {
				struct oi_duple *oiduple;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						i = 0;
						varlen = 0;
						os_strcpy(tmp1buf, token);
						os_strcpy(tmp2buf, token);
						token1 = strtok(tmp1buf, "-");
						while (token1 != NULL) {
							varlen += 1;
							token1 = strtok(NULL, "-");
						}
						oiduple = os_zalloc(sizeof(struct oi_duple) + varlen);
						oiduple->length = varlen;

						token1 = strtok(tmp2buf, "-");
						while(token1 != NULL) {
							AtoH(token1, &oiduple->oi[i], 1);
							DBGPRINT(RT_DEBUG_TRACE, "roaming consortium_oi[%d] = 0x%02x\n", i, oiduple->oi[i]);
							token1 = strtok(NULL, "-");
							i++;
						}

						dl_list_add_tail(&conf->oi_duple_list, &oiduple->list);

						token = strtok(token + (varlen * 3), ",");
					}

					if (!dl_list_empty(&conf->oi_duple_list))
						conf->have_roaming_consortium_list = 1;
				}

			} else if (os_strcmp(token, "advertisement_proto_id") == 0) {
				token = strtok(NULL, ":");
				while (token != NULL) {
					conf->advertisement_proto_num++;
					token = strtok(NULL, ":");
				}
				if (conf->advertisement_proto_num > 0)
				{
					conf->advertisement_proto = os_zalloc(conf->advertisement_proto_num);
					if (!conf->advertisement_proto) {
						DBGPRINT(RT_DEBUG_ERROR, "Not available memory\n");
						goto error1;
					}
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					i = 0;
					
					while (token != NULL && i < conf->advertisement_proto_num) {
						conf->advertisement_proto[i] = atoi(token);
						DBGPRINT(RT_DEBUG_TRACE, "advertisement proto[%d] = %x\n", i, atoi(token));
						i++;
						token = strtok(NULL, ":");
					}
				}
			} else if (os_strcmp(token, "domain_name") == 0) {
				struct domain_name_field *dname_field;
				token = strtok(NULL, ";");
				
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						dname_field = os_zalloc(sizeof(struct domain_name_field) + os_strlen(token));
						dname_field->length = os_strlen(token);
						DBGPRINT(RT_DEBUG_TRACE, "length of domain name = %d\n", dname_field->length);
						DBGPRINT(RT_DEBUG_TRACE, "domain name:%s\n", token);
						os_strcpy(dname_field->domain_name, token);
						dl_list_add_tail(&conf->domain_name_list, &dname_field->list);
						token = strtok(NULL, ";");
					}

					if (!dl_list_empty(&conf->domain_name_list))
						conf->have_domain_name_list = 1;
				}

			} else if (os_strncmp(token, "venue_name", 10) == 0) {
				struct venue_name_duple *vname_duple;
				token = strtok(NULL, "%");
				if (os_strcmp(token, "n/a") != 0) {
					int max_venue_len = 255, copy_len = 0;
					unsigned char *venue_ptr;
					token = strtok(NULL, "%");
					vname_duple = os_zalloc(sizeof(struct venue_name_duple) + 256);

					token = strtok(tmpbuf, "=");
					token = strtok(NULL, "%");
					vname_duple->length += 3;
					os_strncpy(vname_duple->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of venue name = %s\n", token);
					token = strtok(NULL, "%");
					if (token[0] == '{')
					{
						token++;	
						venue_ptr = vname_duple->venue_name;
						
						while(1)
						{				
							if (token[os_strlen(token)-1] == '}')
							{
								if ((vname_duple->length)+(os_strlen(token)-1) > max_venue_len)
									copy_len = max_venue_len-vname_duple->length;
								else	
									copy_len = os_strlen(token)-1;
									
								vname_duple->length += copy_len;
								os_strncpy(venue_ptr, token, copy_len);
								venue_ptr += copy_len;
								break;
							}
							else
							{
								if ((vname_duple->length)+(os_strlen(token)+1) > max_venue_len)
								{
									copy_len = max_venue_len-vname_duple->length;
									vname_duple->length += copy_len;
									os_strncpy(venue_ptr, token, copy_len);
									venue_ptr += copy_len;
									break;
								}
								else	
								{
									copy_len = os_strlen(token)+1;
									vname_duple->length += copy_len;
									os_strncpy(venue_ptr, token, copy_len-1);
									venue_ptr += copy_len-1;
									*venue_ptr = 0x0a;
									venue_ptr += 1;
								}
							}
							if (hotspot_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
								os_strcpy(tmpbuf, pos);
								varlen = 0;
								token = strtok(pos, "=");
							}
						}
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, "venue name format error!! no { start\n");
					}
				
					DBGPRINT(RT_DEBUG_TRACE, "venue name:%s\n", token);
					dl_list_add_tail(&conf->venue_name_list, &vname_duple->list);
					conf->venue_name_nums++;

					if (!dl_list_empty(&conf->venue_name_list))
						conf->have_venue_name = 1;
				}
			} else if (os_strncmp(token, "network_auth_type", 17) == 0) {
				struct net_auth_type_unit *auth_type_unit;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");
				
					if (token)
						auth_type_unit = os_zalloc(sizeof(struct net_auth_type_unit) + os_strlen(token));
					else
						auth_type_unit = os_zalloc(sizeof(struct net_auth_type_unit));
					
					token = strtok(tmpbuf, "=");
					DBGPRINT(RT_DEBUG_TRACE, "%s\n", token);
					token = strtok(NULL, ",");
					auth_type_unit->net_auth_type_indicator = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "Network auth type indicator = %d\n", atoi(token));
					token = strtok(NULL, ",");

					if (token)
						auth_type_unit->re_direct_URL_len = os_strlen(token);
					else
						auth_type_unit->re_direct_URL_len = 0;				

					if (token) {
						os_strcpy(auth_type_unit->re_direct_URL, token);
						DBGPRINT(RT_DEBUG_TRACE, "re direct URL = %s\n", token);
					}

					dl_list_add_tail(&conf->network_auth_type_list, &auth_type_unit->list);
					conf->network_auth_type_nums++;

					if (!dl_list_empty(&conf->network_auth_type_list))
						conf->have_network_auth_type = 1;
				}
			} else if (os_strcmp(token, "ipv4_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->ipv4_address_type = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "ipv4_type = %d\n", conf->ipv4_address_type);
					conf->have_ip_address_type = 1;
				}
			} else if (os_strcmp(token, "ipv6_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->ipv6_address_type = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "ipv6_type = %d\n", conf->ipv6_address_type);
					conf->have_ip_address_type = 1;
				}
			} else if (os_strncmp(token, "osu_providers_list", 18) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsProviderList = 1;
						providers_list = os_zalloc(sizeof(*providers_list));
						/* 5:osu_server_uri_len + osu_method_list_len + osu_nai_len + icon_avail_len */
						/* 4:osu_friendly_name_len + osu_service_len */
						providers_list->osu_providers_list_field_len = 5 + 4; 
						dl_list_init(&providers_list->osu_friendly_name_list);
						dl_list_init(&providers_list->osu_method_list);
						dl_list_init(&providers_list->icon_list);
						dl_list_init(&providers_list->osu_nai_list);
						dl_list_init(&providers_list->osu_service_desc_list);
						dl_list_add_tail(&conf->osu_providers_list, &providers_list->list);
					}
				} else
					conf->osu_providers_list_nums = 0;
			} else if (os_strcmp(token, "osu_friendly_name") == 0) {
				struct osu_friendly_name *friendly_name;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					friendly_name = os_zalloc(sizeof(struct osu_friendly_name) + os_strlen(token));					
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					friendly_name->len += 3;
					os_strncpy(friendly_name->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of osu_friendly_name = %s\n", token);
					token = strtok(NULL, ":");
					friendly_name->len += os_strlen(token);
					os_strncpy(friendly_name->osu_friendly_name_value, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_friendly_name:%s\n", token);
					providers_list->osu_providers_list_field_len += 4+os_strlen(token);
					dl_list_add_tail(&providers_list->osu_friendly_name_list, &friendly_name->list);
					providers_list->osu_friendly_name_len += 4+os_strlen(token);
				}
			} else if (os_strcmp(token, "osu_server_uri") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					providers_list->osu_server_uri = os_zalloc(os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_server_uri:%s\n", token);
					os_memcpy(providers_list->osu_server_uri, token, os_strlen(token));
					providers_list->osu_server_uri_len = os_strlen(token);
					providers_list->osu_providers_list_field_len += os_strlen(token);
				}
			} else if (os_strcmp(token, "osu_method") == 0) {
				struct osu_method *method;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					method = os_zalloc(sizeof(struct osu_method));			
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, " ");
					method->osu_method_value = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "osu_method = %d\n", atoi(token));
					dl_list_add_tail(&providers_list->osu_method_list, &method->list);
					providers_list->osu_method_len++;
					providers_list->osu_providers_list_field_len += 1;
				}
			} else if (os_strcmp(token, "icon") == 0) {
				struct icon_available *icon;
				char *type, *name;
				u8 *lang;
				int weight, height;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {			
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					weight = atoi(token);
					token = strtok(NULL, ":");
					height = atoi(token);
					lang = strtok(NULL, ":");
					type = strtok(NULL, ":");
					name = strtok(NULL, ":");
					DBGPRINT(RT_DEBUG_TRACE, "typelen=%d, filelen=%d\n", os_strlen(type), os_strlen(name));
					icon = os_zalloc(sizeof(*icon)+os_strlen(type)+os_strlen(name));		
					
					icon->weight = weight;
					icon->height = height;
					os_memcpy(icon->language, lang, 3);
					os_memcpy(icon->icon_buf, type, os_strlen(type));		
					icon->type_len = os_strlen(type);
					
					os_memcpy(&icon->icon_buf[icon->type_len], name, os_strlen(name));	
					icon->filename_len = os_strlen(name);
					dl_list_add_tail(&providers_list->icon_list, &icon->list);
					providers_list->icon_len += 9+os_strlen(type)+os_strlen(name);
					providers_list->osu_providers_list_field_len += 9+os_strlen(type)+os_strlen(name); //providers_list->icon_len;
				}
			} else if (os_strcmp(token, "osu_nai") == 0) {
				struct osu_nai *nai;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {			
					nai = os_zalloc(sizeof(*nai) + os_strlen(token));
					nai->len = os_strlen(token);
					os_memcpy(nai->osu_nai_value, token, os_strlen(token));	
					
					DBGPRINT(RT_DEBUG_TRACE, "osu_nai = %s\n", token);
					providers_list->osu_providers_list_field_len += nai->len;
					dl_list_add_tail(&providers_list->osu_nai_list, &nai->list);
					providers_list->osu_nai_len += nai->len;
				}
			} else if (os_strcmp(token, "osu_service_desc") == 0) {
				struct osu_service_desc *service_desc;
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {			
					service_desc = os_zalloc(sizeof(*service_desc) + os_strlen(token));					
					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ":");
					service_desc->len += 3;
					os_strncpy(service_desc->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of osu_service_desc = %s\n", token);
					token = strtok(NULL, ":");
					service_desc->len += os_strlen(token);
					os_strncpy(service_desc->osu_service_desc_value, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "osu_service_desc:%s\n", token);
					providers_list->osu_providers_list_field_len += 4+os_strlen(token);
					dl_list_add_tail(&providers_list->osu_service_desc_list, &service_desc->list);
					providers_list->osu_service_len += 4+os_strlen(token);
				}
			} else if ((os_strcmp(token, "}") == 0) && IsProviderList) {
				conf->osu_providers_list_nums++;
				conf->have_osu_providers_list = 1;
				IsProviderList = 0;
				DBGPRINT(RT_DEBUG_TRACE, "total len=%d\n", providers_list->osu_providers_list_field_len);
				DBGPRINT(RT_DEBUG_TRACE, "list num:%d\n", conf->osu_providers_list_nums);
			} else if (os_strncmp(token, "nai_realm_data", 14) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsNAIRealmData = 1;
						realm_data = os_zalloc(sizeof(*realm_data));
						realm_data->nai_realm_data_field_len = 3;
						realm_data->nai_realm_encoding = 0;
						dl_list_init(&realm_data->eap_method_list);
					}
				} else
					conf->nai_realm_data_nums = 0;
			} else if ((os_strcmp(token, "}") == 0) && IsNAIRealmData) {
				dl_list_for_each(eapmethod, &realm_data_new->eap_method_list, 
										struct eap_method, list) {
					realm_data_new->nai_realm_data_field_len += 1;
					realm_data_new->nai_realm_data_field_len += eapmethod->len;
				}
				conf->nai_realm_data_nums++;
				IsNAIRealmData = 0;
			} else if (os_strcmp(token, "nai_realm") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "nai_realm:%s\n", token);
				varlen = os_strlen(token);
				realm_data_new = os_realloc(realm_data, sizeof(*realm_data) + varlen);
				dl_list_init(&realm_data_new->eap_method_list);
				os_memcpy(realm_data_new->nai_realm, token, varlen);
				realm_data_new->nai_realm_len = varlen;
				realm_data_new->nai_realm_data_field_len += varlen;
				conf->have_nai_realm_list = 1;
				dl_list_add_tail(&conf->nai_realm_list, &realm_data_new->list);
			} else if (os_strncmp(token, "eap_method", 10) == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "eap_method:%s\n", token);
				if (os_strcmp(token, "eap-ttls") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_TTLS;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-tls") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_TLS;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-sim") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_SIM;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				} else if (os_strcmp(token, "eap-aka") == 0) {
					realm_data_new->eap_method_count++;
					eapmethod = os_zalloc(sizeof(*eapmethod));
					eapmethod->len = 2;
					eapmethod->eap_method = EAP_AKA;
					dl_list_init(&eapmethod->auth_param_list);
					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);
				}
			} else if (os_strncmp(token, "auth_param", 9) == 0) {
				DBGPRINT(RT_DEBUG_TRACE, "auth_param:\n");
				token = strtok(NULL, ":");
				authparam = os_zalloc(sizeof(*authparam));
				eapmethod->auth_param_count++;
				switch (atoi(token)) {
				case EXPANDED_EAP_METHOD:
					authparam->id = EXPANDED_EAP_METHOD;
					authparam->len = 7;
					varlen = 7;
					eapmethod->len += 9;
					DBGPRINT(RT_DEBUG_TRACE, "ID = EXPANDED_EAP_METHOD\n")
					break;
				case NON_EAP_INNER_AUTH_TYPE:
					authparam->id = NON_EAP_INNER_AUTH_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = NON_EAP_INNER_AUTH_TYPE\n")
					break;
				case INNER_AUTH_EAP_METHOD_TYPE:
					authparam->id = INNER_AUTH_EAP_METHOD_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = INNER_AUTH_EAP_METHOD_TYPE\n")
					break;
				case EXPANDED_INNER_EAP_METHOD:
					authparam->id = EXPANDED_INNER_EAP_METHOD;
					authparam->len = 7;
					varlen = 7;
					eapmethod->len += 9;
					DBGPRINT(RT_DEBUG_TRACE, "ID = EXPANDED_INNER_EAP_METHOD\n")
					break;
				case CREDENTIAL_TYPE:
					authparam->id = CREDENTIAL_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = CREDENTIAL_TYPE\n")
					break;
				case TUNNELED_EAP_METHOD_CREDENTIAL_TYPE:
					authparam->id = TUNNELED_EAP_METHOD_CREDENTIAL_TYPE;
					authparam->len = 1;
					varlen = 1;
					eapmethod->len += 3;
					DBGPRINT(RT_DEBUG_TRACE, "ID = TUNNELED_EAP_METHOD_CREDENTIAL_TYPE\n")
					break;
				case VENDOR_SPECIFIC:
					DBGPRINT(RT_DEBUG_TRACE, "ID = VENDOR_SPECIFIC\n")
					/* TODO: varlen is variable */
				default:
					DBGPRINT(RT_DEBUG_ERROR, "Unknown authentication parameter types\n");
					break;
				}
			
				authparam_new = os_realloc(authparam, sizeof(*authparam) + varlen);
				token = strtok(NULL, ":");
				DBGPRINT(RT_DEBUG_TRACE, "Value = %d\n", atoi(token));
				while (token) {
					if (varlen == 1) {
						*authparam_new->auth_param_value = atoi(token);
					} else if (varlen == 7) {
						/* TODO */
					}
					token = strtok(NULL, ":");
				}	

				dl_list_add_tail(&eapmethod->auth_param_list, &authparam_new->list);

			} else if (os_strncmp(token, "op_friendly_name", 16) == 0) {
				struct operator_name_duple *op_name_duple;
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");
					op_name_duple = os_zalloc(sizeof(struct operator_name_duple) + os_strlen(token));

					token = strtok(tmpbuf, "=");
					token = strtok(NULL, ",");
					op_name_duple->length += 3;
					os_strncpy(op_name_duple->language, token, 3);
					DBGPRINT(RT_DEBUG_TRACE, "Language of operator friendly name = %s\n", token);
					token = strtok(NULL, ",");
					op_name_duple->length += os_strlen(token);
					os_strncpy(op_name_duple->operator_name, token, os_strlen(token));
					DBGPRINT(RT_DEBUG_TRACE, "operator friendly name:%s , strlen:%d\n", token,os_strlen(token));
					dl_list_add_tail(&conf->operator_friendly_duple_list, &op_name_duple->list);
					conf->op_friendly_name_nums++;

					if (!dl_list_empty(&conf->operator_friendly_duple_list))
						conf->have_operator_friendly_name = 1;
				}
			} else if (os_strncmp(token, "plmn", 4) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					if (os_strcmp(token, "{") == 0) {
						IsPLMN = 1;
						plmn_unit = os_zalloc(sizeof(*plmn_unit));
					}
				} else
					conf->plmn_nums = 0;
			} else if ((os_strcmp(token, "}") == 0) && IsPLMN) {
				dl_list_add_tail(&conf->plmn_list, &plmn_unit->list);
				conf->plmn_nums++;
				if (!dl_list_empty(&conf->plmn_list)) {
					conf->have_3gpp_network_info = 1;
				}
				IsPLMN = 0;
			} else if (os_strcmp(token, "mcc") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "mcc = %s\n", token);
				for (i = 0; i < 3; i++) {
					plmn_unit->mcc[i] = token[i] - '0';
					DBGPRINT(RT_DEBUG_TRACE, "mcc[%d] = %d\n", i, plmn_unit->mcc[i]);
				}
			} else if (os_strcmp(token, "mnc") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "mnc = %s\n", token);
				for (i = 0; i < 3; i++) { 
					if (i == 2 && os_strlen(token) == 2)
						plmn_unit->mnc[i] = 0x0f;
					else
						plmn_unit->mnc[i] = token[i] - '0';
					DBGPRINT(RT_DEBUG_TRACE, "mnc[%d] = %d\n", i, plmn_unit->mnc[i]);
				}

			} else if (os_strncmp(token, "proto_port", 10) == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "{") == 0) {
					IsProtoPort = 1;
					proto_port_unit = os_zalloc(sizeof(*proto_port_unit));	
				}
			} else if ((os_strcmp(token, "}") == 0) && IsProtoPort) {
				dl_list_add_tail(&conf->connection_capability_list, &proto_port_unit->list);
				conf->proto_port_nums++;
				if (!dl_list_empty(&conf->connection_capability_list))
					conf->have_connection_capability_list = 1;
				IsProtoPort = 0;
			} else if (os_strcmp(token, "operating_class") == 0) {
				struct operating_class_unit *operating_class;
				token = strtok(NULL, ",");
				
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						operating_class = os_zalloc(sizeof(struct operating_class_unit));
						DBGPRINT(RT_DEBUG_TRACE, "operating class:%s\n", token);
						operating_class->op_class = atoi(token);
						dl_list_add_tail(&conf->operating_class_list, &operating_class->list);
						token = strtok(NULL, ",");
					}

					if (!dl_list_empty(&conf->operating_class_list))
						conf->have_operating_class = 1;
				}
			} else if (os_strcmp(token, "ip_protocol") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->ip_protocol = atoi(token);
			} else if (os_strcmp(token, "port") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->port = atoi(token);
			} else if (os_strcmp(token, "status") == 0) {
				token = strtok(NULL, "");
				proto_port_unit->status = atoi(token);
			} else if (os_strcmp(token, "wan_metrics") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "{") == 0)
					IsWanMetrics = 1;
			} else if ((os_strcmp(token, "}") == 0) && IsWanMetrics) {
				conf->wan_metrics_nums++;
				conf->have_wan_metrics = 1;
				IsWanMetrics = 0;
			} else if (os_strcmp(token, "link_status") == 0) {
				token = strtok(NULL, "");
				conf->metrics.link_status = atoi(token);
			} else if (os_strcmp(token, "at_capacity") == 0) {
				token = strtok(NULL, "");
				conf->metrics.at_capacity = atoi(token);
			} else if (os_strcmp(token, "dl_speed") == 0) {
				token = strtok(NULL, "");
				conf->metrics.dl_speed = atoi(token);
			} else if (os_strcmp(token, "ul_speed") == 0) {
				token = strtok(NULL, "");
				conf->metrics.ul_speed = atoi(token);
			} else if (os_strcmp(token, "dl_load") == 0) {
				token = strtok(NULL, "");
				conf->metrics.dl_load = atoi(token);
			} else if (os_strcmp(token, "up_load") == 0) {
				token = strtok(NULL, "");
				conf->metrics.ul_load = atoi(token);
			} else if (os_strcmp(token, "lmd") == 0) {
				token = strtok(NULL, "");
				conf->metrics.lmd = atoi(token);
			} else if (os_strcmp(token, "preferred_candi_list_included") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "preferred candi list included = %s\n", token);
				conf->preferred_candi_list_included = atoi(token);
			} else if (os_strcmp(token, "abridged") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "abridged = %s\n", token);
				conf->abridged = atoi(token);
			} else if (os_strcmp(token, "disassociation_imminent") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "disassociationimminent = %s\n", token);
				conf->disassociation_imminent = atoi(token);
			} else if (os_strcmp(token, "bss_termination_included") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "bss termination included = %s\n", token);
				conf->bss_termination_included = atoi(token);
			} else if (os_strcmp(token, "ess_disassociation_imminent") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "ess disassociation imminent = %s\n", token);
				conf->ess_disassociation_imminent = atoi(token);
			} else if (os_strcmp(token, "disassociation_timer") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "disassociation timer = %s\n", token);
				conf->disassociation_timer = atoi(token);
			} else if (os_strcmp(token, "validity_interval") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "validity interval = %s\n", token);
				conf->validity_interval = atoi(token);
			} else if (os_strcmp(token, "bss_termination_duration") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->have_bss_termination_duration = 1;
					token = strtok(NULL, ",");
					DBGPRINT(RT_DEBUG_TRACE, "bss termination tsf = %s\n", token);
					conf->bss_termination_tsf = atoi(token);
					token = strtok(NULL, "");
					DBGPRINT(RT_DEBUG_TRACE, "bss termination duration = %s\n", token);
					conf->bss_termination_duration = atoi(token);
				}
			} else if (os_strcmp(token, "session_information_url") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "session information url = %s\n", token);
				conf->have_session_info_url = 1;
				conf->session_info_url_len = os_strlen(token);
				conf->session_info_url = os_zalloc(conf->session_info_url_len);
				os_memcpy(conf->session_info_url, token, conf->session_info_url_len);
			} else if (os_strcmp(token, "bss_transisition_candi_list_preferences") == 0) {
				token = strtok(NULL, ",");
				if (os_strcmp(token, "n/a") != 0) {
					token = strtok(NULL, ",");

					while (token != NULL) {
						struct bss_transition_candi_preference_unit *preference_unit;
						preference_unit = os_zalloc(sizeof(*preference_unit));
						preference_unit->preference = atoi(token);
						dl_list_add_tail(&conf->bss_transition_candi_list, &preference_unit->list);
						conf->have_bss_transition_candi_list = 1;
						token = strtok(NULL, ",");
					}
				}
			} else if (os_strcmp(token, "timezone") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "time zone = %s\n", token);
				conf->time_zone_len = os_strlen(token);
				if (conf->time_zone_len > 0) {
					char time_zone[50];
					conf->time_zone = os_zalloc(conf->time_zone_len);
					conf->have_time_zone = 1;
					os_memcpy(conf->time_zone, token, conf->time_zone_len);
					conf->time_zone[conf->time_zone_len] = '\0';
					/* Set time zone in TZ Environement */
					sprintf(time_zone, "TZ=%s", token);
					putenv(time_zone);
				}
			} else if (os_strcmp(token, "dgaf_disabled") == 0) {
				token = strtok(NULL, "");
				conf->DGAF_disabled = atoi(token);
			} else if (os_strcmp(token, "proxy_arp") == 0) {
				token = strtok(NULL, "");
				conf->proxy_arp = atoi(token);
			} else if (os_strcmp(token, "l2_filter") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "l2_filter = %s\n", token);
				conf->l2_filter = atoi(token);
			} else if (os_strcmp(token, "icmpv4_deny") == 0) {
				token = strtok(NULL, "");
				DBGPRINT(RT_DEBUG_TRACE, "icmpv4_deny = %s\n", token);
				conf->icmpv4_deny = atoi(token);
			} else if (os_strcmp(token, "p2p_cross_connect_permitted") == 0) {
				token = strtok(NULL, "");
				conf->p2p_cross_connect_permitted = atoi(token);
			} else if (os_strcmp(token, "mmpdu_size") == 0) {
				token = strtok(NULL, "");
				conf->mmpdu_size = atoi(token);
			} else if (os_strcmp(token, "external_anqp_server_test") == 0) {
				token = strtok(NULL, "");
				conf->external_anqp_server_test = atoi(token);
			} else if (os_strcmp(token, "gas_cb_delay") == 0) {
				token = strtok(NULL, "");
				conf->gas_cb_delay = atoi(token);
			} else if (os_strcmp(token, "hs2_openmode_test") == 0) {
				token = strtok(NULL, "");
				conf->hs2_openmode_test = atoi(token);
			} else if (os_strcmp(token, "anonymous_nai") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					DBGPRINT(RT_DEBUG_TRACE, "anonymous_nai = %s\n", token);
					conf->have_anonymous_nai = 1;
					conf->anonymous_nai_len = os_strlen(token);
					conf->anonymous_nai = os_zalloc(conf->anonymous_nai_len);
					os_memcpy(conf->anonymous_nai, token, conf->anonymous_nai_len);
				}
			}	
			else if (os_strcmp(token, "osu_interface") == 0) {
				token = strtok(NULL, "");
				os_strcpy(conf->osu_iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "OSU Interface = %s\n", conf->osu_iface);
			}
			else if (os_strcmp(token, "legacy_osu") == 0) {
				token = strtok(NULL, "");
				conf->legacy_osu_exist = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "Legacy_osu = %d\n", conf->legacy_osu_exist);
			}
			else if (os_strcmp(token, "icon_path") == 0) {
				token = strtok(NULL, "");
				conf->have_iconfile_path = 1;
				conf->iconfile_path_len = os_strlen(token);
				conf->iconfile_path = os_zalloc(conf->iconfile_path_len);
				os_memcpy(conf->iconfile_path, token, conf->iconfile_path_len);
				DBGPRINT(RT_DEBUG_TRACE, "icon path = %s, len = %d\n", token, conf->iconfile_path_len);
			} 
//JERRY DBG
#if 0
			else if (os_strcmp(token, "icon_tag") == 0) {
                char icon_id1[] = "/etc_ro/icon_blue_zxx_default.png";
				char icon_id1_1[] = "/etc_ro/icon_blue_eng_default.png";
                char icon_id2[] = "/etc_ro/wifi-abgn-logo_270x73.png";
                //char osu_icon_name[] = "/etc_ro/icon_blue_zxx.png";
				//char osu_icon_name2[] = "/etc_ro/icon_blue_eng.png";
				char osu_icon_name[] = "/etc_ro/icon_red_zxx.png";
                char osu_icon_name2[] = "/etc_ro/icon_red_eng.png";


                token = strtok(NULL, "");
                conf->icon_tag = atoi(token);
                //DBGPRINT(RT_DEBUG_TRACE, "icon_tag = %d\n", conf->icon_tag);
                printf ("icon_tag = %d\n", conf->icon_tag);
				if (conf->icon_tag == 1) {
                    printf ("get default icon name: %s\n", icon_id1);
                    mtk_copy_file (icon_id1, osu_icon_name);
					printf ("get default icon name 2: %s\n", icon_id1_1);
                    mtk_copy_file (icon_id1_1, osu_icon_name2);
                    //system ("cp /etc_ro/WiFi-alliancelogo_3D.default.png /etc_ro/WiFi-alliancelogo_3D.png");
                } else if (conf->icon_tag == 2) {
                    printf ("get default icon name: %s\n", icon_id2);
                    mtk_copy_file (icon_id2, osu_icon_name);
					mtk_copy_file (icon_id2, osu_icon_name2);
                    //system ("cp /etc_ro/wifi-abgn-logo_270x73.png /etc_ro/WiFi-alliancelogo_3D.png");
                }
			}
#endif 
			else if (os_strcmp(token, "qosmap") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qosmap_enable = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qosmap_enable = %d\n", conf->qosmap_enable);
				}
			} else if (os_strcmp(token, "dscp_range") == 0) {
				qos_cnt = 0;
				token = strtok(NULL, ":");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						conf->dscp_range[qos_cnt] = atoi(token);
						token = strtok(NULL, ":");
						conf->dscp_range[qos_cnt++] |= (atoi(token) << 8);
						token = strtok(NULL, ":");
					}
				}
			} else if (os_strcmp(token, "dscp_exception") == 0) {
				qos_cnt = 0;
				token = strtok(NULL, ":");
				if (os_strcmp(token, "n/a") != 0) {
					while (token != NULL) {
						conf->dscp_exception[qos_cnt] = atoi(token);
						token = strtok(NULL, ":");
						conf->dscp_exception[qos_cnt++] |= (atoi(token) << 8);
						token = strtok(NULL, ":");
						conf->dscp_field++;
					}
				}
			} else if (os_strcmp(token, "qload_test") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_mode = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload mode = %d\n", conf->qload_mode);
				}
			} else if (os_strcmp(token, "qload_cu") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_cu = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload_cu = %d\n", conf->qload_cu);
				}
			} else if (os_strcmp(token, "qload_sta_cnt") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "n/a") != 0) {
					conf->qload_sta_cnt = atoi(token);
					DBGPRINT(RT_DEBUG_TRACE, "qload_sta_cnt = %d\n", conf->qload_sta_cnt);
				}
			} 
		}
	}

	conf->have_anqp_capability_list = 1;
	conf->have_hs_capability_list = 1;
	
	capability_info = os_zalloc(sizeof(*capability_info));
	capability_info->info_id = ANQP_CAPABILITY;
	dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);

	if (conf->have_venue_name) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = VENUE_NAME_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_network_auth_type) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = NETWORK_AUTH_TYPE_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_roaming_consortium_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = ROAMING_CONSORTIUM_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_ip_address_type) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = IP_ADDRESS_TYPE_AVAILABILITY_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_nai_realm_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = NAI_REALM_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
		conf->have_nai_home_realm_query = 1;
	}

	if (conf->have_3gpp_network_info) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = ThirdGPP_CELLULAR_NETWORK_INFO;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	if (conf->have_domain_name_list) {
		capability_info = os_zalloc(sizeof(*capability_info));
		capability_info->info_id = DOMAIN_NAME_LIST;
		dl_list_add_tail(&conf->anqp_capability_list, &capability_info->list);
	}

	/* Following are HS2.0 capability list */
	hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
	hs_capability_subtype->subtype = HS_CAPABILITY;
	dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);

	if (conf->have_operator_friendly_name) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OPERATOR_FRIENDLY_NAME;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_wan_metrics) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = WAN_METRICS;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_connection_capability_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = CONNECTION_CAPABILITY;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_nai_realm_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = NAI_HOME_REALM_QUERY;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_operating_class) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OPERATING_CLASS;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_osu_providers_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = OSU_PROVIDE_LIST;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}
	
	if (conf->have_anonymous_nai) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
		hs_capability_subtype->subtype = ANONYMOUS_NAI;
		dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
	}

	if (conf->have_osu_providers_list) {
		hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
        hs_capability_subtype->subtype = ICON_REQUEST;
        dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
        
        hs_capability_subtype = os_zalloc(sizeof(*hs_capability_subtype));
        hs_capability_subtype->subtype = ICON_BINARY_FILE;
        dl_list_add_tail(&conf->hs_capability_list, &hs_capability_subtype->list);
    }
	
	dl_list_add_tail(&hs->hs_conf_list, &conf->list);

	fclose(file);
	
	/* Set interworking capability to driver */
	if (conf->interworking)
	{
		char interworking_enable=1;
		hotspot_set_interworking_enable(hs, conf->iface, &interworking_enable, 1);		
	}
	else
	{
		char interworking_enable=0;
		hotspot_set_interworking_enable(hs, conf->iface, &interworking_enable, 1);
	}
	/* Set parameter to driver */
	if (conf->interworking)
		ret = hotspot_init_param_setting(hs, conf);


	return ret;

error1:
	fclose(file);
error:
	os_free(conf);
	return -1;
}

static int hotspot_deinit_config(struct hotspot *hs, struct hotspot_conf *conf)
{
	int ret = 0;

	if (conf->have_anqp_capability_list) {
		struct anqp_capability *capability_info, *capability_info_tmp;
		dl_list_for_each_safe(capability_info, capability_info_tmp, &conf->anqp_capability_list,
										struct anqp_capability, list) {
			dl_list_del(&capability_info->list);
			os_free(capability_info);
		}
	}

	if (conf->have_venue_name) {
		struct venue_name_duple *vname_duple, *vname_duple_tmp;
		dl_list_for_each_safe(vname_duple, vname_duple_tmp, &conf->venue_name_list, 
										struct venue_name_duple, list) {
			dl_list_del(&vname_duple->list);
			os_free(vname_duple);
		}
	}

	if (conf->have_roaming_consortium_list) {
		struct oi_duple *oiduple, *oiduple_tmp;
		dl_list_for_each_safe(oiduple, oiduple_tmp, &conf->oi_duple_list,
										struct oi_duple, list) {
			dl_list_del(&oiduple->list);
			os_free(oiduple);
		}
	}


	if (conf->have_nai_realm_list) {
		struct nai_realm_data *nai_realm, *nai_realm_tmp;
		struct eap_method *eapmethod, *eapmethod_tmp;
		struct auth_param *authparam, *authparam_tmp;
		dl_list_for_each_safe(nai_realm, nai_realm_tmp, &conf->nai_realm_list,
										struct nai_realm_data, list) {
			dl_list_del(&nai_realm->list);
			dl_list_for_each_safe(eapmethod, eapmethod_tmp, &nai_realm->eap_method_list,
										struct eap_method, list) {

				dl_list_del(&eapmethod->list);
				dl_list_for_each_safe(authparam, authparam_tmp, &eapmethod->auth_param_list,
										struct auth_param, list) {
					dl_list_del(&authparam->list);
					os_free(authparam);
				}
				
				os_free(eapmethod);
			}
			os_free(nai_realm);
		
		}
	}

	if (conf->have_3gpp_network_info) {
		struct plmn *plmn_unit, *plmn_unit_tmp;
		dl_list_for_each_safe(plmn_unit, plmn_unit_tmp, &conf->plmn_list,
								struct plmn, list) {
			dl_list_del(&plmn_unit->list);
			os_free(plmn_unit);
		}
	}

	if (conf->have_domain_name_list) {
		struct domain_name_field *dname_field, *dname_field_tmp;
		dl_list_for_each_safe(dname_field, dname_field_tmp, &conf->domain_name_list,
										struct domain_name_field, list) {
			dl_list_del(&dname_field->list);
			os_free(dname_field);
		}
	}

	if (conf->have_network_auth_type) {
		struct net_auth_type_unit *auth_type_unit, *auth_type_unit_tmp;
		dl_list_for_each_safe(auth_type_unit, auth_type_unit_tmp, &conf->network_auth_type_list,
										struct net_auth_type_unit, list) {
			dl_list_del(&auth_type_unit->list);
			os_free(auth_type_unit);
		}
	}

	/* Following are hotspot2.0 elements */
	if (conf->have_hs_capability_list) {
		struct anqp_hs_capability *hs_capability_subtype, *hs_capability_subtype_tmp;
		dl_list_for_each_safe(hs_capability_subtype, hs_capability_subtype_tmp,
							  &conf->hs_capability_list, struct anqp_hs_capability, list) {
			dl_list_del(&hs_capability_subtype->list);
			os_free(hs_capability_subtype);
		}
	}

	if (conf->have_operator_friendly_name) {
		struct operator_name_duple *op_name_duple, *op_name_duple_tmp;
		dl_list_for_each_safe(op_name_duple, op_name_duple_tmp, &conf->operator_friendly_duple_list, 
										struct operator_name_duple, list) {
			dl_list_del(&op_name_duple->list);
			os_free(op_name_duple);
		}
	}

	if (conf->have_connection_capability_list) {
		struct proto_port_tuple *proto_port, *proto_port_tmp;
		dl_list_for_each_safe(proto_port, proto_port_tmp, &conf->connection_capability_list, 
										struct proto_port_tuple, list) {
			dl_list_del(&proto_port->list);
			os_free(proto_port);
		}
	}

	if (conf->have_operating_class) {
		struct operating_class_unit *operating_class, *operating_class_tmp;
		dl_list_for_each_safe(operating_class, operating_class_tmp,
						&conf->operating_class_list, struct operating_class_unit, list) {

			dl_list_del(&operating_class->list);
			os_free(operating_class);
		}
	}

	if (conf->have_osu_providers_list) {
		struct osu_providers *providers_list, *providers_list_tmp;
		struct osu_friendly_name *friendly_name_list, *friendly_name_list_tmp;
		struct osu_method *method_list, *method_list_tmp;
		struct icon_available *icon_list, *icon_list_tmp;
		struct osu_nai *nai_list, *nai_list_tmp;
		struct osu_service_desc *service_desc_list, *service_desc_list_tmp;
		
		dl_list_for_each_safe(providers_list, providers_list_tmp,
						&conf->osu_providers_list, struct osu_providers, list) {
			dl_list_del(&providers_list->list);
			
			dl_list_for_each_safe(friendly_name_list, friendly_name_list_tmp, &providers_list->osu_friendly_name_list,
										struct osu_friendly_name, list) {
				dl_list_del(&friendly_name_list->list);
				os_free(friendly_name_list);
			}

			dl_list_for_each_safe(method_list, method_list_tmp, &providers_list->osu_method_list,
										struct osu_method, list) {
				dl_list_del(&method_list->list);
				os_free(method_list);
			}
			
			dl_list_for_each_safe(icon_list, icon_list_tmp, &providers_list->icon_list,
										struct icon_available, list) {
				dl_list_del(&icon_list->list);
				os_free(icon_list);
			}
			
			dl_list_for_each_safe(nai_list, nai_list_tmp, &providers_list->osu_nai_list,
										struct osu_nai, list) {
				dl_list_del(&nai_list->list);
				os_free(nai_list);
			}
			
			dl_list_for_each_safe(service_desc_list, service_desc_list_tmp, &providers_list->osu_service_desc_list,
										struct osu_service_desc, list) {
				dl_list_del(&service_desc_list->list);
				os_free(service_desc_list);
			}
			
			if (providers_list->osu_server_uri_len != 0)
				os_free(providers_list->osu_server_uri);
				
			os_free(providers_list);
		}
	}
	
	if (conf->have_anonymous_nai)
		os_free(conf->anonymous_nai);
	
	if (conf->have_iconfile_path)
		os_free(conf->iconfile_path);
		
	if (conf->have_session_info_url)
		os_free(conf->session_info_url);

	if (conf->have_bss_transition_candi_list) {
		struct bss_transition_candi_preference_unit *preference_unit, *preference_unit_tmp;
		dl_list_for_each_safe(preference_unit, preference_unit_tmp, &conf->bss_transition_candi_list, 
				struct bss_transition_candi_preference_unit, list) {
			dl_list_del(&preference_unit->list);
			os_free(preference_unit);
		}
	}

	if (conf->have_time_zone)
		os_free(conf->time_zone);

	return ret;
}

int hotspot_init_all_config(struct hotspot *hs, const char *confname)
{
	FILE *file;
	char buf[256], *pos, *token, *tokentmp;
	int line = 0, ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	os_memset(buf, 0, 256);

	file = fopen(confname, "r");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, ("reading configuration fail, maybe do not have this file\n"));
		goto error;
	}

	dl_list_init(&hs->hs_conf_list);

	while (hotspot_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		token = strtok(pos, "=");
		
		if (token != NULL) {
			if (os_strcmp(token, "conf_list") == 0) {
				token = strtok(NULL, ";");
				while (token != NULL) {
					tokentmp = token + os_strlen(token) + 1;
					if (hs->opmode == OPMODE_STA)
						ret = hotspot_init_sta_config(hs, token);
					else
						ret = hotspot_init_ap_config(hs, token);

					token = strtok(tokentmp, ";");
				}
			}
		}
	}

	fclose(file);

	/* init location IE */	
	os_memset(hs->civic_IE, 0, LOCATION_IE_LEN);
	hs->civic_IE_len = 0;
	os_memset(hs->lci_IE, 0, LOCATION_IE_LEN);
	hs->lci_IE_len = 0;
	os_memset(hs->public_id_uri, 0, LOCATION_IE_LEN);	
	hs->public_id_uri_len = 0;

error:
	return ret;
}

int hotspot_deinit_all_config(struct hotspot *hs)
{
	struct hotspot_conf *conf, *conf_tmp;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	dl_list_for_each_safe(conf, conf_tmp, &hs->hs_conf_list, struct hotspot_conf, list) {
		hotspot_deinit_config(hs, conf);
		dl_list_del(&conf->list);
		os_free(conf);
	}

	return 0;
}

int hotspot_deinit(struct hotspot *hs)
{
	int ret = 0;
	//struct hs_peer_entry *peer_entry, *peer_entry_tmp;
	
	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __FUNCTION__);

#if 0
	dl_list_for_each_safe(peer_entry, peer_entry_tmp, &hs->hs_peer_list,
											struct hs_peer_entry, list) {
		dl_list_del(&peer_entry->list);
		os_free(peer_entry);
	}
#endif 

	/* deinit control interface */
	hotspot_ctrl_iface_deinit(hs);

	ret = hs->drv_ops->drv_inf_exit(hs);

	if (ret)
		return -1;

	return 0;
}

static void hotspot_terminate(int sig, void *signal_ctx)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	eloop_terminate();
}

int hotspot_ap_reload(struct hotspot *hs,
					  const char *iface)
{
	int ret = 0;
	
	ret = hotspot_onoff(hs, iface, 0, EVENT_TRIGGER_ON, HS_AP_RELOAD);

	return ret;
}

static void hotspot_reconfig(int sig, void *signal_ctx)
{
	struct hotspot *hs = (struct hotspot *)signal_ctx;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	if (hs->opmode == OPMODE_AP)
		hotspot_ap_reload(hs, "ra0");
}

void hotspot_run(struct hotspot *hs)
{

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	eloop_register_signal_terminate(hotspot_terminate, hs);

	eloop_register_signal_reconfig(hotspot_reconfig, hs);

	eloop_run();
}

inline int hotspot_test(struct hotspot *hs, const char *iface)
{
	
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}
	
	ret = hs->drv_ops->drv_test(hs->drv_data, iface);

	return ret;
}

inline int hotspot_driver_version(struct hotspot *hs, const char *iface,
						   char *reply, size_t *reply_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = hs->drv_ops->drv_wifi_version(hs->drv_data, iface, reply, reply_len);

	return ret;
}

inline int hotspot_ipv4_proxy_arp_list(struct hotspot *hs, const char *iface,
						   			   char *reply, size_t *reply_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = hs->drv_ops->drv_ipv4_proxy_arp_list(hs->drv_data, iface, reply, reply_len);
	
	return ret;
}

inline int hotspot_ipv6_proxy_arp_list(struct hotspot *hs, const char *iface,
									   char *reply, size_t *reply_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = hs->drv_ops->drv_ipv6_proxy_arp_list(hs->drv_data, iface, reply, reply_len);
	
	return ret;
}

inline int hotspot_set_ie(struct hotspot *hs, const char *iface, 
				          char *ie, size_t ie_len)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hs->drv_ops->drv_set_ie(hs->drv_data, iface, ie, ie_len);
	
	return ret;
}

inline int hotspot_set_osu_asan(struct hotspot *hs, const char *iface, char *enable, size_t len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hs->drv_ops->drv_set_osu_asan(hs->drv_data, iface, enable, len);
	
	return ret;
}

inline int hotspot_set_interworking_enable(struct hotspot *hs, const char *iface, char *enable, size_t len)
{	
	int ret;

	DBGPRINT(RT_DEBUG_ERROR, "==%s\n", __FUNCTION__);
	
	ret = hs->drv_ops->drv_set_interworking(hs->drv_data, iface, enable, len);
	
	return ret;	
}
inline int hotspot_set_bss_load(struct hotspot *hs, struct hotspot_conf *conf)
{
	int ret;
	u8 tmpbuf[10];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	tmpbuf[0] = conf->qload_mode;
	tmpbuf[1] = conf->qload_cu;
	memcpy(&tmpbuf[2], &conf->qload_sta_cnt, 2);
	
	ret = hs->drv_ops->drv_set_bss_load(hs->drv_data, conf->iface, tmpbuf, 4);
	
	return ret;
}

int hotspot_get_legacy_osu_ssid(struct hotspot *hs, struct hotspot_conf *conf)
{
	int ret = 0;
	u8	tmpbuf[256];
	
	conf->legacy_osu_ssidlen = 255;	
	DBGPRINT(RT_DEBUG_TRACE, "%s  \n", __FUNCTION__);
	ret = hs->drv_ops->drv_get_osu_ssid(hs->drv_data, conf->osu_iface, tmpbuf, &conf->legacy_osu_ssidlen);

	if (!ret) {
		int tmp;
		conf->legacy_osu_ssid = os_zalloc(conf->legacy_osu_ssidlen);
		os_memcpy(conf->legacy_osu_ssid, tmpbuf, conf->legacy_osu_ssidlen);
		DBGPRINT(RT_DEBUG_TRACE, "%s  SSID:[%s] SSID_len:%d\n"
			, __FUNCTION__,conf->legacy_osu_ssid,conf->legacy_osu_ssidlen);
	}
	
	return ret;
}

static int hotspot_set_p2p_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct p2p_info_element *ie;
	struct p2p_attribute *p2p_attri;
	char *buf, *pos;
	int ie_len = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");

	buf = os_zalloc(sizeof(*ie) + 4);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}
	
	ie = (struct p2p_info_element *)buf;

	ie->eid = 0xdd;
	ie_len += 1;

	ie->oui[0] = 0x50;
	ie->oui[1] = 0x6F;
	ie->oui[2] = 0x9A;
	ie_len += 3;
	
	/* <WFA ANA> */
	ie->oui_type = 0x09;
	ie_len += 1;

	/* P2P attribute */
	p2p_attri = (struct p2p_attribute *)ie->variable;

	/* P2P manageability attribute */
	p2p_attri->attribute_id = 0x0a;
	p2p_attri->length = cpu2le16(0x0001);
	pos = p2p_attri->variable;

	/* P2P Device Management bit */
	*pos = (*pos & ~0x01) | (0x01);

	/*  Cross Connection Permitted bit */
	*pos = (*pos & ~0x02) | (conf->p2p_cross_connect_permitted & 0x01);

	ie_len += 4;
	
	ie->length = ie_len - 1;
	ie_len += 1;

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_hs_indication_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct hotspot2dot0_indication_element *ie;
	char *buf, version;
	int ie_len = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	buf = os_zalloc(sizeof(*ie));

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}

	ie = (struct hotspot2dot0_indication_element *)buf;

	ie->eid = IE_HS2_INDICATION;
	ie_len += 1;

	ie->oi[0] = 0x50;
	ie->oi[1] = 0x6F;
	ie->oi[2] = 0x9A;
	ie_len += 3;

	/* <WFA ANA> */
	ie->type = 0x10;
	ie_len += 1;
	
	/* Hotspot Configuration */
	ie->hotspot_conf = (conf->DGAF_disabled & 0x01)|((hs->version-1)<<4);

	ie_len += 1;

	ie->length = ie_len - 1;
	ie_len += 1;

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_qosmap_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct qosmap_element *ie;
	char *buf, version, *pos;
	int ie_len = 0, varlen = 0, field = 0;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	varlen = conf->dscp_field*2;
	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not availale"));
		return -1;
	}
		
	ie = (struct qosmap_element *)buf;

	ie->eid = IE_QOS_MAP_SET;
	ie_len += 1;

	pos = ie->dscp_range;
	
	/* dscp exception */
	for (field = 0;field < conf->dscp_field;field++)
	{
		*pos = conf->dscp_exception[field] & 0xff;
		*(pos+1) = (conf->dscp_exception[field] >> 8) & 0xff;
		pos += 2;
		ie_len += 2;
	}
	
	/* dscp range */
	for (field = 0;field < 8;field++)
	{
		*pos = conf->dscp_range[field] & 0xff;
		*(pos+1) = (conf->dscp_range[field] >> 8) & 0xff;
		pos += 2;
		ie_len += 2;
	}
	
	ie->length = ie_len - 1;
	ie_len += 1;

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

static int hotspot_set_interworking_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct interworking_element *ie;
	char *buf, *pos;
	int ret;
	int ie_len = 0, varlen = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);	

	if (conf->is_venue_group && conf->is_venue_type)
		varlen += 2;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct interworking_element *)buf;

	ie->eid = IE_INTERWORKING;
	ie_len += 1;

	ie->access_network_options = (ie->access_network_options & ~0x0F) 
										| (conf->access_network_type & 0x0F);
	ie->access_network_options = (ie->access_network_options & ~0x10)
										| ((conf->internet << 4) & 0x10);
	ie_len += 1;

	pos = ie->variable;

	if (conf->is_venue_group && conf->is_venue_type) {
		*pos++ = conf->venue_group;
		*pos++ = conf->venue_type;
		ie_len += 2;
	}

	if (conf->is_hessid) {
		os_memcpy(pos, conf->hessid, 6);
		pos += 6;
		ie_len += 6;
	}

	ie->length = ie_len - 1;
	ie_len += 1;

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);
	
	return ret;
}

static int hotspot_set_advertisement_proto_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct advertisement_proto_element *ie;
	char *buf, *pos;
	int ret;
	int i, ie_len = 0, varlen = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	varlen += 2 * conf->advertisement_proto_num;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct advertisement_proto_element *)buf;
	
	ie->eid = IE_ADVERTISEMENT_PROTO;
	ie_len += 1;

	pos  = ie->variable;

	for (i = 0; i < conf->advertisement_proto_num; i++) {
		*pos++  = 0x7F;
		*pos++ = conf->advertisement_proto[i];
		ie_len += 2; 
	}

	ie->length = ie_len - 1;
	ie_len += 1;	
	
	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);
	
	return ret;
}

static int hotspot_set_roaming_consortium_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct roaming_consortium_info_element *ie;
	char *buf, *pos;
	int ret;
	int ie_len = 0, varlen = 0;
	int i = 0;
	struct oi_duple *oiduple; 

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i < 3)
			varlen += oiduple->length;
		i++;
	}

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("memory is not available"));
		return -1;
	}

	ie = (struct roaming_consortium_info_element *)buf;

	ie->eid = IE_ROAMING_CONSORTIUM;
	ie_len += 1;

	if ((i - 3) > 0)
		ie->num_anqp_oi = i - 3;
	else
		ie->num_anqp_oi = 0;
	
	ie_len += 1;

	i = 0;

	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i == 0) {
			ie->oi1_oi2_length = (ie->oi1_oi2_length & ~0x0F) | oiduple->length;
		} else if (i == 1) {
			ie->oi1_oi2_length = (ie->oi1_oi2_length & ~0xF0) |
										((oiduple->length << 4) & 0xF0);
		} else
			break;
		i++;
	}
			

	ie_len += 1;

	pos = ie->variable;

	i = 0;
	dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
		if (i < 3) {
			os_memcpy(pos, oiduple->oi, oiduple->length);
			pos += oiduple->length;
			ie_len += oiduple->length;
			i++;
		} else
			break;
	}

	ie->length = ie_len - 1;
	ie_len += 1;
	
	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

#if 0
static int hotspot_set_time_advertisement_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct tm utc_time;
	time_t utime;
	struct time_advertisement_element *ie;
	char *buf, *pos;
	int ie_len = 0, varlen = 0;
	int ret;
	u16 year;
	u8 month, day_of_month, hours, mins, secs;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	varlen = 10; /* Time Value */

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	ie = (struct time_advertisement_element *)buf;
	pos = (char *)ie;

	ie->eid = IE_TIME_ADVERTISEMENT;
	ie_len += 1;
	pos++;

	/* ie length */
	ie_len += 1;
	pos++;

	ie->timing_capabilities = 2;
	ie_len += 1;
	pos++;

	/* Following are Time value */
	time(&utime);
	gmtime_r(&utime, &utc_time);
	year = utc_time.tm_year + 1900;
	year = cpu2le16(year);
	os_memcpy(pos, &year, 2);
	ie_len += 2;
	pos +=2 ;
	
	month = utc_time.tm_mon;
	*pos = month + 1;
	ie_len += 1;
	pos++;

	day_of_month = utc_time.tm_mday;
	*pos = day_of_month;
	ie_len += 1;
	pos++;

	hours = utc_time.tm_hour;
	*pos = hours;
	ie_len += 1;
	pos++;

	mins = utc_time.tm_min;
	*pos = mins;
	ie_len += 1;
	pos++;

	secs = utc_time.tm_sec;
	*pos = secs;
	ie_len += 1;
	pos++;

	os_memset(pos, 0, 3);
	ie_len += 3;
	pos += 3;

	/* Time error */
	os_memset(pos, 0, 5);
	ie_len += 5;
	pos += 5;

	/* Time update counter */
	*pos = 0;
	ie_len++;
	pos++;

	ie->length = ie_len - 2;

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);
	
	return ret;
}
#endif

int hotspot_set_time_zone_ie(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct time_zone_element *ie;
	char *buf, *pos;
	int ie_len = 0, varlen = 0; 
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	varlen = conf->time_zone_len;

	buf = os_zalloc(sizeof(*ie) + varlen);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "memory is not available\n");
		return -1;
	}

	ie = (struct time_zone_element *)buf;
	pos = (char *)ie;

	ie->eid = IE_TIME_ZONE;
	ie_len += 1;
	pos++;

	ie->length = conf->time_zone_len;
	ie_len += 1;
	pos++;

	os_memcpy(ie->variable, conf->time_zone, conf->time_zone_len);
	ie_len += conf->time_zone_len;
	pos += conf->time_zone_len; 

	ret = hotspot_set_ie(hs, conf->iface, buf, ie_len);

	os_free(buf);

	return ret;
}

int hotspot_set_sta_ifaces_all_ies(struct hotspot *hs)
{
	int ret = 0;
	struct hotspot_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	 
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		ret = hotspot_set_interworking_ie(hs, conf);
	
		if (ret)
			return -1;
	}

	return ret;
}

static int hotspot_reset_ap_resource(struct hotspot *hs, const char *iface)
{
	int ret;

	ret = hs->drv_ops->drv_reset_resource(hs->drv_data, iface);

	return ret;
}

int hotspot_set_ap_all_ies(struct hotspot *hs, const char *iface)
{
	int ret;
	struct hotspot_conf *conf;
	u8 is_found = 0;

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	ret = hotspot_set_hs_indication_ie(hs, conf);

	if (ret)
		return -1;
	
	ret = hotspot_set_p2p_ie(hs, conf);

	if (ret)
		return -1;

	ret = hotspot_set_interworking_ie(hs, conf);

	if (ret)
		return -1;

	ret = hotspot_set_advertisement_proto_ie(hs, conf);

	if (ret)
		return -1;

	if (conf->have_roaming_consortium_list) {
		ret = hotspot_set_roaming_consortium_ie(hs, conf);

		if (ret)
			return -1;
	}

    if (hs->version >= 2)
    {
	ret = hotspot_set_qosmap_ie(hs, conf);
	
	if (ret)
		return -1;
    }			
	return ret;
}


int hotspot_set_ap_ifaces_all_ies(struct hotspot *hs)
{
	int ret = 0;
	struct hotspot_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		ret = hotspot_set_ap_all_ies(hs, conf->iface);
		if (ret == -1)
			return ret;		
	}

	return ret;
}

int hotspot_reset_all_ap_resource(struct hotspot *hs)
{
	int ret = 0;
	struct hotspot_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		ret = hotspot_reset_ap_resource(hs, conf->iface);
	}

	return ret;
}

inline int hotspot_send_anqp_req(struct hotspot *hs, const char *iface, 
				                 const char *peer_sta_addr, 
						         const char *anqp_req, 
					             size_t anqp_req_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}
	
	ret = hs->drv_ops->drv_send_anqp_req(hs->drv_data, iface, peer_sta_addr, 
								         anqp_req, anqp_req_len);

	return ret;

}

int hotspot_send_anqp_rsp(struct hotspot *hs, const char *iface, 
				          const char *peer_mac_addr, 
				          const char *anqp_rsp, 
				          size_t anqp_rsp_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
/*	//location needs to send anqp rsp even with hs disabled
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}
*/
	ret = hs->drv_ops->drv_send_anqp_rsp(hs->drv_data, iface, peer_mac_addr,
									     anqp_rsp, anqp_rsp_len);

	return ret;
}

int hotspot_send_btm_req(struct hotspot *hs, const char *iface,
						 const char *peer_mac_addr,
						 const char *btm_req,
						 size_t btm_req_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
#if 0	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}
#endif /* 0 */	

	ret = hs->drv_ops->drv_send_btm_req(hs->drv_data, iface, peer_mac_addr,
										btm_req, btm_req_len);

	return ret;
}

int hotspot_send_qosmap_configure(struct hotspot *hs, const char *iface,
						 const char *peer_mac_addr,
						 const char *qosmap,
						 size_t qosmap_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = hs->drv_ops->drv_send_qosmap_configure(hs->drv_data, iface, peer_mac_addr,
										qosmap, qosmap_len);

	return ret;
}

int hotspot_send_wnm_notify_req(struct hotspot *hs, const char *iface,
						 const char *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len,
						 int	type)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = hotspot_drv_ops_pre_check(hs, iface);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: hotspot drv ops pre check fail\n", __FUNCTION__);
		return -1;
	}

	ret = hs->drv_ops->drv_send_wnm_notify_req(hs->drv_data, iface, peer_mac_addr,
										wnm_req, wnm_req_len, type);

	return ret;
}

int hotspot_onoff(struct hotspot *hs, const char *iface, int enable, 
					int event_trigger, int event_type)
{
	int ret, is_found = 0;
	struct hotspot_conf *conf;
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	if (!enable) 
		conf->hotspot_onoff = 0;	
	
	if (enable) {
		DBGPRINT(RT_DEBUG_OFF, "Enable hotspot2.0 feature(%s)\n", iface);
	} else
		DBGPRINT(RT_DEBUG_OFF, "Disable hotspot2.0 feature(%s)\n", iface);
	
	ret = hs->drv_ops->drv_hotspot_onoff(hs->drv_data, iface, enable, event_trigger, event_type);

	return ret;
}

static u8 hotspot_validate_security_type(struct hotspot *hs, const char *iface)
{
	int ret;

	ret = hs->drv_ops->drv_validate_security_type(hs->drv_data, iface);

	return ret;
}

int hotspot_onoff_all(struct hotspot *hs, int enable)
{
	int ret = 0, setting = enable;
	struct hotspot_conf *conf;
	u8 valid_security_type = 0;

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {

		enable = setting;
		if (conf->hs2_openmode_test)
			valid_security_type = 1;
		else
			valid_security_type = hotspot_validate_security_type(hs, conf->iface);

		if (enable && valid_security_type && conf->interworking) 
			enable = 1;
		else
			enable = 0;

		ret = hotspot_onoff(hs, conf->iface, enable, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
		
		if (ret)
			return -1;

#if 0	
		if ((conf->legacy_osu_exist == 0) && (enable == 0)) {
			ret = hs->drv_ops->drv_hotspot_onoff(hs->drv_data, conf->osu_iface, enable, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
			
			if (ret)
			{
				DBGPRINT(RT_DEBUG_ERROR, "Disable osu(%s) fail\n", conf->osu_iface);
				return -1;
			}
		}
#endif
	}
	
	return ret;
}

int hotspot_ap_test(struct hotspot *hs, const char *iface)
{
	int ret = 0;

	return ret;
}

static int hotspot_anqp_query_test(struct hotspot *hs, struct hotspot_conf *conf)
{
	struct anqp_frame *anqp_req;
	char *buf, *pos;
	size_t anqp_req_len = 0, varlen = 0;
	u16 tmp;
	int ret;

	if (conf->query_anqp_capability_list)
		varlen += 2;
	
	if (conf->query_venue_name)
		varlen += 2;

	if (conf->query_emergency_call_number)
		varlen += 2;

	if (conf->query_network_auth_type)
		varlen += 2;

	if (conf->query_roaming_consortium_list)
		varlen += 2;

	if (conf->query_ip_address_type)
		varlen += 2;

	if (conf->query_nai_realm_list)
		varlen += 2;

	if (conf->query_3gpp_network_info)
		varlen += 2;

	if (conf->query_ap_geospatial_location)
		varlen += 2;

	if (conf->query_ap_civic_location)
		varlen += 2;

	if (conf->query_ap_location_public_uri)
		varlen += 2;

	if (conf->query_domain_name_list)
		varlen += 2;

	if (conf->query_emergency_alert_uri)
		varlen += 2;

	if (conf->query_emergency_nai)
		varlen += 2;

	buf = os_zalloc(sizeof(*anqp_req) + varlen);

	anqp_req = (struct anqp_frame *)buf;

	anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
	anqp_req_len += 2;

	anqp_req->length = cpu2le16(varlen);
	anqp_req_len += 2;

	pos = anqp_req->variable;

	if (conf->query_anqp_capability_list) {
		tmp = cpu2le16(ANQP_CAPABILITY);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}
	
	if (conf->query_venue_name) {
		tmp = cpu2le16(VENUE_NAME_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_call_number) {
		tmp = cpu2le16(EMERGENCY_CALL_NUMBER_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_network_auth_type) {
		tmp = cpu2le16(NETWORK_AUTH_TYPE_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_roaming_consortium_list) {
		tmp = cpu2le16(ROAMING_CONSORTIUM_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ip_address_type) {
		tmp = cpu2le16(IP_ADDRESS_TYPE_AVAILABILITY_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_nai_realm_list) {
		tmp = cpu2le16(NAI_REALM_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_3gpp_network_info) {
		tmp = cpu2le16(ThirdGPP_CELLULAR_NETWORK_INFO);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_geospatial_location) {
		tmp = cpu2le16(AP_GEOSPATIAL_LOCATION);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_civic_location) {
		tmp = cpu2le16(AP_CIVIC_LOCATION);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_ap_location_public_uri) {
		tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_domain_name_list) {
		tmp = cpu2le16(DOMAIN_NAME_LIST);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_alert_uri) {
		tmp = cpu2le16(EMERGENCY_ALERT_IDENTIFIER_URI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}

	if (conf->query_emergency_nai) {
		tmp = cpu2le16(EMERGENCY_NAI);
		os_memcpy(pos, &tmp, 2);
		pos += 2;
	}
	
	anqp_req_len += varlen;
	
	ret = hotspot_send_anqp_req(hs, conf->iface, conf->hs_peer_mac, buf, anqp_req_len);

	os_free(buf);

	return ret;
}

static int hotspot_anqp_hs_query_test(struct hotspot *hs, struct hotspot_conf *conf)
{

	struct hs_anqp_frame *hs_anqp_req;
	char *buf, *pos;
	size_t hs_anqp_req_len = 0, varlen = 0;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	int ret;

	if (conf->query_hs_capability_list)
		varlen++;

	if (conf->query_operator_friendly_name)
		varlen++;

	if (conf->query_wan_metrics)
		varlen++;

	if (conf->query_connection_capability_list)
		varlen++;

	if (conf->query_nai_home_realm)
		varlen++;


	buf = os_zalloc(sizeof(*hs_anqp_req) + varlen);

	hs_anqp_req = (struct hs_anqp_frame *)buf;

	hs_anqp_req->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
	hs_anqp_req_len += 2;

	hs_anqp_req->length = 5 + varlen;
	hs_anqp_req_len += 2;

	os_memcpy(hs_anqp_req->oi, wfa_oi, 3);
	hs_anqp_req_len += 3;

	hs_anqp_req->type = WFA_TIA_HS;
	hs_anqp_req_len++;

	hs_anqp_req->subtype = HS_QUERY_LIST;
	hs_anqp_req_len++;

	pos = hs_anqp_req->variable;

	if (conf->query_hs_capability_list) {
		*pos = HS_CAPABILITY;
		pos++;
	}

	if (conf->query_operator_friendly_name) {
		*pos = OPERATOR_FRIENDLY_NAME;
		pos++;
	}

	if (conf->query_wan_metrics) {
		*pos = WAN_METRICS;
		pos++;
	}

	if (conf->query_connection_capability_list) {
		*pos = CONNECTION_CAPABILITY;
		pos++;
	}

	if (conf->query_nai_home_realm) {
		*pos = NAI_HOME_REALM_QUERY;
		pos++;
	}

	hs_anqp_req_len += varlen;
	
	ret = hotspot_send_anqp_req(hs, conf->iface, conf->hs_peer_mac, buf, hs_anqp_req_len);
	
	os_free(buf);

	return ret;
}

int hotspot_sta_test(struct hotspot *hs, const char *iface)
{
	int ret = 0;

	struct hotspot_conf *conf;
	
	DBGPRINT(RT_DEBUG_TRACE, "\n");

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0)
			break;
	}

	if (conf->anqp_req_type == GAS_ANQP_QUERY)
		ret = hotspot_anqp_query_test(hs, conf);
	else if(conf->anqp_req_type == GAS_ANQP_HS_QUERY)
		ret = hotspot_anqp_hs_query_test(hs, conf);

	return ret;
}

static int hotspot_collect_hs_anqp_rsp(struct hotspot *hs,
							struct hotspot_conf *conf, char *buf)
{
	unsigned char *pos;
	struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)buf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u16 tmplen = 0;	

	if (conf->query_hs_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query HS capability list\n");
		if (conf->have_hs_capability_list) {
			struct anqp_hs_capability *hs_capability_subtype;
			DBGPRINT(RT_DEBUG_TRACE, "Collect HS capability list\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type = WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = HS_CAPABILITY;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			if (conf->have_hs_capability_list) {
				dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
											struct anqp_hs_capability, list) {
					*pos = hs_capability_subtype->subtype;
					pos++;
					tmplen++;
				}
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;

		}
		conf->query_hs_capability_list = 0;
	}

	if (conf->query_operator_friendly_name) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query operator friendly name\n");
	
		if (conf->have_operator_friendly_name) {
			struct operator_name_duple *op_name_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect operator friendly name\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type = WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OPERATOR_FRIENDLY_NAME;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
										struct operator_name_duple, list) {
				*pos = op_name_duple->length;
				pos++;
				tmplen++;

				os_memcpy(pos, op_name_duple->language, 3);
				pos += 3;
				tmplen += 3;

				os_memcpy(pos, op_name_duple->operator_name, op_name_duple->length - 3);
				pos += op_name_duple->length - 3;
				tmplen += op_name_duple->length - 3;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}

		conf->query_operator_friendly_name = 0;
	}
	
	if (conf->query_wan_metrics) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query WAN Metrics\n");
		if (conf->have_wan_metrics) {
			u32 tmpspeed;
			u16 tmplmd;
			DBGPRINT(RT_DEBUG_TRACE, "Collect WAN Metrics\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = WAN_METRICS;
			tmplen += 2;
			
			pos = hs_anqp_rsp->variable;

			*pos = (*pos & ~0x03) | (conf->metrics.link_status & 0x03);
		
			if (conf->metrics.dl_speed == conf->metrics.ul_speed)
				*pos = (*pos & ~0x04) | 0x04;
			else
				*pos = (*pos & ~0x04);

			if (conf->metrics.at_capacity)
				*pos = (*pos & ~0x08) | 0x08;
			else
				*pos = *pos & ~0x08;

			pos++;
			tmplen++;

			tmpspeed = conf->metrics.dl_speed;
			tmpspeed = cpu2le32(tmpspeed);
			os_memcpy(pos, &tmpspeed, 4);
			pos += 4;
			tmplen += 4;

			tmpspeed = conf->metrics.ul_speed;
			tmpspeed = cpu2le32(tmpspeed);
			os_memcpy(pos, &tmpspeed, 4);
			pos += 4;
			tmplen += 4;

			*pos = conf->metrics.dl_load;
			pos++;
			tmplen++;

			*pos = conf->metrics.ul_load;
			pos++;
			tmplen++;

			tmplmd = conf->metrics.lmd;
			tmplmd = cpu2le16(tmplmd);
			os_memcpy(pos, &tmplmd, 2);
			pos += 2;
			tmplen += 2;
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_wan_metrics = 0;
	}


	if (conf->query_connection_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query connection capability\n");
		if (conf->have_connection_capability_list) {
			struct proto_port_tuple	*proto_port_unit;
			u16 tmpport;
			DBGPRINT(RT_DEBUG_TRACE, "Collect Connection Capability\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = CONNECTION_CAPABILITY;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(proto_port_unit, &conf->connection_capability_list,
									struct proto_port_tuple, list) {
				*pos = proto_port_unit->ip_protocol;
				pos++;
				tmplen++;

				tmpport = proto_port_unit->port;
				tmpport = cpu2le16(tmpport);
				os_memcpy(pos, &tmpport, 2);
				pos += 2;
				tmplen += 2;

				*pos = proto_port_unit->status;
				pos++;
				tmplen++;
			}

			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_connection_capability_list = 0;
	}

	if (conf->query_operating_class) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query operating class\n");
		if (conf->have_operating_class) {
			struct operating_class_unit *operating_class;
			DBGPRINT(RT_DEBUG_TRACE, "Collect operating class\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OPERATING_CLASS;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			dl_list_for_each(operating_class, &conf->operating_class_list,
							struct operating_class_unit, list) {
				*pos = operating_class->op_class;
				pos++;
				tmplen++;
			}
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_operating_class = 0;
	}

	if (conf->query_osu_providers_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query osu providers list\n");
		if (conf->have_osu_providers_list) {
			struct osu_providers *providers_list;
			struct osu_friendly_name *friendly_name_list;
			struct osu_method *method_list;
			struct icon_available *icon;
			struct osu_nai *nai;
			struct osu_service_desc *service_desc;
			u16 tmp;
			
			DBGPRINT(RT_DEBUG_TRACE, "Collect osu providers list\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = OSU_PROVIDE_LIST;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;

#if 0			
			if (!conf->legacy_osu_exist) {
				unsigned short nontxlen = cpu2le16(conf->nontransmitted_len);
				
				memcpy(pos, &nontxlen, 2);
				pos += 2;
				conf->nontransmitted_profile = os_zalloc(conf->nontransmitted_len);
				pos += conf->nontransmitted_len;
				*pos = 0;
				pos++;
				tmplen += 3+conf->nontransmitted_len;
			} 
			else 
#endif				
			{
				*pos = conf->legacy_osu_ssidlen;
				pos++;
				tmplen += 1;
				
				os_memcpy(pos, conf->legacy_osu_ssid, conf->legacy_osu_ssidlen);
				pos += conf->legacy_osu_ssidlen;
				tmplen += conf->legacy_osu_ssidlen;
				
				*pos = conf->osu_providers_list_nums;
				pos++;
				tmplen++;
			}
			
			dl_list_for_each(providers_list, &conf->osu_providers_list,
							struct osu_providers, list) {
				*pos = providers_list->osu_providers_list_field_len & 0xff;
				*(pos+1) = (providers_list->osu_providers_list_field_len >> 8)& 0xff;

				pos += 2;
				tmplen += 2;
				
				*pos = providers_list->osu_friendly_name_len;
//JERRY
				*(pos+1) = (providers_list->osu_friendly_name_len >> 8)& 0xff;

				tmplen += 2;
				pos += 2;
				
				dl_list_for_each(friendly_name_list, &providers_list->osu_friendly_name_list,
										struct osu_friendly_name, list) {
					*pos = friendly_name_list->len;
					tmplen += 1;
					pos++;
					
					os_memcpy(pos, friendly_name_list->language, 3);
					pos += 3;
					tmplen += 3;
					
					os_memcpy(pos, friendly_name_list->osu_friendly_name_value, friendly_name_list->len - 3);
					pos += friendly_name_list->len - 3;
					tmplen += friendly_name_list->len - 3;
				}
				
				*pos = providers_list->osu_server_uri_len;
				pos++;
				tmplen++;
				
				os_memcpy(pos, providers_list->osu_server_uri, providers_list->osu_server_uri_len);
				pos += providers_list->osu_server_uri_len;
				tmplen += providers_list->osu_server_uri_len;
				
				*pos = providers_list->osu_method_len;
				tmplen++;
				pos++;
					
				dl_list_for_each(method_list, &providers_list->osu_method_list,
										struct osu_method, list) {					
					*pos = method_list->osu_method_value;
					pos += 1;
					tmplen += 1;
				}
				
				tmp = providers_list->icon_len;
				tmp = cpu2le16(tmp);
				os_memcpy(pos, &tmp, 2);
				pos += 2;
				tmplen += 2;			
					
				dl_list_for_each(icon, &providers_list->icon_list,
										struct icon_available, list) {
					tmp = icon->weight;
					tmp = cpu2le16(tmp);
					os_memcpy(pos, &tmp, 2);
					pos += 2;
					tmplen += 2;								
					tmp = icon->height;
					tmp = cpu2le16(tmp);
					os_memcpy(pos, &tmp, 2);
					pos += 2;
					tmplen += 2;
					
					os_memcpy(pos, icon->language, 3);
					pos += 3;
					tmplen += 3;
					
					*pos = icon->type_len;
					pos += 1;
					tmplen += 1;
					
					os_memcpy(pos, icon->icon_buf, icon->type_len);
					pos += icon->type_len;
					tmplen += icon->type_len;
					
					*pos = icon->filename_len;
					pos += 1;
					tmplen += 1;
					
					os_memcpy(pos, &icon->icon_buf[icon->type_len], icon->filename_len);
					pos += icon->filename_len;
					tmplen += icon->filename_len;
				}
				
				*pos = providers_list->osu_nai_len;
				pos++;
				tmplen++;
				
				dl_list_for_each(nai, &providers_list->osu_nai_list,
										struct osu_nai, list) {
					os_memcpy(pos, nai->osu_nai_value, nai->len);
					pos += nai->len;
					tmplen += nai->len;
				}
				
				*pos = providers_list->osu_service_len;
//JERRY
				*(pos+1) = (providers_list->osu_service_len >> 8)& 0xff;

				tmplen += 2;
				pos += 2;
				
				dl_list_for_each(service_desc, &providers_list->osu_service_desc_list,
										struct osu_service_desc, list) {
					*pos = service_desc->len;
					tmplen += 1;
					pos++;
					
					os_memcpy(pos, service_desc->language, 3);
					pos += 3;
					tmplen += 3;
					
					os_memcpy(pos, service_desc->osu_service_desc_value, service_desc->len - 3);
					pos += service_desc->len - 3;
					tmplen += service_desc->len - 3;
				}
			}
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_osu_providers_list = 0;
	}
	
	if (conf->query_anonymous_nai) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query anonymous nai\n");
		if (conf->have_anonymous_nai) {
			DBGPRINT(RT_DEBUG_TRACE, "Collect anonymous nai\n");
			tmplen = 0;
			hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
			os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
			tmplen += 3;
			hs_anqp_rsp->type= WFA_TIA_HS;
			tmplen++;
			hs_anqp_rsp->subtype = ANONYMOUS_NAI;
			tmplen += 2;
			pos = hs_anqp_rsp->variable;
			memcpy(pos, conf->anonymous_nai, conf->anonymous_nai_len);
			pos += conf->anonymous_nai_len;
			tmplen += conf->anonymous_nai_len;			
			
			hs_anqp_rsp->length = cpu2le16(tmplen);
			hs_anqp_rsp = (struct hs_anqp_frame *)pos;
		}
		conf->query_anonymous_nai = 0;
	}
	
	return 0;
}

static int hotspot_collect_nai_home_realm_anqp_rsp(struct hotspot *hs,
							struct hotspot_conf *conf, char *buf)
{
	char *pos;
	struct anqp_frame *anqp_rsp = (struct anqp_frame *)buf;
	struct nai_home_realm_data_query *home_realm_data_query, *home_realm_data_query_tmp;
	u16 tmplen = 0;
	u16 nai_realm_count  = 0;

	DBGPRINT(RT_DEBUG_TRACE, "STA query NAI home realm list\n");
	
	DBGPRINT(RT_DEBUG_TRACE, "Collect NAI home realm list\n");
	tmplen = 0;
	anqp_rsp->info_id = cpu2le16(NAI_REALM_LIST);
	pos = anqp_rsp->variable + 2;
	tmplen += 2; /* NAI Realm Count length */

	if (conf->have_nai_realm_list) {
		struct nai_realm_data *realm_data;
		u16 nai_realm_data_field_len_tmp;
		struct eap_method *eapmethod;
		struct auth_param *authparam;
		dl_list_for_each(home_realm_data_query, &conf->nai_home_realm_name_query_list,
								 struct nai_home_realm_data_query, list) {
			dl_list_for_each(realm_data, &conf->nai_realm_list,
									 struct nai_realm_data, list) {
				if (strncasecmp(home_realm_data_query->nai_home_realm, realm_data->nai_realm,
						  		home_realm_data_query->nai_home_realm_len) == 0) {
					DBGPRINT(RT_DEBUG_TRACE, "home realm = %s\n", home_realm_data_query->nai_home_realm);
					DBGPRINT(RT_DEBUG_TRACE, "home realm len = %d\n", home_realm_data_query->nai_home_realm_len);
					nai_realm_count++;
					nai_realm_data_field_len_tmp = cpu2le16(realm_data->nai_realm_data_field_len);
					os_memcpy(pos, &nai_realm_data_field_len_tmp, 2);
					tmplen += 2;
					pos += 2;
				
					*pos = realm_data->nai_realm_encoding;
					tmplen += 1;
					pos++;
				
					*pos = realm_data->nai_realm_len;
					tmplen += 1;
					pos++;
				
					os_memcpy(pos, realm_data->nai_realm, realm_data->nai_realm_len);
					tmplen += realm_data->nai_realm_len;
					pos += realm_data->nai_realm_len;

					*pos = realm_data->eap_method_count;
					tmplen += 1;
					pos++;

					dl_list_for_each(eapmethod, &realm_data->eap_method_list,
												struct eap_method, list) {
						*pos = eapmethod->len;
						tmplen += 1;
						pos++;

						*pos = eapmethod->eap_method;
						tmplen += 1;
						pos++;

						*pos = eapmethod->auth_param_count;
						tmplen += 1;
						pos++;

						dl_list_for_each(authparam, &eapmethod->auth_param_list,
												struct auth_param, list) {
							*pos = authparam->id;
							tmplen += 1;
							pos++;

							*pos = authparam->len;
							tmplen += 1;
							pos++;

							os_memcpy(pos, authparam->auth_param_value, authparam->len);
							tmplen += authparam->len;
							pos += authparam->len;
						}
	
					}
				}
			}
		}
	} else
		DBGPRINT(RT_DEBUG_TRACE, "AP does not have nai realm list info\n");
		

	DBGPRINT(RT_DEBUG_TRACE, "NAI Realm Count = %d\n", nai_realm_count);
	nai_realm_count = cpu2le16(nai_realm_count);
	os_memcpy(anqp_rsp->variable, &nai_realm_count, 2);
	anqp_rsp->length = cpu2le16(tmplen);
	
	/* Clear home_realm_name_query_list */
	dl_list_for_each_safe(home_realm_data_query, home_realm_data_query_tmp,
		&conf->nai_home_realm_name_query_list, struct nai_home_realm_data_query, list) {

		dl_list_del(&home_realm_data_query->list);
		os_free(home_realm_data_query);
	}

	dl_list_init(&conf->nai_home_realm_name_query_list);

	conf->query_nai_home_realm = 0;

	return 0;
}

static int hotspot_collect_icon_binary_file(struct hotspot *hs,
							struct hotspot_conf *conf, char *buf)
{
	char *pos;
	struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)buf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u16 tmplen = 0;	
	struct icon_binary *icon_binary_data, *icon_binary_data_tmp;
	struct icon_available *icon;
	struct osu_providers *providers_list;
	
	DBGPRINT(RT_DEBUG_TRACE, "Collect HS icon binary file\n");
	
	tmplen = 0;
	hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
	os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
	tmplen += 3;
	hs_anqp_rsp->type = WFA_TIA_HS;
	tmplen++;
	hs_anqp_rsp->subtype = ICON_BINARY_FILE;
	tmplen += 2;
	pos = hs_anqp_rsp->variable;
	
	if (conf->calc_hs_icon_file_len == 14) {
		*pos = FILE_NOT_FOUND;
		pos++;
		tmplen++;
		os_memset(pos, 0, 3);
		pos += 3;
		tmplen += 3;
		DBGPRINT(RT_DEBUG_OFF, ("file not found\n"));
	}
	else if (conf->have_osu_providers_list) {
		dl_list_for_each(icon_binary_data, &conf->icon_file_list,
									  struct icon_binary, list) {
			dl_list_for_each(providers_list, &conf->osu_providers_list, struct osu_providers, list) {
	        	dl_list_for_each(icon, &providers_list->icon_list, struct icon_available, list) {
	        		#if 0
	        		{
	        			unsigned char *tmp1= icon->filename;
	        			unsigned char *tmp2= icon_binary_data->filename;
	        			int u;
	        			printf("cmp:%d\n", icon->filename_len);
	        			for(u=0;u<icon->filename_len;u++)
	        				printf("%02x:", icon->filename[u]);
	        			printf("\n");
	        			for(u=0;u<icon->filename_len;u++)
	        				printf("%02x:", icon_binary_data->filename[u]);
	        			printf("\n");		
	        		}
	        		#endif
	        		unsigned char *filename= &icon->icon_buf[icon->type_len];
					if (strncasecmp(filename, icon_binary_data->filename, icon->filename_len) == 0) {
						if (icon_binary_data->filesize != 0) {
							FILE *file;
							unsigned char tmp, tmpfile[256];
							int filelen = 0;//, total = 0;
						
							*pos = SUCCESS;
							pos++;
							tmplen++;
							*pos = icon->type_len;
							pos++;
							tmplen++;
							//os_memcpy(pos, icon->type, icon->type_len);
							os_memcpy(pos, icon->icon_buf, icon->type_len);
							pos += icon->type_len;
							tmplen += icon->type_len;
							
							os_memcpy(tmpfile, conf->iconfile_path, conf->iconfile_path_len);
							os_memcpy(&tmpfile[conf->iconfile_path_len], icon_binary_data->filename, icon->filename_len);
							tmpfile[conf->iconfile_path_len+icon->filename_len] = '\0';
							
							file = fopen(tmpfile, "r");
							if (!file) {
								DBGPRINT(RT_DEBUG_ERROR, "open icon binary file(%s) fail\n", icon_binary_data->filename);
								icon_binary_data->filesize = 0;	
								os_memcpy(pos, &icon_binary_data->filesize, 2);
								pos += 2;
								tmplen += 2;
							}
							else {
								icon_binary_data->filesize = cpu2le16(icon_binary_data->filesize);
								os_memcpy(pos, &icon_binary_data->filesize, 2);
								pos += 2;
								tmplen += 2;
								#if 0
								while(fread(&tmp,sizeof(tmp),1,file) != 0) {
									*pos = tmp;
									pos += 1;
									tmplen += 1;
								}    
								#else
								fseek( file, 0, SEEK_END);
								filelen = ftell(file);
								fseek( file, 0, SEEK_SET);
								fread( pos, 1, filelen, file );
								pos += filelen;
								tmplen += filelen;
								#endif
							}
							fclose(file);
						}
						else {
							DBGPRINT(RT_DEBUG_TRACE, ("file not found\n"));
							*pos = FILE_NOT_FOUND;
							pos++;
							tmplen++;
							os_memset(pos, 0, 3);
							pos += 3;
							tmplen += 3;
						}
						goto check_file_end;
					}
				}
			}
		}
	}
	else {
		*pos = UNSPECIFIC_FILE_ERROR;
		pos++;
		tmplen++;
		os_memset(pos, 0, 3);
		pos += 3;
		tmplen += 3;
	}

check_file_end:		
	hs_anqp_rsp->length = cpu2le16(tmplen);
	hs_anqp_rsp = (struct hs_anqp_frame *)pos;
	
	/* Clear icon_file_list */
	dl_list_for_each_safe(icon_binary_data, icon_binary_data_tmp,
		&conf->icon_file_list, struct icon_binary, list) {

		dl_list_del(&icon_binary_data->list);
		os_free(icon_binary_data);
	}

	dl_list_init(&conf->icon_file_list);
	
	conf->query_icon_binary_file = 0;
	
	return 0;
}

static int hotspot_collect_anqp_rsp(struct hotspot *hs,
							 struct hotspot_conf *conf, char *buf)
{
	char *pos;
	struct anqp_frame *anqp_rsp = (struct anqp_frame *)buf;
	u16 tmp, tmplen = 0;

	if (conf->query_anqp_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query ANQP capability list\n");
		if (conf->have_anqp_capability_list) {
			struct anqp_capability *capability_info;
			DBGPRINT(RT_DEBUG_TRACE, "Collect ANQP capability list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ANQP_CAPABILITY);
			pos = anqp_rsp->variable;
			dl_list_for_each(capability_info, &conf->anqp_capability_list, 
											struct anqp_capability, list) {
				tmp = cpu2le16(capability_info->info_id);
				os_memcpy(pos, &tmp, 2);
				tmplen += 2;
				pos += 2;
			}

//JERRY
#if 1
			{
				struct anqp_hs_capability *hs_capability_subtype;
				struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)pos;
			    const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
				u16 tmphslen = 0;

	            DBGPRINT(RT_DEBUG_TRACE, "Collect HS capability list\n");
				tmplen += 4; //0xdd 0xdd len 2bytes
        	    hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
            	os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
	            tmplen += 3;
				tmphslen += 3;
    	        hs_anqp_rsp->type = WFA_TIA_HS;
        	    tmplen++;
				tmphslen++;
            	hs_anqp_rsp->subtype = HS_CAPABILITY;
	            tmplen += 2;
				tmphslen += 2;

    	        pos = hs_anqp_rsp->variable;
        	    if (conf->have_hs_capability_list) {
            	    dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
                                            struct anqp_hs_capability, list) {
                	    *pos = hs_capability_subtype->subtype;
                    	pos++;
	                    tmplen++;
						tmphslen++;
    	            }
        	    }
				hs_anqp_rsp->length = cpu2le16(tmphslen);
			}
#endif

			anqp_rsp->length = cpu2le16(tmplen);
            anqp_rsp = (struct anqp_frame *)pos;
		}
		
		conf->query_anqp_capability_list = 0;
	}

	if (conf->query_venue_name) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query venue name information\n");
		if (conf->have_venue_name) {
			struct venue_name_duple *vname_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect venue name information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(VENUE_NAME_INFO);
			pos = anqp_rsp->variable;
			
			*pos = conf->venue_group;
			pos++;
			tmplen++;

			*pos = conf->venue_type;
			pos++;
			tmplen++;

			dl_list_for_each(vname_duple, &conf->venue_name_list, 
						struct venue_name_duple, list) {
				*pos = vname_duple->length;
				pos++;
				tmplen++;

				os_memcpy(pos, vname_duple->language, 3);
				pos += 3;
				tmplen += 3;

				os_memcpy(pos, vname_duple->venue_name, (vname_duple->length -3));
				pos += (vname_duple->length -3);
				tmplen += (vname_duple->length - 3);
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have venu name information\n");
			
		conf->query_venue_name = 0;
	}

	if (conf->query_network_auth_type) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query network authentication type information\n");
		if (conf->have_network_auth_type) {
			struct net_auth_type_unit *auth_type_unit;
			u16 tmp_url_len = 0;
			DBGPRINT(RT_DEBUG_TRACE, "Collect network authentication type information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(NETWORK_AUTH_TYPE_INFO);
		
			pos = anqp_rsp->variable;
			dl_list_for_each(auth_type_unit, &conf->network_auth_type_list,
									struct net_auth_type_unit, list) {
				*pos = auth_type_unit->net_auth_type_indicator;
				pos++;
				tmplen++;
				
				tmp_url_len = auth_type_unit->re_direct_URL_len;
				tmp_url_len = cpu2le16(tmp_url_len);
				os_memcpy(pos, &tmp_url_len, 2);
				pos += 2;
				tmplen += 2;

				os_memcpy(pos, auth_type_unit->re_direct_URL, 
									auth_type_unit->re_direct_URL_len);
				tmplen += auth_type_unit->re_direct_URL_len;
				pos += auth_type_unit->re_direct_URL_len;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have network auth type info\n");

		conf->query_network_auth_type = 0;
	}

	if (conf->query_roaming_consortium_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query roaming consortium list\n");
		if (conf->have_roaming_consortium_list) {
			struct oi_duple *oiduple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect roaming consortium list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ROAMING_CONSORTIUM_LIST);
		
			pos = anqp_rsp->variable;
			dl_list_for_each(oiduple, &conf->oi_duple_list, 
											struct oi_duple, list) {
				*pos = oiduple->length;
				pos++;
				tmplen++;
				os_memcpy(pos, oiduple->oi, oiduple->length);
				tmplen += oiduple->length;
				pos += oiduple->length;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have roaming consortium list info\n");
			
		conf->query_roaming_consortium_list = 0;
	}

	if (conf->query_ip_address_type) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query IP address type availability\n");
		if (conf->have_ip_address_type) {
			DBGPRINT(RT_DEBUG_TRACE, "Collect IP address type availability\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(IP_ADDRESS_TYPE_AVAILABILITY_INFO);
		
			pos = anqp_rsp->variable;
			*pos = conf->ipv6_address_type & ~0xFC;
			*pos = *pos | ((conf->ipv4_address_type & 0x3F) << 2);
			tmplen++;
			pos++;
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have ip address type availability info\n");
		
		conf->query_ip_address_type = 0;
	}

	if (conf->query_nai_realm_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query NAI realm list\n");
		u16 nai_realm_count  = 0;
		struct nai_realm_data *realm_data;
		u16 nai_realm_data_field_len_tmp;
		struct eap_method *eapmethod;
		struct auth_param *authparam;
		DBGPRINT(RT_DEBUG_TRACE, "Collect NAI realm list\n");
		tmplen = 0;
		anqp_rsp->info_id = cpu2le16(NAI_REALM_LIST);
		pos = anqp_rsp->variable + 2;
		tmplen += 2; /* NAI Realm Count length */
		dl_list_for_each(realm_data, &conf->nai_realm_list,
								 struct nai_realm_data, list) {
			nai_realm_count++;
			nai_realm_data_field_len_tmp = cpu2le16(realm_data->nai_realm_data_field_len);
			os_memcpy(pos, &nai_realm_data_field_len_tmp, 2);
			tmplen += 2;
			pos += 2;
				
			*pos = realm_data->nai_realm_encoding;
			tmplen += 1;
			pos++;
				
			*pos = realm_data->nai_realm_len;
			tmplen += 1;
			pos++;
				
			os_memcpy(pos, realm_data->nai_realm, realm_data->nai_realm_len);
			tmplen += realm_data->nai_realm_len;
			pos += realm_data->nai_realm_len;

			*pos = realm_data->eap_method_count;
			tmplen += 1;
			pos++;

			dl_list_for_each(eapmethod, &realm_data->eap_method_list,
										struct eap_method, list) {
				*pos = eapmethod->len;
				tmplen += 1;
				pos++;

				*pos = eapmethod->eap_method;
				tmplen += 1;
				pos++;

				*pos = eapmethod->auth_param_count;
				tmplen += 1;
				pos++;

				dl_list_for_each(authparam, &eapmethod->auth_param_list,
										struct auth_param, list) {
					*pos = authparam->id;
					tmplen += 1;
					pos++;

					*pos = authparam->len;
					tmplen += 1;
					pos++;

					os_memcpy(pos, authparam->auth_param_value, authparam->len);
					tmplen += authparam->len;
					pos += authparam->len;
				}

			}
		}
		DBGPRINT(RT_DEBUG_TRACE, "NAI Realm Count = %d\n", nai_realm_count);
		nai_realm_count = cpu2le16(nai_realm_count);
		os_memcpy(anqp_rsp->variable, &nai_realm_count, 2);
		anqp_rsp->length = cpu2le16(tmplen);
		anqp_rsp = (struct anqp_frame *)pos;
		
		conf->query_nai_realm_list = 0;
	}

	if (conf->query_3gpp_network_info) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query 3GPP cellular network information\n");
		if (conf->have_3gpp_network_info) {
			struct plmn *plmn_unit;
			struct plmn_IEI *plmn_iei;
			DBGPRINT(RT_DEBUG_TRACE, "Collect 3GPP cellular network information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ThirdGPP_CELLULAR_NETWORK_INFO);
			pos = anqp_rsp->variable;
			
			char *udhl;
			/* GUD */
			*pos = GUD_VER1;
			pos++;
			tmplen++;

			/* Skip UDHL filling */
			udhl = pos;
			pos++;
			tmplen++;

			/* IEI */
			plmn_iei = (struct plmn_IEI *)pos;

			plmn_iei->plmn_list_iei = IEI_PLMN;
			
			tmplen += 3; /* plmn list iei, length of plmn list, and number of plmn */

			
			pos = plmn_iei->variable;

			dl_list_for_each(plmn_unit, &conf->plmn_list, struct plmn, list) {
				plmn_iei->plmn_list_num++;
			
				*pos = (((plmn_unit->mcc[1] << 4) & 0xf0)  | (plmn_unit->mcc[0] & 0x0f));
				pos++;
				tmplen++;

					
				*pos = (((plmn_unit->mnc[2] << 4) & 0xf0) | (plmn_unit->mcc[2] & 0x0f));
				pos++;
				tmplen++;

				*pos = (((plmn_unit->mnc[1] << 4) & 0xf0) | (plmn_unit->mnc[0] & 0x0f));
				pos++;
				tmplen++;
			}

			plmn_iei->plmn_list_len = 1 + (plmn_iei->plmn_list_num * 3);
			
			*udhl = 2 + plmn_iei->plmn_list_len;
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have 3gpp cellular network info\n");
		
		conf->query_3gpp_network_info = 0;
	}

	if (conf->query_domain_name_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query domain name list\n");
		if (conf->have_domain_name_list) {
			struct domain_name_field *dname_field;
			DBGPRINT(RT_DEBUG_TRACE, "Collect domain name list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(DOMAIN_NAME_LIST);
			pos = anqp_rsp->variable;
			dl_list_for_each(dname_field, &conf->domain_name_list, 
											struct domain_name_field, list) {
				*pos = dname_field->length;
				pos++;
				tmplen += 1;
				os_memcpy(pos, dname_field->domain_name, dname_field->length);
				tmplen += dname_field->length;
				pos += dname_field->length;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have domain name list info\n");

		conf->query_domain_name_list = 0;
	}
#if 1 //Kyle location debug
	if (conf->query_ap_civic_location) {
		if(hs->civic_IE_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP civic location\n");		
			DBGPRINT(RT_DEBUG_ERROR, "Collect civic location\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_CIVIC_LOCATION);
			pos = anqp_rsp->variable;
			hex_dump("civic_IE == ",hs->civic_IE,hs->civic_IE_len);
			//*pos = hs->civic_IE_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, hs->civic_IE, hs->civic_IE_len);
			tmplen += hs->civic_IE_len;
			pos += hs->civic_IE_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have civic location\n");
		conf->query_ap_civic_location = 0;
	}
	
	if (conf->query_ap_geospatial_location) {
		if(hs->lci_IE_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP geospatial location\n");	
			DBGPRINT(RT_DEBUG_ERROR, "Collect geospatial location\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_GEOSPATIAL_LOCATION);
			pos = anqp_rsp->variable;
			hex_dump("lci_IE == ",hs->lci_IE,hs->lci_IE_len);

			//*pos = hs->lci_IE_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, hs->lci_IE, hs->lci_IE_len);
			tmplen += hs->lci_IE_len;
			pos += hs->lci_IE_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have geospatial location\n");
		conf->query_ap_geospatial_location = 0;
	}

	if (conf->query_ap_location_public_uri) {
		if(hs->public_id_uri_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP PUBLIC_IDENTIFIER_URI\n");	
			DBGPRINT(RT_DEBUG_ERROR, "Collect PUBLIC_IDENTIFIER_URI\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
			pos = anqp_rsp->variable;
			hex_dump("public_id_uri == ",hs->public_id_uri,hs->public_id_uri_len);

			//*pos = hs->public_id_uri_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, hs->public_id_uri, hs->public_id_uri_len);
			tmplen += hs->public_id_uri_len;
			pos += hs->public_id_uri_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have PUBLIC_IDENTIFIER_URI\n");
		conf->query_ap_location_public_uri = 0;
	}
#endif
	return 0;
}

static int hotspot_collect_total_anqp_rsp(struct hotspot *hs,
							 			  struct hotspot_conf *conf, char *anqp_rsp)
{
	char *curpos = anqp_rsp;
	int ret;

	ret = hotspot_collect_anqp_rsp(hs, conf, curpos);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect anqp rsp fail\n");
		return ret;
	}

	curpos += conf->calc_anqp_rsp_len;

	ret = hotspot_collect_hs_anqp_rsp(hs, conf, curpos);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect hs anqp rsp fail\n");
		return ret;
	}

	if (conf->query_nai_home_realm) {

		curpos += conf->calc_hs_anqp_rsp_len;

		ret = hotspot_collect_nai_home_realm_anqp_rsp(hs, conf, curpos);
		
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "Collect hs nai home realm anqp rsp fail\n");
			return ret;
		}
	}

    if (hs->version >= 2)
    {
	    if (conf->query_icon_binary_file) {
		
		curpos += conf->calc_hs_anqp_rsp_len + conf->calc_hs_nai_home_realm_anqp_rsp_len;		
		
		ret = hotspot_collect_icon_binary_file(hs, conf, curpos);
		
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "Collect icon binary file fail\n");
			return ret;
		}
	    }
    }	
	return 0;
}

size_t hotspot_calc_anqp_rsp_len(struct hotspot *hs,
								 struct hotspot_conf *conf,
								 size_t anqp_req_len,
								 const char *curpos)
{
	size_t varlen = 0, curlen = 0;
	u16 info_id, tmpbuf;
	struct anqp_capability *capability_info;
	struct oi_duple *oiduple;
	struct domain_name_field *dname_field;
	struct venue_name_duple *vname_duple;
	struct nai_realm_data *realm_data;
	struct plmn *plmn_unit;
	struct net_auth_type_unit *auth_type_unit;
	struct anqp_hs_capability *hs_capability_subtype;

	while (curlen < anqp_req_len) {
		os_memcpy(&tmpbuf, curpos, 2);
		info_id = le2cpu16(tmpbuf);
		switch(info_id) {
			case ANQP_CAPABILITY:
				if (!conf->query_anqp_capability_list) {
					conf->query_anqp_capability_list = 1;

					if (conf->have_anqp_capability_list) {
						varlen += 4;
						dl_list_for_each(capability_info, &conf->anqp_capability_list,
													struct anqp_capability, list) {
							varlen += 2;
						}
					}
//JERRY
#if 1					
					if (conf->have_hs_capability_list) {
                        varlen += 10;
                        dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
                                                        struct anqp_hs_capability, list) {
                            varlen += 1;
                        }
                    }
#endif
				}
				break;
			case VENUE_NAME_INFO:
				if (!conf->query_venue_name) {
					conf->query_venue_name = 1;
					if (conf->have_venue_name) {
						/* Including venue group and venue type */
						varlen += 6;
				
						dl_list_for_each(vname_duple, &conf->venue_name_list,
												  struct venue_name_duple, list) {

							varlen += 1; /* Length Field */
							varlen += vname_duple->length;
						}
					}
				}
				break;
			case NETWORK_AUTH_TYPE_INFO:
				if (!conf->query_network_auth_type) {
					conf->query_network_auth_type = 1; 
					if (conf->have_network_auth_type) {
				
						varlen += 4;

						dl_list_for_each(auth_type_unit, &conf->network_auth_type_list,
											struct net_auth_type_unit, list) {
							varlen += 3;
							varlen += auth_type_unit->re_direct_URL_len;
						}
					}
				}

				break;
			case ROAMING_CONSORTIUM_LIST:
				if (!conf->query_roaming_consortium_list) {
					conf->query_roaming_consortium_list = 1;
					if (conf->have_roaming_consortium_list) {
						varlen += 4;
				
						dl_list_for_each(oiduple, &conf->oi_duple_list,
											struct oi_duple, list) {
							varlen += 1;
							varlen += oiduple->length;
						}
					}
				}
				break;
			case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
				if (!conf->query_ip_address_type) {
					conf->query_ip_address_type = 1;
					if (conf->have_ip_address_type) {
						varlen += 4;
						varlen += 1;
					}
				}
				break;
			case NAI_REALM_LIST:
				if (!conf->query_nai_realm_list) {
					conf->query_nai_realm_list = 1;
					varlen += 6;
					dl_list_for_each(realm_data, &conf->nai_realm_list,
										struct nai_realm_data, list) {
						varlen += 2; /* NAI Realm Data Field Length */
						varlen += realm_data->nai_realm_data_field_len;
					}
				}
				break;
			case ThirdGPP_CELLULAR_NETWORK_INFO:
				if (!conf->query_3gpp_network_info) {
					conf->query_3gpp_network_info = 1;
					if (conf->have_3gpp_network_info) {
						varlen += 4;
						varlen += 5;
						dl_list_for_each(plmn_unit, &conf->plmn_list,
											struct plmn, list) {
							varlen += 3;
						}
					}
				}
				break;
			case DOMAIN_NAME_LIST:
				if (!conf->query_domain_name_list) {
					conf->query_domain_name_list = 1;
					if (conf->have_domain_name_list) {
						varlen += 4;
						dl_list_for_each(dname_field, &conf->domain_name_list,
											struct domain_name_field, list) {
							varlen += 1;
							varlen += dname_field->length;
						}
					}
				}
				break;
			/* add location anqp_rsp */
			case AP_GEOSPATIAL_LOCATION:
				if (!conf->query_ap_geospatial_location && hs->lci_IE_len > 0){
					conf->query_ap_geospatial_location = 1;
					varlen += 4;
					varlen += hs->lci_IE_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_GEOSPATIAL_LOCATION  len(%d)\n",__FUNCTION__, hs->lci_IE_len);
				}
				break;
			case AP_CIVIC_LOCATION:
				if (!conf->query_ap_civic_location && hs->civic_IE_len > 0){
					conf->query_ap_civic_location = 1;
					varlen += 4;
					varlen += hs->civic_IE_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_CIVIC_LOCATION  len(%d)\n",__FUNCTION__, hs->civic_IE_len);
				}
				break;
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
				if (!conf->query_ap_location_public_uri && hs->public_id_uri_len > 0){
					conf->query_ap_location_public_uri = 1;
					varlen += 4;
					varlen += hs->public_id_uri_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_LOCATION_PUBLIC_IDENTIFIER_URI  len(%d)\n",__FUNCTION__, hs->public_id_uri_len);
				}
				break;
			default:
				DBGPRINT(RT_DEBUG_OFF, "unknown query req info id(%d)\n", info_id);
				break;			
		}

		curpos += 2;
		curlen += 2;
	}

	return varlen;
}

static size_t hotspot_calc_nai_home_realm_anqp_rsp_len(struct hotspot *hs,
													   struct hotspot_conf *conf,
													   size_t nai_home_realm_anqp_req_len,
													   const char *curpos)
{
	size_t varlen = 0, curlen = 0;
	u8 nai_home_realm_count = 0, i;
	u8 nai_home_realm_encoding;
	u8 nai_home_realm_name_len = 0;
	struct nai_realm_data *realm_data;
	struct nai_home_realm_data_query *home_realm_data_query; 

	DBGPRINT(RT_DEBUG_TRACE, "\n");

	while (curlen < nai_home_realm_anqp_req_len) {
		nai_home_realm_count = *curpos;
		curpos++;
		curlen++;

		/* Info ID, Lenth, and NAI Realm Count */
		varlen += 6;

		for (i = 0; i < nai_home_realm_count; i++) {
			/* NAI Realm Encoding */
			nai_home_realm_encoding = *curpos;
			curpos++;
			curlen++;

			/* NAI Home Realm Name Length */
			nai_home_realm_name_len = *curpos;
			curpos++;
			curlen++;

			/* Add to nai_home_realm_name_query_list */
			home_realm_data_query = os_zalloc(sizeof(*home_realm_data_query) + 
										nai_home_realm_name_len);

			home_realm_data_query->nai_home_realm_encoding = nai_home_realm_encoding;
			home_realm_data_query->nai_home_realm_len = nai_home_realm_name_len;
			os_memcpy(home_realm_data_query->nai_home_realm, curpos, nai_home_realm_name_len);
			dl_list_add_tail(&conf->nai_home_realm_name_query_list, &home_realm_data_query->list);

			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm_encoding = %d\n",  home_realm_data_query->nai_home_realm_encoding);
			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm_len = %d\n",  home_realm_data_query->nai_home_realm_len);
			DBGPRINT(RT_DEBUG_TRACE, "nai_home_realm = %s\n",  home_realm_data_query->nai_home_realm);

			/* Filter if matching nai realm name, if match calc varlen */
			if (conf->have_nai_realm_list) {
				dl_list_for_each(realm_data, &conf->nai_realm_list,
								struct nai_realm_data, list) {

					if (strncasecmp(curpos, realm_data->nai_realm, nai_home_realm_name_len) == 0) {
						varlen += 2; /* NAI Realm Data Field Length */
						varlen += realm_data->nai_realm_data_field_len;

						break;
					}
				}
			}

			curpos += nai_home_realm_name_len;
			curlen += nai_home_realm_name_len;
		}
	}

	conf->query_nai_home_realm = 1;
	return varlen;
}

static size_t hotspot_calc_icon_binary_file_len(struct hotspot *hs,
													   struct hotspot_conf *conf,
													   size_t hs_anqp_req_len,
													   const char *icon_request)
{
	size_t varlen = 0, curlen = 0;
	struct osu_providers *providers_list;
	struct icon_available *icon;
	
	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	conf->query_icon_binary_file = 1;
		
	if (conf->have_osu_providers_list) {
		dl_list_for_each(providers_list, &conf->osu_providers_list,
											  struct osu_providers, list) {
			dl_list_for_each(icon, &providers_list->icon_list,
									struct icon_available, list) {
				DBGPRINT(RT_DEBUG_TRACE, ("icon->type_len=%d,file_name=%d, %d\n", icon->type_len, icon->filename_len, hs_anqp_req_len));
				if ((strncasecmp(&icon->icon_buf[icon->type_len], icon_request, hs_anqp_req_len) == 0) && (icon->filename_len == hs_anqp_req_len)) {
					FILE *file;
					int filesize = 0;
					unsigned char tmpfile[256];
					struct icon_binary *icon_binary_data;
					
					os_memcpy(tmpfile, conf->iconfile_path, conf->iconfile_path_len);
					DBGPRINT(RT_DEBUG_TRACE, ("path len=%d\n", conf->iconfile_path_len));
					os_memcpy(&tmpfile[conf->iconfile_path_len], &icon->icon_buf[icon->type_len], hs_anqp_req_len);
					tmpfile[conf->iconfile_path_len+hs_anqp_req_len] = '\0';
					DBGPRINT(RT_DEBUG_TRACE, "tmpfile=%s\n", tmpfile);
					file = fopen(tmpfile, "r");
					if (!file) {
						DBGPRINT(RT_DEBUG_ERROR, "open configuration(%s) fail\n", &icon->icon_buf[icon->type_len]);
						filesize = 0;
						return 10 + 4;
					} else {
						if(fseek(file, 0, SEEK_END)) {
							filesize = 0;
						} else {
							filesize = ftell(file);
							rewind(file);
						}
						fclose(file);
					}
					DBGPRINT(RT_DEBUG_TRACE, ("filesize=%d\n", filesize));
					icon_binary_data = os_zalloc(sizeof(*icon_binary_data) + 
										icon->filename_len);
					os_memcpy(icon_binary_data->filename, &icon->icon_buf[icon->type_len], icon->filename_len);
					icon_binary_data->filesize = filesize;					
					dl_list_add_tail(&conf->icon_file_list, &icon_binary_data->list);
					
					DBGPRINT(RT_DEBUG_TRACE, ("icon type=%d, filesize=%d\n", icon->type_len, filesize));					
					return 10 + 4 + icon->type_len + filesize;
				}
			}
		}
	}	
	
	return 10 + 4;
}

static size_t hotspot_calc_hs_anqp_rsp_len(struct hotspot *hs,
									struct hotspot_conf *conf,
									size_t hs_anqp_req_len,
									const char *curpos)
{

	size_t varlen = 0, curlen = 0;
	struct anqp_hs_capability *hs_capability_subtype;
	struct operator_name_duple *op_name_duple;
	struct proto_port_tuple *proto_port_unit;
	struct operating_class_unit *operating_class;
	struct osu_providers *providers_list;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	while (curlen < hs_anqp_req_len) {
		switch(*curpos) {
			case HS_CAPABILITY:
				if (!conf->query_hs_capability_list) {
					conf->query_hs_capability_list = 1;
					if (conf->have_hs_capability_list) {
						varlen += 10;
						dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
														struct anqp_hs_capability, list) {
							varlen += 1;
						}
					}
				}

				break;
			case OPERATOR_FRIENDLY_NAME:
				if (!conf->query_operator_friendly_name) {
					conf->query_operator_friendly_name = 1;
					if (conf->have_operator_friendly_name) {
						varlen += 10;
				
						dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
												  struct operator_name_duple, list) {

							varlen += 1;
							varlen += op_name_duple->length;
						}
					}
				}

				break;
			case WAN_METRICS:
				if (!conf->query_wan_metrics) {
					conf->query_wan_metrics = 1;
					if (conf->have_wan_metrics) {
						varlen += 10;
						varlen += 13;
					}
				}
				break;
			case CONNECTION_CAPABILITY:
				if (!conf->query_connection_capability_list) {
					conf->query_connection_capability_list = 1;
					if (conf->have_connection_capability_list) {
						varlen += 10;
						dl_list_for_each(proto_port_unit, &conf->connection_capability_list,
														struct proto_port_tuple, list)
						varlen += 4;
					}

				}
				break;
			case OPERATING_CLASS:
				if (!conf->query_operating_class) {
					conf->query_operating_class = 1;
					if (conf->have_operating_class) {
						varlen += 10;
						dl_list_for_each(operating_class, &conf->operating_class_list,
											struct operating_class_unit, list)
							varlen += 1;
					}
				}
				break;
			case OSU_PROVIDE_LIST:
				if (!conf->query_osu_providers_list && (hs->version >= 2)) {
					conf->query_osu_providers_list = 1;
					if (conf->have_osu_providers_list) {
						varlen += 10;
			
						//if (!conf->legacy_osu_exist)
						//	varlen += conf->nontransmitted_len + 2;						
						//else	
							varlen += conf->legacy_osu_ssidlen + 2;
							
						dl_list_for_each(providers_list, &conf->osu_providers_list,
										  struct osu_providers, list) {
							varlen += providers_list->osu_providers_list_field_len;
							varlen += 2;
						}
					}
				}
				break;	
			case ANONYMOUS_NAI:
				if (!conf->query_anonymous_nai && (hs->version >= 2)) {
					conf->query_anonymous_nai = 1;
					if (conf->have_anonymous_nai) {
						varlen += 10;
						varlen += conf->anonymous_nai_len;
					}
				}
				break;	
			default:
				DBGPRINT(RT_DEBUG_ERROR, "Unknown HS2.0 subtype\n");
				break; 
		}	

		curpos ++;
		curlen ++;
	}

	return varlen;
}


int hotspot_calc_total_anqp_rsp_len(struct hotspot *hs,
									struct hotspot_conf *conf,
									char *anqp_req,
									size_t total_anqp_req_len)
{
	size_t varlen = 0, curlen = 0;
	char *curpos;
	u16 tmpbuf, info_id;

	curpos = anqp_req;

	while (curlen < total_anqp_req_len) {	
		os_memcpy(&tmpbuf, curpos, 2);

		/* Info ID */
		info_id = le2cpu16(tmpbuf);
	
		if (info_id == ANQP_QUERY_LIST) {

			DBGPRINT(RT_DEBUG_TRACE, "Info ID is a ANQP query list\n");
			curpos += 2;
			curlen += 2;

			/* Length Filed */
			os_memcpy(&tmpbuf, curpos, 2);

			curpos += 2;
			curlen += 2;

			conf->calc_anqp_rsp_len = hotspot_calc_anqp_rsp_len(hs,
				 		   			 		    				conf,
									 		    				le2cpu16(tmpbuf),
									 		   		 			curpos);
			varlen += conf->calc_anqp_rsp_len;

			curpos += le2cpu16(tmpbuf);
			curlen += le2cpu16(tmpbuf);
		} else if (info_id == ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST) {
			DBGPRINT(RT_DEBUG_TRACE, "Info ID is a ANQP vendor-specific query list\n");
			curpos += 2;
			curlen += 2;
			const char wfa_oi[3] = {0x50, 0x6F, 0x9A};

			/* Length Field */
			os_memcpy(&tmpbuf, curpos, 2);			
	
			curpos += 2;
			curlen += 2;

			if (os_memcmp(curpos, wfa_oi, 3) != 0) {
				//DBGPRINT(RT_DEBUG_ERROR, "oi field do not match WFA OI [%02x:%02x:%02x]\n", *curpos, *(curpos+1), *(curpos+2));
				printf("oi do not match [%02x:%02x:%02x]\n", *curpos, *(curpos+1), *(curpos+2));
				//return -1;
				return varlen;
			}

			curpos += 3;
			curlen += 3;

			/* Type */
			if (*curpos != WFA_TIA_HS)	{
				struct oi_duple *oiduple;
				DBGPRINT(RT_DEBUG_ERROR, "type(%d) field do not match WFA TIA for HS2.0\n", *curpos);
			
				/* Calc Roaming consortium list */
				if (conf->have_roaming_consortium_list) {
					if (!conf->query_roaming_consortium_list) {
						varlen += 4;
						dl_list_for_each(oiduple, &conf->oi_duple_list,
												struct oi_duple, list) {
							varlen += 1;
							varlen += oiduple->length;
						}
						conf->query_roaming_consortium_list = 1;
					}
				} else {
					DBGPRINT(RT_DEBUG_TRACE, "AP does not have roaming consortium list info\n");
				}
				curpos += (le2cpu16(tmpbuf) - 3);
				curlen += (le2cpu16(tmpbuf) - 3);
				continue;
			}

			curpos++;
			curlen++;
		
			/* Subtype */
			if (*curpos == HS_QUERY_LIST) {
				DBGPRINT(RT_DEBUG_TRACE, "subtype field is HS Query list\n");

				/* Include Reserved */
				curpos += 2;
				curlen += 2;

				conf->calc_hs_anqp_rsp_len = hotspot_calc_hs_anqp_rsp_len(hs,
										 		       					  conf,
										 		       					  le2cpu16(tmpbuf) - 6,
										 		       					  curpos);

				varlen += conf->calc_hs_anqp_rsp_len;

			} else if  (*curpos == NAI_HOME_REALM_QUERY) {
				DBGPRINT(RT_DEBUG_TRACE, "subtype is NAI home realm query\n");
			
				/* Include Reserved */
				curpos += 2;
				curlen += 2;

				conf->calc_hs_nai_home_realm_anqp_rsp_len = 
										hotspot_calc_nai_home_realm_anqp_rsp_len(hs,
																				 conf,
															  	  				 le2cpu16(tmpbuf) - 6,
															  	  				 curpos);
				varlen += conf->calc_hs_nai_home_realm_anqp_rsp_len;

			} else if  (*curpos == ICON_REQUEST) {
				DBGPRINT(RT_DEBUG_TRACE, "subtype is ICON request\n");
				
				/* Include Reserved */
				curpos += 2;
				curlen += 2;
				
				if (hs->version >= 2)
					conf->calc_hs_icon_file_len = hotspot_calc_icon_binary_file_len(hs,
										 		       					  	   conf,
										 		       					  	   le2cpu16(tmpbuf) - 6,
										 		       					  	   curpos);
				else
					conf->calc_hs_icon_file_len = 0;
					
				varlen += conf->calc_hs_icon_file_len;
			} else {
				DBGPRINT(RT_DEBUG_ERROR, "subtype(%d) is not HS Query list and not NAI home realm query and not ICON request\n", *curpos);
			}
			curpos += (le2cpu16(tmpbuf) - 6);
			curlen += (le2cpu16(tmpbuf) - 6);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, "Info ID(%d) is not ANQP query list and not ANQP vendor-specific query list\n", info_id);
			curpos += 2;
			curlen += 2;

			/* Length Filed */
			os_memcpy(&tmpbuf, curpos, 2);

			curpos += (le2cpu16(tmpbuf) + 2);
			curlen += (le2cpu16(tmpbuf) + 2);
		}
	}

	return varlen;
}

int hotspot_event_anqp_req(struct hotspot *hs, 
						   const char *iface, 
				           const char *peer_mac_addr, 
				           char *anqp_req, 
				           size_t anqp_req_len)
{
	char *buf;
	size_t varlen = 0;
	struct hotspot_conf *conf;
	u8 is_found = 0;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
	//DBGPRINT(RT_DEBUG_ERROR, "####conf->iface(%s) iface(%s)\n",conf->iface, iface);
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_TRACE, "Can not find match hotspot configuration file (%s)\n",iface);
		return -1;
	}

	//if (!conf->hotspot_onoff)  //location needs anqp_rep while hs is not on
		//return -1;

	varlen = hotspot_calc_total_anqp_rsp_len(hs,
											 conf,
											 anqp_req,
											 anqp_req_len);
	if (varlen > 0) {
		buf = os_zalloc(varlen);

		if (!buf) {
			DBGPRINT(RT_DEBUG_ERROR, "Memory is not available(%s)\n", __FUNCTION__);
			return -1;
		}
	
		hotspot_collect_total_anqp_rsp(hs, 
									   conf, 
									   buf);
		/* Send ANQP resonse to driver */
		hotspot_send_anqp_rsp(hs, conf->iface, peer_mac_addr, buf, varlen);

		os_free(buf);
	}else {
		DBGPRINT(RT_DEBUG_OFF, "anqp rsp cal len is zero, maybe anqp req is wrong(%s)\n", __FUNCTION__);
		return -1;
	}

	conf->calc_anqp_rsp_len = 0;
	conf->calc_hs_anqp_rsp_len = 0;
	conf->calc_hs_nai_home_realm_anqp_rsp_len = 0;
	return 0;
}

static int hotspot_show_anqp_query_results(struct hotspot *hs,
							   struct hotspot_conf *conf)
{
	int ret = 0;

	if (conf->query_anqp_capability_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query ANQP capability list\n");
		if (conf->have_anqp_capability_list) {
			struct anqp_capability *anqp_capability_unit;
			DBGPRINT(RT_DEBUG_OFF, "Receive ANQP capability list from AP\n");
			dl_list_for_each(anqp_capability_unit, &conf->anqp_capability_list,
												struct anqp_capability, list) {

				DBGPRINT(RT_DEBUG_OFF, "Info ID = %d\n", anqp_capability_unit->info_id);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have ANQP capability list\n");
	}

	if (conf->query_venue_name) {
		DBGPRINT(RT_DEBUG_OFF, "STA query venue name information\n");
		if (conf->have_venue_name) {
			struct venue_name_duple *vname_duple;
			DBGPRINT(RT_DEBUG_OFF, "Receive venue name from AP\n");

			dl_list_for_each(vname_duple, &conf->venue_name_list,
								struct venue_name_duple, list) {
				DBGPRINT(RT_DEBUG_OFF, "language = %s\n", vname_duple->language);
				DBGPRINT(RT_DEBUG_OFF, "venue name = %s\n", vname_duple->venue_name);	
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have venue name information\n");
	}

	if (conf->query_emergency_call_number) {
			DBGPRINT(RT_DEBUG_OFF, "STA query emergency call number\n");
		if (conf->have_emergency_call_number) {
			DBGPRINT(RT_DEBUG_OFF, "Receive emergency call number from AP\n");
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have emergency call number\n");
	}

	if (conf->query_roaming_consortium_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query roaming consortium list\n");	
		if (conf->have_roaming_consortium_list) {
			struct oi_duple *oiduple;
			int i;
			DBGPRINT(RT_DEBUG_OFF, "Receive roaming consortium list from AP\n");	
		
			dl_list_for_each(oiduple, &conf->oi_duple_list, struct oi_duple, list) {
				for (i = 0; i < oiduple->length; i++)
					DBGPRINT(RT_DEBUG_OFF, "roaming consortium_oi[%d] = 0x%02x\n", i, oiduple->oi[i]);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have roaming consortium list\n");
	}

	if (conf->query_nai_realm_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query NAI realm list\n");
		if (conf->have_nai_realm_list) {
			struct nai_realm_data *realm_data;
			struct eap_method *eapmethod;
			struct auth_param *authparam;
			DBGPRINT(RT_DEBUG_OFF, "Receive NAI realm list from AP\n");	

			dl_list_for_each(realm_data, &conf->nai_realm_list, struct nai_realm_data, list) {
				DBGPRINT(RT_DEBUG_OFF, "NAI realm = %s\n", realm_data->nai_realm);
				DBGPRINT(RT_DEBUG_OFF, "EAP method count = %d\n", realm_data->eap_method_count);

				dl_list_for_each(eapmethod, &realm_data->eap_method_list,
												struct eap_method, list) {
					DBGPRINT(RT_DEBUG_OFF, "EAPMethod = %d\n", eapmethod->eap_method);
					DBGPRINT(RT_DEBUG_OFF, "Auth Param Count = %d\n", eapmethod->auth_param_count);

					dl_list_for_each(authparam, &eapmethod->auth_param_list, 
												struct auth_param, list) {
						DBGPRINT(RT_DEBUG_OFF, "Auth ID = %d\n", authparam->id);
						if (authparam->len == 1) {
							DBGPRINT(RT_DEBUG_OFF, "Auth Value = %d\n", *(authparam->auth_param_value));
						} else {
							/* TODO */
						} 
					}
				}

			}

		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have NAI realm list\n");
	}
	
	if (conf->query_3gpp_network_info) {
		DBGPRINT(RT_DEBUG_OFF, "STA query 3gpp network information\n");

		if (conf->have_3gpp_network_info) {
			struct plmn *plmn_unit;
			int i, j = 0;
			DBGPRINT(RT_DEBUG_OFF, "Receive 3gpp network information from AP\n");	
			DBGPRINT(RT_DEBUG_OFF, "GUD = %d\n", conf->gud);
			DBGPRINT(RT_DEBUG_OFF, "UDHL = %d\n", conf->udhl);			

			dl_list_for_each(plmn_unit, &conf->plmn_list, struct plmn, list) {
				DBGPRINT(RT_DEBUG_OFF, "PLMN(%d):\n", j);
				DBGPRINT(RT_DEBUG_OFF, "MCC:");
				for (i = 2; i >= 0; i--)
					printf("%d", plmn_unit->mcc[i]);

				printf("\n");

				DBGPRINT(RT_DEBUG_OFF, "MNC:");

				for (i = 2; i >= 0; i--)
					printf("%d", plmn_unit->mnc[i]);

				printf("\n");
				j++;
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have 3gpp network information\n");
	}	

	if (conf->query_domain_name_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query domain name list\n");
		if (conf->have_domain_name_list) {
			struct domain_name_field *dname_field;
			DBGPRINT(RT_DEBUG_OFF, "Receive domain name list from AP\n");	
	
			dl_list_for_each(dname_field, &conf->domain_name_list, struct domain_name_field, list) {
				DBGPRINT(RT_DEBUG_OFF, "domain name = %s\n", dname_field->domain_name);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have domain name list\n");
	}

	return ret;
}

static int hotspot_show_anqp_hs_query_results(struct hotspot *hs,
							   struct hotspot_conf *conf)
{
	int ret = 0;

	/* Following are HS2.0 elements */
	if (conf->query_hs_capability_list) {
		DBGPRINT(RT_DEBUG_OFF, "STA query HS capability list\n");
		if (conf->have_hs_capability_list) {
			struct anqp_hs_capability *hs_capability;
			DBGPRINT(RT_DEBUG_OFF, "Receive HS capability list from AP\n");	

			dl_list_for_each(hs_capability, &conf->hs_capability_list,
									struct anqp_hs_capability, list) {
				DBGPRINT(RT_DEBUG_OFF, "subtype = %d\n", hs_capability->subtype);
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have HS capability list\n");
	}

	if (conf->query_operator_friendly_name) {
		DBGPRINT(RT_DEBUG_OFF, "STA query Operator friendly name\n");
		if (conf->have_operator_friendly_name) {
			struct operator_name_duple *op_name_duple;
			DBGPRINT(RT_DEBUG_OFF, "Receive Operator friendly name from AP\n");

			dl_list_for_each(op_name_duple, &conf->operator_friendly_duple_list,
								struct operator_name_duple, list) {
				DBGPRINT(RT_DEBUG_OFF, "length = %d\n", op_name_duple->length);
				DBGPRINT(RT_DEBUG_OFF, "language = %s\n", op_name_duple->language);
				DBGPRINT(RT_DEBUG_OFF, "operator_name = %s\n", op_name_duple->operator_name);	
			}
		} else
			DBGPRINT(RT_DEBUG_OFF, "AP does not have HS capability list\n");
	}

	return ret;
}

static int hotspot_clear_query_results(struct hotspot *hs,
								struct hotspot_conf *conf)
{
	int ret = 0;

	if (conf->have_anqp_capability_list) {
		struct anqp_capability *anqp_capability_unit, *anqp_capability_unit_tmp;
		
		dl_list_for_each_safe(anqp_capability_unit, anqp_capability_unit_tmp,
							  &conf->anqp_capability_list, struct anqp_capability, list) {
			dl_list_del(&anqp_capability_unit->list);
			os_free(anqp_capability_unit);
		}
		
		conf->have_anqp_capability_list = 0;
	}

	if (conf->have_roaming_consortium_list) {
		struct oi_duple *oiduple, *oiduple_tmp;
		
		dl_list_for_each_safe(oiduple, oiduple_tmp, &conf->oi_duple_list,
							  struct oi_duple, list) {
			dl_list_del(&oiduple->list);
			os_free(oiduple);
		}
		
		conf->have_roaming_consortium_list = 0;
	}

	if (conf->have_venue_name) {
		struct venue_name_duple *vname_duple, *vname_duple_tmp;

		dl_list_for_each_safe(vname_duple, vname_duple_tmp, &conf->venue_name_list,
							  struct venue_name_duple, list) {
			dl_list_del(&vname_duple->list);
			os_free(vname_duple);
		}

		conf->have_venue_name = 0;
	}

	if (conf->have_nai_realm_list) {
		struct nai_realm_data *realm_data, *realm_data_tmp;
		struct eap_method *eapmethod, *eapmethod_tmp;
		struct auth_param *authparam, *authparam_tmp;

		dl_list_for_each_safe(realm_data, realm_data_tmp,
								&conf->nai_realm_list, struct nai_realm_data, list) {
			dl_list_del(&realm_data->list);
			
			dl_list_for_each_safe(eapmethod, eapmethod_tmp,
									&realm_data->eap_method_list, struct eap_method, list) {
				dl_list_del(&eapmethod->list);
					dl_list_for_each_safe(authparam, authparam_tmp,
										&eapmethod->auth_param_list, struct auth_param, list) {
						dl_list_del(&authparam->list);
						os_free(authparam);
					}
				os_free(eapmethod);
			}
			os_free(realm_data);
		}
		conf->have_nai_realm_list = 0;
	}

	if (conf->have_3gpp_network_info) {
		struct plmn *plmn_unit, *plmn_unit_tmp;
		
		dl_list_for_each_safe(plmn_unit, plmn_unit_tmp,
								&conf->plmn_list, struct plmn, list) {
			dl_list_del(&plmn_unit->list);
			os_free(plmn_unit);
		}

		conf->have_3gpp_network_info = 0;
	}

	if (conf->have_domain_name_list) {
		struct domain_name_field *dname_field, *dname_field_tmp;
		
		dl_list_for_each_safe(dname_field, dname_field_tmp, &conf->domain_name_list,
							  struct domain_name_field, list) {
			dl_list_del(&dname_field->list);
			os_free(dname_field);
		}
		
		conf->have_domain_name_list = 0;
	}

	if (conf->have_hs_capability_list) {
		struct anqp_hs_capability *hs_capability, *hs_capability_tmp;

		dl_list_for_each_safe(hs_capability, hs_capability_tmp, &conf->hs_capability_list,
								struct anqp_hs_capability, list) {
			dl_list_del(&hs_capability->list);
			os_free(hs_capability);
		}

		conf->have_hs_capability_list = 0;
	}

	if (conf->have_operator_friendly_name) {
		struct operator_name_duple *op_name_duple, *op_name_duple_tmp;

		dl_list_for_each_safe(op_name_duple, op_name_duple_tmp, &conf->operator_friendly_duple_list,
								struct operator_name_duple, list) {
			dl_list_del(&op_name_duple->list);
			os_free(op_name_duple);
		}

		conf->have_operator_friendly_name = 0;
	}

	if (conf->have_connection_capability_list) {

	}

	if (conf->have_nai_home_realm_query) {
		struct nai_home_realm_data_query *home_realm_data_query, *home_realm_data_query_tmp;

		dl_list_for_each_safe(home_realm_data_query, home_realm_data_query_tmp,
			&conf->nai_home_realm_name_query_list, struct nai_home_realm_data_query, list) {

			dl_list_del(&home_realm_data_query->list);
			os_free(home_realm_data_query);
		}

		conf->have_nai_home_realm_query = 0;
	}

	return ret;
}

static int hotspot_receive_anqp_rsp(struct hotspot *hs,
							 struct hotspot_conf *conf,
							 const char *pos,
							 size_t buflen,
							 u16 info_id)
{
	u16 tmp_info_id;
	int i, j, k, ret = 0;

	switch (info_id) {
	case ANQP_CAPABILITY:
		while (buflen > 0) {
			struct anqp_capability *anqp_capability_unit;
			anqp_capability_unit = os_zalloc(sizeof(*anqp_capability_unit));
			memcpy(&tmp_info_id, pos, 2);
			anqp_capability_unit->info_id = le2cpu16(tmp_info_id);
			dl_list_add_tail(&conf->anqp_capability_list, &anqp_capability_unit->list);
			pos += 2;
			buflen -= 2;
		}

		if (!dl_list_empty(&conf->anqp_capability_list)) {
			conf->have_anqp_capability_list = 1;
		}

		break;
	case VENUE_NAME_INFO:
		if (buflen > 0) {
			conf->venue_group = *pos;
			pos++;
			buflen--;

			conf->venue_type = *pos;
			pos++;
			buflen--;
		}

		while (buflen > 0) {
			struct venue_name_duple *vname_duple, *vname_duple_new;
			vname_duple = os_zalloc(sizeof(*vname_duple));

			vname_duple->length = *pos;
			pos++;
			buflen--;

			os_memcpy(vname_duple->language, pos, 3);
			pos += 3;
			buflen -= 3;
			
			vname_duple_new = os_realloc(vname_duple, sizeof(*vname_duple) 
										+ vname_duple->length - 3);

			os_memcpy(vname_duple_new->venue_name, pos, vname_duple_new->length - 3);
			pos += (vname_duple_new->length - 3);
			buflen -= (vname_duple_new->length - 3);

			dl_list_add_tail(&conf->venue_name_list, &vname_duple_new->list);
		}
		
		if (!dl_list_empty(&conf->venue_name_list)) {
			conf->have_venue_name = 1;
		}

		break;
	case ROAMING_CONSORTIUM_LIST:
		while (buflen > 0 ) {
			struct oi_duple *oiduple;
			u8 oi_len = *pos;
			pos++;
			buflen--;
			oiduple = os_zalloc(sizeof(*oiduple) + oi_len);
			oiduple->length = oi_len;
			os_memcpy(oiduple->oi, pos, oi_len);
			dl_list_add_tail(&conf->oi_duple_list, &oiduple->list);
			pos += oi_len;
			buflen-= oi_len;
		}

		if (!dl_list_empty(&conf->oi_duple_list)) {
			conf->have_roaming_consortium_list = 1;
		}

		break;
	case NAI_REALM_LIST: 
		while (buflen > 0) {
			struct nai_realm_data *realm_data, *realm_data_new;
			u16 nai_realm_count;
			struct eap_method *eapmethod;
			struct auth_param *authparam, *authparam_new;
			u16 i;
			u16 tmp16;
					
			os_memcpy(&tmp16, pos, 2);
			nai_realm_count = le2cpu16(tmp16);
			pos += 2;
			buflen -= 2;

			DBGPRINT(RT_DEBUG_TRACE, "NAI realm count = %d\n", nai_realm_count);

			for (i = 0; i < nai_realm_count; i++) {
				realm_data = os_zalloc(sizeof(*realm_data));
				dl_list_init(&realm_data->eap_method_list);

				os_memcpy(&tmp16, pos, 2);
					
				realm_data->nai_realm_data_field_len = le2cpu16(tmp16);
				pos += 2;
				buflen -= 2;
					
				realm_data->nai_realm_encoding = *pos;
				pos++;
				buflen--;
					
				realm_data->nai_realm_len = *pos;
				pos++;
				buflen--;
					
				realm_data_new = os_realloc(realm_data, 
									sizeof(*realm_data) + realm_data->nai_realm_len);

				dl_list_init(&realm_data_new->eap_method_list);
				os_memcpy(realm_data_new->nai_realm, pos, realm_data_new->nai_realm_len);
				pos += realm_data_new->nai_realm_len;
				buflen -= realm_data_new->nai_realm_len;

				DBGPRINT(RT_DEBUG_TRACE, "%d\n", realm_data_new->nai_realm_len);

				realm_data_new->eap_method_count = *pos;
				pos++;
				buflen--;

				dl_list_add_tail(&conf->nai_realm_list, &realm_data_new->list);

				DBGPRINT(RT_DEBUG_TRACE, "eap method count = %d\n", realm_data_new->eap_method_count);

				for (j = 0; j < realm_data_new->eap_method_count; j++) {
					eapmethod = os_zalloc(sizeof(*eapmethod));
					dl_list_init(&eapmethod->auth_param_list);
					eapmethod->len = *pos;
					pos++;
					buflen--;

					eapmethod->eap_method = *pos;
					DBGPRINT(RT_DEBUG_TRACE, "EAP method = %d\n", eapmethod->eap_method);
					pos++;
					buflen--;

					eapmethod->auth_param_count = *pos;
					pos++;
					buflen--;

					dl_list_add_tail(&realm_data_new->eap_method_list, &eapmethod->list);

					DBGPRINT(RT_DEBUG_TRACE, "auth param count = %d\n", eapmethod->auth_param_count);
					for (k = 0; k < eapmethod->auth_param_count; k++) {
						authparam = os_zalloc(sizeof(*authparam));
						authparam->id = *pos;
						DBGPRINT(RT_DEBUG_TRACE, "id = %d\n", authparam->id);
						pos++;
						buflen--;

						authparam->len = *pos;
						DBGPRINT(RT_DEBUG_TRACE, "len = %d\n", authparam->len);
						pos++;
						buflen--;

						authparam_new = os_realloc(authparam, 
							sizeof(*authparam_new) + authparam->len);
								
						os_memcpy(authparam_new->auth_param_value, pos, authparam_new->len);
						pos += authparam_new->len;
						buflen -= authparam_new->len;
						dl_list_add_tail(&eapmethod->auth_param_list, &authparam_new->list);
					}
				}
			}
		}

		if (!dl_list_empty(&conf->nai_realm_list)) {
			conf->have_nai_realm_list = 1;
		}
		break;
	case ThirdGPP_CELLULAR_NETWORK_INFO:
		while (buflen > 0) {
			struct plmn *plmn_unit;
			conf->gud = *pos;
			pos++;
			buflen--;
			conf->udhl = *pos;
			pos++;
			buflen--;

			if (*pos == IEI_PLMN) {
				//u8 plmn_value_len;
				u8 plmn_num;
				pos++;
				buflen--;
				
				/* Length of PLMN list value contents */
				//plmn_value_len = *pos;
				pos++;
				buflen--;

				/* Number of PLMNs */
				plmn_num = *pos;
				pos++;
				buflen--;

				for (i = 0; i < plmn_num; i++) {
					plmn_unit = os_zalloc(sizeof(*plmn_unit));
					plmn_unit->mcc[0] = *pos & 0x0f;
					plmn_unit->mcc[1] = (*pos & 0xf0) >> 4; 
					pos++;
					buflen--;

					plmn_unit->mcc[2] = *pos & 0x0f;
					plmn_unit->mnc[2] = (*pos & 0xf0) >> 4;
					pos++;
					buflen--;

					plmn_unit->mnc[0] = *pos & 0x0f;
					plmn_unit->mnc[1] = (*pos & 0xf0) >> 4;
					pos++;
					buflen--;
					dl_list_add_tail(&conf->plmn_list, &plmn_unit->list);
				}
				
			}
			
		}

		if (!dl_list_empty(&conf->plmn_list)) {
			conf->have_3gpp_network_info = 1;
		}
		break;
	case DOMAIN_NAME_LIST: 				
		while (buflen > 0 ) {
			struct domain_name_field *dname_field;
			u8 dname_len = *pos;
			pos++;
			buflen--;
			dname_field = os_zalloc(sizeof(*dname_field) + dname_len);
			dname_field->length = dname_len;
			os_memcpy(dname_field->domain_name, pos, dname_len);
			dl_list_add_tail(&conf->domain_name_list, &dname_field->list);
			pos += dname_len;
			buflen-= dname_len;
		}

		if (!dl_list_empty(&conf->domain_name_list)) {
			conf->have_domain_name_list = 1;
		}

		break;
	}

	return ret;
}

static int hotspot_receive_anqp_hs_rsp(struct hotspot *hs,
								struct hotspot_conf *conf,
							 	const char *pos,
							 	size_t buflen,
								u8 subtype)
{
	int ret = 0;

	switch (subtype) {
	case HS_CAPABILITY:
		while (buflen > 0) {
			struct anqp_hs_capability *anqp_hs_capability_unit;
			anqp_hs_capability_unit = os_zalloc(sizeof(*anqp_hs_capability_unit));
			anqp_hs_capability_unit->subtype = *pos;
			pos++;
			buflen--;
			dl_list_add_tail(&conf->hs_capability_list, &anqp_hs_capability_unit->list);
		}

		if (!dl_list_empty(&conf->hs_capability_list)) {
			conf->have_hs_capability_list = 1;
		}

		break;
	case OPERATOR_FRIENDLY_NAME:
		while (buflen > 0) {
			struct operator_name_duple *op_name_duple, *op_name_duple_new;
			op_name_duple = os_zalloc(sizeof(*op_name_duple));
			op_name_duple->length = *pos;
			pos++;
			buflen--;
			os_memcpy(op_name_duple->language, pos, 3);
			pos += 3;
			buflen -= 3;
			
			op_name_duple_new = os_realloc(op_name_duple, sizeof(*op_name_duple) 
										+ op_name_duple->length - 3);
			os_memcpy(op_name_duple_new->operator_name, pos, op_name_duple_new->length - 3);
			pos += (op_name_duple_new->length - 3);
			buflen -= (op_name_duple_new->length - 3);
			dl_list_add_tail(&conf->operator_friendly_duple_list, &op_name_duple_new->list);
		}

		if (!dl_list_empty(&conf->operator_friendly_duple_list)) {
			conf->have_operator_friendly_name = 1;
		}
				
		break;

	case WAN_METRICS:

		break;
	case CONNECTION_CAPABILITY:

		break;

	default:

		break;
	}

	return ret;
}

int hotspot_event_anqp_rsp(struct hotspot *hs,
						   const char *iface,
						   const char *peer_mac_addr,
						   int status,
						   const char *anqp_rsp,
						   size_t anqp_rsp_len)
{
	const char *pos = anqp_rsp;
	size_t buflen;
	u16 info_id;
	struct hotspot_conf *conf;
	const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
	u8 subtype;

	DBGPRINT(RT_DEBUG_TRACE,"\n");

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0)
			break;
	}

	if (status == 0) {
		while (anqp_rsp_len > 0) {
			os_memcpy(&info_id, pos, 2);
			info_id = le2cpu16(info_id);
			pos += 2;
			anqp_rsp_len -= 2;
			switch (info_id) {
			case ANQP_CAPABILITY:
			case VENUE_NAME_INFO:
			case EMERGENCY_CALL_NUMBER_INFO:
			case NETWORK_AUTH_TYPE_INFO:
			case ROAMING_CONSORTIUM_LIST:
			case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
			case NAI_REALM_LIST:
			case ThirdGPP_CELLULAR_NETWORK_INFO:
			case AP_GEOSPATIAL_LOCATION:
			case AP_CIVIC_LOCATION:
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
			case DOMAIN_NAME_LIST:
			case EMERGENCY_ALERT_IDENTIFIER_URI:
			case EMERGENCY_NAI:
				os_memcpy(&buflen, pos, 2);
				buflen = le2cpu16(buflen);
				pos += 2;
				anqp_rsp_len -= 2;				

				hotspot_receive_anqp_rsp(hs, conf, pos, buflen, info_id);
				pos += buflen;
				anqp_rsp_len -= buflen;
	
				break;

			case ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST:
				os_memcpy(&buflen, pos, 2);
				buflen = le2cpu16(buflen);
				pos += 2;
				anqp_rsp_len -= 2;
	
				if (os_memcmp(pos, wfa_oi, 3) == 0) {
					pos += 3;
					anqp_rsp_len -= 3;
					buflen -= 3 ;

					if (*pos == WFA_TIA_HS) {
						pos++;
						anqp_rsp_len--;
						buflen--;

						subtype = *pos;
						pos++;
						anqp_rsp_len--;
						buflen--;
						hotspot_receive_anqp_hs_rsp(hs, conf, pos, buflen, subtype);
						pos += buflen;
						anqp_rsp_len -= buflen;
					} else
						DBGPRINT(RT_DEBUG_TRACE, "Unknown type field\n");

				} else
					DBGPRINT(RT_DEBUG_TRACE, "Unknown OI field\n");

				break;
			default:
				DBGPRINT(RT_DEBUG_TRACE, "Unknown info ID(%d) from AP\n", info_id);	
				goto clear_query_results;
				break;	
			}
		}
	
		/* Show query results */
		if (conf->anqp_req_type == GAS_ANQP_QUERY)
			hotspot_show_anqp_query_results(hs, conf);
		else if (conf->anqp_req_type == GAS_ANQP_HS_QUERY)
			hotspot_show_anqp_hs_query_results(hs, conf);
		else
			DBGPRINT(RT_DEBUG_ERROR, "Unknow anqp request type\n");

clear_query_results:
 		/* Clear query results */
		hotspot_clear_query_results(hs, conf);

	} else if (status == TIMEOUT) {
		DBGPRINT(RT_DEBUG_OFF, "Timeout\n");
	}
	
	/* Send again to test */
	hotspot_sta_test(hs, iface);

	return 0;
}

int hotspot_event_test(struct hotspot *hs)
{

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	return 0;
}

int hotspot_event_ap_reload(struct hotspot *hs,
							const char *iface)
{
	char confname[256];
	int ret = 0, is_found = 0;
	struct hotspot_conf *conf;
	u8 valid_security_type = 0;	
	
	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			sprintf(confname, conf->confname);
			break;
		}
	}

	if (!is_found)
		return -1;
	
	/* remove and deinit configuration from hs_conf_list */
	hotspot_deinit_config(hs, conf);
	dl_list_del(&conf->list);
	os_free(conf);

	/* Reload configuration file to hotspot configuration */
	//sprintf(confname, "/etc_ro/hotspot_ap_%s.conf", iface);
	ret = hotspot_init_ap_config(hs, confname);

	if (ret) 
		return -1;

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found)
		return -1;

	/* Reset all driver hs resource */
	ret = hotspot_reset_ap_resource(hs, iface);

	if (ret)
		return -1;

	/* Set hotspot IE */
	ret = hotspot_set_ap_all_ies(hs, iface);

	if (ret)
		return -1;

	if (conf->hs2_openmode_test)
		valid_security_type = 1;
	else
		valid_security_type = hotspot_validate_security_type(hs, conf->iface);

	DBGPRINT(RT_DEBUG_TRACE, "%s: interworking = %d, valid_security_type = %d\n", __FUNCTION__,
					conf->interworking, valid_security_type);

	if (conf->interworking && valid_security_type)
		ret = hotspot_onoff(hs, iface, 1, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
	else
		ret = hotspot_onoff(hs, iface, 0, EVENT_TRIGGER_ON, HS_ON_OFF_BASE);
		
	return ret;
}

int hotspot_event_hs_onoff(struct hotspot *hs,
						   const char *iface,
						   int enable)
{
	int ret = 0, is_found = 0;
	struct hotspot_conf *conf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}
	
	if (!is_found)
		return -1;

	if (enable)
			conf->hotspot_onoff = 1;

	return ret;
}

int hotspot_event_get_location_IE(struct hotspot *hs,						   
						   struct location_IE *location)
{
	int ret = 0, is_found = 0;
	struct hotspot_conf *conf;

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __FUNCTION__);

	/*dl_list_for_each(conf, &hs->hs_conf_list, struct hotspot_conf, list) {
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}
	
	if (!is_found)
		return -1;
	*/
	if (location->len != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, "GOT location IE type: %d len: %d\n", location->type, location->len);
		switch(location->type)
		{
			case AP_GEOSPATIAL_LOCATION:
				os_memset(hs->lci_IE, 0, LOCATION_IE_LEN);				
				os_memcpy(hs->lci_IE,location->location_buf,location->len);
				hs->lci_IE_len = location->len;
				hex_dump("civic_IE == ",hs->lci_IE,hs->lci_IE_len);
			break;
			case AP_CIVIC_LOCATION:
				os_memset(hs->civic_IE, 0, LOCATION_IE_LEN);
				os_memcpy(hs->civic_IE,location->location_buf,location->len);
				hs->civic_IE_len = location->len;
			break;
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
				os_memset(hs->public_id_uri, 0, LOCATION_IE_LEN);
				os_memcpy(hs->public_id_uri,location->location_buf,location->len);
				hs->public_id_uri_len = location->len;
			break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, "unknown location IE type: %d len: %d\n", location->type, location->len);
		}
	}

	return ret;
}

size_t hotspot_calc_btm_req_len(struct hotspot *hs,
							 	struct hotspot_conf *conf)
{
	struct bss_transition_candi_preference_unit *preference_unit;
	size_t btm_req_len = 0;

	btm_req_len += 4;
	
	if (conf->have_bss_termination_duration)
		btm_req_len += 12;

	if (conf->have_session_info_url)
		btm_req_len += conf->session_info_url_len + 1;
	
	if (conf->have_bss_transition_candi_list) {
		dl_list_for_each(preference_unit, &conf->bss_transition_candi_list,
							struct bss_transition_candi_preference_unit, list)
			btm_req_len += 18;
	}

	return btm_req_len;
}

int hotspot_collect_btm_req(struct hotspot *hs,
							struct hotspot_conf *conf,
							const char *peer_addr,
							char *btm_req)
{
	struct btm_frame *frame;
	char *pos = btm_req;
	struct bss_transition_candi_preference_unit *preference_unit;
	struct neighbor_report_element *report_element;
	size_t report_element_len = 0;
	struct neighbor_report_subelement *report_subelement;

	frame = (struct btm_frame *)btm_req;

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x01) | 
									(conf->preferred_candi_list_included);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x02) |
									(conf->abridged << 1);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x04) |
									(conf->disassociation_imminent << 2);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x08) |
									(conf->bss_termination_included << 3);

	frame->u.btm_req.request_mode = (frame->u.btm_req.request_mode & ~0x10) |
									(conf->ess_disassociation_imminent << 4);

	pos += 1;

	frame->u.btm_req.disassociation_timer = cpu2le16(conf->disassociation_timer);
	pos += 2;

	frame->u.btm_req.validity_interval = conf->validity_interval;
	pos += 1;


	if (conf->have_bss_termination_duration) {
		report_subelement = (struct neighbor_report_subelement *)pos;
		report_subelement->subelement_id = BSS_TERMINATION_DURATION;
		report_subelement->length = 10;
		report_subelement->u.bss_termination_duration.bss_termination_tsf = 
											cpu2le64(conf->bss_termination_tsf);
		report_subelement->u.bss_termination_duration.duration = 
											cpu2le16(conf->bss_termination_duration);
		pos += 12;
	}

	if (conf->have_session_info_url) {
		/* session url length */
		*pos = conf->session_info_url_len;
		pos++;
		
		/* session url */
		os_memcpy(pos, conf->session_info_url, conf->session_info_url_len);
		pos += conf->session_info_url_len;
	}
	
	if (conf->have_bss_transition_candi_list) {
			report_element = (struct neighbor_report_element *)pos;
			report_element->eid = IE_NEIGHBOR_REPORT;
			pos += 2;
			
			os_memcpy(report_element->bssid, peer_addr, 6);
			pos += 6;
			report_element_len += 6;

			os_memset(&report_element->bss_info, 0, 4);
			pos += 4;
			report_element_len += 4;

			report_element->regulatory_class = 12;
			pos++;
			report_element_len++;

			report_element->channel_number = 11;
			pos++;
			report_element_len++;

			report_element->phy_type = 4;
			pos++;
			report_element_len++;

			dl_list_for_each(preference_unit, &conf->bss_transition_candi_list,
						struct bss_transition_candi_preference_unit, list) {
				report_subelement = (struct neighbor_report_subelement *)pos;
				report_subelement->subelement_id = BSS_TRANSITION_CANDIDATE_PREFERENCE;
				report_subelement->length = 1;
				report_subelement->u.bss_transition_candi_preference.preference = 
												preference_unit->preference;
				pos += 3;
				report_element_len += 3;
			}
			report_element->length = report_element_len;
	}

	return 0;
}

int hotspot_event_btm_query(struct hotspot *hs,
						    const char *iface,
						    const char *peer_addr,
						    const char *btm_query,
						    size_t btm_query_len)
{
	int is_found = 0, ret = 0;
	struct hotspot_conf *conf;
	size_t btm_req_len;
	char *buf;

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

	btm_req_len = hotspot_calc_btm_req_len(hs, conf);

	buf = os_zalloc(btm_req_len);

	if (!buf) {
		DBGPRINT(RT_DEBUG_ERROR, "Not available memory(%s)\n", __FUNCTION__);
		return -1;
	}
	
	ret = hotspot_collect_btm_req(hs, conf, peer_addr, buf);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect btm req fail(%s)\n", __FUNCTION__);
		os_free(buf);
		return -1;
	}

	ret = hotspot_send_btm_req(hs, iface, peer_addr, buf, btm_req_len);

	os_free(buf);

	return ret;
}

int hotspot_event_btm_rsp(struct hotspot *hs,
						  const char *iface,
						  const char *peer_mac_addr,
						  const char *btm_rsp,
						  size_t btm_rsp_len)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	return 0;
}

static void hotspot_proxy_arp_ipv4(char *buf,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char	IsDAD)
{
	char *pos;
	u16 protocol_type = cpu2be16(0x0806);
	u16 hw_address_type = cpu2be16(0x0001);
	u16 protocol_address_type;
	u16 arp_operation = cpu2be16(0x0002);;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	pos = buf;
	
	/* DA */
	os_memcpy(pos, source_mac_addr, 6);
	pos += 6;

	/* SA */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* Protocol type */
	os_memcpy(pos, &protocol_type, 2);
	pos += 2;

	/* HW address yype */
	os_memcpy(pos, &hw_address_type, 2);
	pos += 2;

	/* Protocol address type */
	protocol_address_type = cpu2be16(0x0800);
	os_memcpy(pos, &protocol_address_type, 2);
	pos += 2;

	/* HW address size */
	*pos = 0x06;
	pos++;

	/* Protocol address size */
	*pos = 0x04;
	pos++;

	/* arp operation */
	os_memcpy(pos, &arp_operation, 2);
	pos += 2;
	
	/* Sender MAC address */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* Sender IP address */
	os_memcpy(pos, target_ip_addr, 4);
	pos += 4;

	/* Target MAC address */
	os_memcpy(pos, source_mac_addr, 6);
	pos += 6;

	/* Target IP address */
	//if (IsDAD == 1)
	//	os_memcpy(pos, target_ip_addr, 4);
	//else	
	os_memcpy(pos, source_ip_addr, 4);
	pos += 4;
}

static u16 icmpv6_csum(const char *saddr,
					   const char *daddr,
					   u16 len,
					   u8 proto,
					   const char *icmp_msg)
{
	struct _ipv6_addr *sa_ipv6_addr = (struct _ipv6_addr *)saddr;
	struct _ipv6_addr *da_ipv6_addr = (struct _ipv6_addr *)daddr;
	u32 carry, ulen, uproto;
	u32 i;
	u32 csum = 0x00;
	u16 chksum;

	if (len % 4)
		return 0;
	
	for( i = 0; i < 4; i++)
	{
		csum += sa_ipv6_addr->ipv6_addr32[i];
		carry = (csum < sa_ipv6_addr->ipv6_addr32[i]);
		csum += carry;
	}

	for( i = 0; i < 4; i++)
	{
		csum += da_ipv6_addr->ipv6_addr32[i];
		carry = (csum < da_ipv6_addr->ipv6_addr32[i]);
		csum += carry;
	}

	ulen = htonl((u32)len);
	csum += ulen;
	carry = (csum < ulen);
	csum += carry;

	uproto = htonl((u32)proto);
	csum += uproto;
	carry = (csum < uproto);
	csum += carry;
	
	for (i = 0; i < len; i += 4)
	{
		csum += *((u32 *)(&icmp_msg[i]));
		carry = (csum < (*((u32 *)(&icmp_msg[i]))));
		csum += carry;
	}

	while (csum>>16)
		csum = (csum & 0xffff) + (csum >> 16);

	chksum = ~csum;
	
	return chksum;
}

static void hotspot_proxy_arp_ipv6(char *buf,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char IsDAD)
{

	char *pos, *pcsum, *icmpv6hdr;
	char DadDestAddr[16]={0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
	u16 protocol_type = cpu2be16(0x86dd);
	u16 payload_len = cpu2be16(0x0020);
	u16	checksum = 0;
	u32 icmpmsglen = 0x20;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	pos = buf;
	
	/* DA */
//JERRY
#if 0
	if (IsDAD == 1) {
		*pos = 0x33;pos++;
		*pos = 0x33;pos++;
		*pos = 0x00;pos++;
		*pos = 0x00;pos++;
		*pos = 0x00;pos++;
		*pos = 0x01;pos++;
	}
	else {
#endif
		os_memcpy(pos, source_mac_addr, 6);
		pos += 6;
//	}

	/* SA */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;
	
	/* Protocol type */
	os_memcpy(pos, &protocol_type, 2);
	pos += 2;

	/* Version, Traffic Class, Flow label */
	*pos = 0x60;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	/* payload length */
	os_memcpy(pos, &payload_len, 2);
	pos += 2;

	/* Next header */
	*pos = 0x3a;
	pos++;

	/* Hop limit */
	*pos = 0xff;
	pos++;

	/* source ip address */
	os_memcpy(pos, target_ip_addr, 16);
	pos += 16;

	/* destination ip address */
	if (IsDAD == 1) {
		*pos = 0xff;pos++;
		*pos = 0x02;pos++;
		pos = pos + 13;
		*pos = 0x01;pos++;
		
		DBGPRINT(RT_DEBUG_OFF, ("ipv6 dad.....\n"));
	}
	else {
		os_memcpy(pos, source_ip_addr, 16);
		pos += 16;
	}

	/* ICMP field */
	icmpv6hdr = pos;
	/* Type */
	*pos = 0x88;
	pos++;

	/* Code */
	*pos = 0x00;
	pos++;

	/* Checksum */
	pcsum = pos;
	os_memcpy(pos, &checksum, 2);
	pos += 2;

	/* flags */
	*pos = 0x60;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	*pos = 0x00;
	pos++;

	/* targer address */
	os_memcpy(pos, target_ip_addr, 16);
	pos += 16;

	/* Possible options */
	/* target linker-layerr address type */
	*pos = 0x02;
	pos++; 

	/* length */
	*pos = 0x01;
	pos++;

	/* target link-layer address */
	os_memcpy(pos, target_mac_addr, 6);
	pos += 6;

	/* re-calculate checksum */
	if (IsDAD == 1)
		checksum = icmpv6_csum(target_ip_addr, DadDestAddr, icmpmsglen, 0x3a, icmpv6hdr);
	else
		checksum = icmpv6_csum(target_ip_addr, source_ip_addr, icmpmsglen, 0x3a, icmpv6hdr);
	os_memcpy(pcsum, &checksum, 2);
}

static int hotspot_event_proxy_arp(struct hotspot *hs,
								   const int ifindex,
								   u8 ip_type,
								   u8 from_ds,
								   const char *source_mac_addr,
								   const char *source_ip_addr,
								   const char *target_mac_addr,
								   const char *target_ip_addr,
								   unsigned char IsDAD)
{
	int sock;
	struct sockaddr_ll sll;
	char *buf;
	u8 bufsize;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* send arp response on behalf of target */
	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	memset(&sll, 0, sizeof(sll));
	
	if (from_ds) {
		sll.sll_ifindex = if_nametoindex("eth2");
		DBGPRINT(RT_DEBUG_TRACE, "Send Proxy ARP Packet to eth2\n");
	
	} else {
		sll.sll_ifindex = ifindex;
		DBGPRINT(RT_DEBUG_TRACE, "Send Proxy ARP Packet to ra0(%d)\n", ifindex);
	}

	if (ip_type == IPV4)
		bufsize = 60;
	else
		bufsize = 86;

	buf = os_zalloc(bufsize);

	if (ip_type == IPV4)
		hotspot_proxy_arp_ipv4(buf, source_mac_addr, source_ip_addr, 
										target_mac_addr, target_ip_addr, IsDAD);
	else
		hotspot_proxy_arp_ipv6(buf, source_mac_addr, source_ip_addr,
										target_mac_addr, target_ip_addr, IsDAD);

	if (sendto(sock, buf, bufsize, 0, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Send ARP response failed\n");
		return -1;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s:Send Proxy ARP Pakcet\n", __FUNCTION__);

	close(sock);

	os_free(buf);

	return 0;
}

/* 
 * hotspot event callback 
 *
 * general feature
 * event_test: test if daemon can receive driver event
 * event_hs_on_off:
 *
 * station mode
 * event_apqp_rsp:
 *
 * ap mode
 * event_anqp_req : 
 */
struct hotspot_event_ops hs_event_ops = {
	.event_anqp_req = hotspot_event_anqp_req,
	.event_anqp_rsp = hotspot_event_anqp_rsp,
	.event_test = hotspot_event_test,
	.event_hs_onoff = hotspot_event_hs_onoff,
	.event_btm_query = hotspot_event_btm_query,
	.event_btm_rsp = hotspot_event_btm_rsp,
	.event_proxy_arp = hotspot_event_proxy_arp,
	.event_ap_reload = hotspot_event_ap_reload,
	.event_get_location_IE = hotspot_event_get_location_IE,
};

int hotspot_init(struct hotspot *hs, 
				 struct hotspot_event_ops *event_ops,
				 int drv_mode,
				 int opmode,
				 int version)
{
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	/* Initialze event loop */
	ret = eloop_init();

	if (ret)
		return -1;

	if (drv_mode == RA_WEXT) {
		/* use ralink wireless extension */
		hs->drv_ops = &hotspot_drv_wext_ops;
	} else if (drv_mode == RA_NETLINK) {
		/* use ralink genenic netlink */
		//hs->drv_ops = &hotspot_drv_ranl_ops;
	}

	hs->event_ops = event_ops;

	hs->opmode = opmode;

	hs->drv_mode = drv_mode;
	
	/* Initialize control interface */
	hs->hs_ctrl_iface = hotspot_ctrl_iface_init(hs); 

	hs->version = version;

	dl_list_init(&hs->hs_conf_list);

	dl_list_init(&hs->hs_peer_list);

	hs->drv_data = hs->drv_ops->drv_inf_init(hs, opmode, drv_mode);

	if (!hs->drv_data) {
		/* deinit control interface */
		hotspot_ctrl_iface_deinit(hs);
		return -1;

	}
	return 0;
}
