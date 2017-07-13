#!/bin/sh

ApCliEnable_Nvram=`nvram_get 2860 CMCC_ApCliEnable`
ApCliAuthMode_Nvram=`nvram_get 2860 CMCC_ApCliAuthMode`
ApCliEncrypType_Nvram=`nvram_get 2860 CMCC_ApCliEncrypType`
ApCliSsid_Nvram=`nvram_get 2860 CMCC_ApCliSsid`
ApCliWPAPSK_Nvram=`nvram_get 2860 CMCC_ApCliWPAPSK`

# debug
echo "CMCC_ApCliEnable=$ApCliEnable_Nvram"
echo "CMCC_ApCliAuthMode=$ApCliAuthMode_Nvram"
echo "CMCC_ApCliEncrypType=$ApCliEncrypType_Nvram"
echo "CMCC_ApCliSsid=$ApCliSsid_Nvram"
echo "CMCC_ApCliWPAPSK=$ApCliWPAPSK_Nvram"
echo "CMCC_ApCliSsid=$ApCliSsid_Nvram"

if [ "$ApCliEnable_Nvram" = "1" ]; then
	ifconfig apcli0 up
	brctl addif br0 ra0
	brctl addif br0 apcli0
	iwpriv apcli0 set ApCliEnable=0
	iwpriv apcli0 set ApCliAuthMode="$ApCliAuthMode_Nvram"
	iwpriv apcli0 set ApCliEncrypType="$ApCliEncrypType_Nvram"
	iwpriv apcli0 set ApCliSsid="$ApCliSsid_Nvram"
	iwpriv apcli0 set ApCliWPAPSK="$ApCliWPAPSK_Nvram"
	iwpriv apcli0 set ApCliSsid="$ApCliSsid_Nvram"
	iwpriv apcli0 set ApCliEnable=1
fi


